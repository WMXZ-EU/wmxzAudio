//Copyright 2017 by Walter Zimmer
// Version 18-05-17
//
// general teensy includes
#include "kinetis.h"
#include "core_pins.h"
#include "usb_serial.h"

#include "config.h"
#include "logger.h"

/***************************** LOGGING ***********************/

/**
   define Disk_buffer to be multiple to write buffer and multiple of data buffer
*/
#if defined(__MK20DX256__)
  #define BUFFSIZE (8*1024/4) // size in int32 of buffer to be written
  #define DISK_BUFFSIZE (2*BUFFSIZE) // size in bytes of memory buffer 
#elif defined(__MK66FX1M0__)
  #define BUFFSIZE (32*1024/4) // size in int32 of buffer to be written
  #define DISK_BUFFSIZE (6*BUFFSIZE) // size in bytes of memory buffer
#endif

uint32_t diskBuffer[DISK_BUFFSIZE] __attribute__( ( aligned ( 4 ) ) ); // memory storage

// include FileSystem Interface
#include "mfs.h"

uint32_t ifn = 0;

//uint32_t isFileOpen = 0;
uint32_t t0 = 0;
uint32_t t1 = 0;

uint32_t loggerCount = 0;
uint32_t errCount=0;
int32_t n2_dat = 0;  //file byte count
uint32_t n1_dat = INF;  // write-in pinter
uint32_t n0_dat = 0;   // write-out pointer
//uint32_t n_guard = DISK_BUFFSIZE;

mProtect fileStatus;

header_s *m_header;

struct HDR { uint32_t cnt; uint32_t sz; uint32_t tt; uint32_t dm;
             uint32_t n0; uint32_t n1; uint32_t nad[6];} recHdr;
/*-------------------------------------------------------------------------*/
void logger_init(header_s *header)
{
	m_header=header;
	m_header->fsize = MAX_BLOCK_COUNT*BUFFSIZE;
	m_header->nclst = MAX_BLOCK_COUNT;
	m_header->hsize = sizeof(HDR);

	mFS.init();
	fileStatus=0;
	for(int ii=0;ii<DISK_BUFFSIZE;ii++) diskBuffer[ii]=INF;
}

/*-------------------------------------------------------------------------*/
static inline  void storeBuffer(uint32_t *buffer, uint32_t ndat)
{
  for (uint32_t ii = 0; ii < ndat; ii++) // ndat is assumed to be a multiple of 4
  {
    diskBuffer[((n1_dat + ii) % DISK_BUFFSIZE)] = buffer[ii]; ii++;
    diskBuffer[((n1_dat + ii) % DISK_BUFFSIZE)] = buffer[ii]; ii++;
    diskBuffer[((n1_dat + ii) % DISK_BUFFSIZE)] = buffer[ii]; ii++;
    diskBuffer[((n1_dat + ii) % DISK_BUFFSIZE)] = buffer[ii];
  }
  n1_dat += ndat;
}

/**
 *  =========================================================================
 */

extern uint32_t i2sProcCount;
uint32_t istart,iend;
/*-------------------------------------------------------------------------*/
// logger_write will be called from I2S ISR (at elevated priority)
void logger_write(uint8_t *data, uint16_t nbuf)
{ // data point to input having a size of nbuf bytes
  // is called whenever new data are available
  //
  if(!((fileStatus==1) || (fileStatus==2))) return; 
  //
  // check if we need to reset buffer pointers
  if(n1_dat==INF) { n1_dat=0; n0_dat=0; }
  //
  // check if we need a global header
  if(fileStatus==1)
  { istart=i2sProcCount;
	fileStatus=2;
	//
	n2_dat = MAX_BLOCK_COUNT * BUFFSIZE;
	m_header->rtc = RTC_TSR;
	m_header->t0 = millis();

	storeBuffer((uint32_t *)m_header,sizeof(header_s)/4);

#ifdef DO_DEBUG
	Serial.printf("header: %d %d %d %d %d\n\r",
	n2_dat,
	m_header->rtc,m_header->t0, m_header->nch,m_header->fsamp);
#endif
	n2_dat -=  sizeof(header_s)/4;
  }

  // update record counter
  recHdr.cnt++;
  //
  // check if disk buffer can take some data
  // n0_dat+DISK_BUFFERSIZE is wrap-around location
  // if((n1_dat+nbuf/4+sizeof(recHdr)/4) > (n0_dat+DISK_BUFFSIZE))
  // { errCount++; return; }
  //
  // !!! don't check, but overwrite, the user will detect overwrite and correct
  // so we don't get out of sync

  // prepare record Header
  recHdr.sz=nbuf;
  recHdr.tt=micros();
  recHdr.dm=loggerCount;
  recHdr.n0=n0_dat;
  recHdr.n1=n1_dat;

  storeBuffer((uint32_t *) &recHdr,sizeof(recHdr)/4);
  
  // store data to disk buffer
  storeBuffer((uint32_t *)data,nbuf/4);

  n2_dat -= nbuf/4;
}

/*-------------------------------------------------------------------------*/
// logger_save will be called from loop() at lowest priority
// it interfaces with uSD
uint32_t logger_save(void)
{ // does also open/close of file when required
  //
  static uint16_t isLogging = 0; // flag to ensure single access to function

  char filename[80];

  if (isLogging) return 0; // we are already busy (should not happen)
  isLogging = 1;

  if(fileStatus==4) { isLogging = 0; return 1; } // don't do anything anymore

  if(fileStatus==0)
  {
	// open new file
	ifn++;
	if (ifn > MXFN) // have end of acquisition reached, so end operation
	{	fileStatus = 4;
#ifdef DO_DEBUG
		Serial.println("end of Acq"); Serial.flush();
#endif
		isLogging = 0; return INF; // tell calling loop() to stop ACQ
    } // end of all operations

    sprintf(filename, FMT, (unsigned int)ifn);
#ifdef DO_DEBUG
    Serial.println(filename); Serial.flush();
#endif
	mFS.open(filename);
	t0 = micros();
	loggerCount=0;
	errCount=0;
	recHdr.cnt=0;
	fileStatus = 1; // flag as open
	isLogging = 0; return 1;
  }


  if(fileStatus==2)
  {
	// check if we are to much behind (overflow)
//	while(n0_dat + DISK_BUFFSIZE <n1_dat)
//	{ 	// don't do any thing but advances pointer
//		n0_dat += BUFFSIZE;
//	}
    // if available save data to disk
    while((n0_dat + BUFFSIZE) < n1_dat)
    {	//
		// flag if we have overrun
		// otherwise continue
		if((n0_dat + DISK_BUFFSIZE) < n1_dat) errCount++;

		// access data in disk buffer
		uint32_t *Buffer = &diskBuffer[n0_dat%DISK_BUFFSIZE];
		// send to disk
		if (!mFS.write((uint8_t *)Buffer, BUFFSIZE*4))
		{ fileStatus = 3; break;} // close file on write failure

		// augment n0_dat only after written to disk
		n0_dat += BUFFSIZE;

		loggerCount++;
		if(loggerCount == MAX_BLOCK_COUNT)
		{ iend=i2sProcCount; fileStatus= 3; break;}
			//
			//write data to file
#ifdef DO_DEBUG
		if (!(loggerCount % 10)) Serial.printf(".");
		if (!(loggerCount % 640)) {Serial.println(errCount); errCount=0;}
#endif
		//
#ifdef DO_DEBUG
		Serial.flush();
#endif
    }
    if(fileStatus==2){ isLogging = 0; return 1; }
  }

  if(fileStatus==3)
  {
	//close file
	t1 = micros();
	mFS.close();
    //
#ifdef DO_DEBUG
    float MBs = (1.0f+ MAX_BLOCK_COUNT * 4*BUFFSIZE) / (1.0f * (t1 - t0));
    Serial.printf("%d\n (%d: %.3f - %.3f MB/s)\n\r", errCount, 
    				iend-istart,(t1 - t0)/1000000.0f, MBs); Serial.flush();
#endif
    errCount=0;
    fileStatus= 0; // flag file as closed   
#if MFS>0
    n1_dat=INF; // reset disk buffer when opening next file
#endif
    isLogging = 0; return 1;
  }
  
  isLogging=0; return 0;
}

