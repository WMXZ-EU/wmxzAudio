//Copyright 2017 by Walter Zimmer
// Version 18-05-17
//
/*
 * FS specific interface 
 * change to your preferred SD library
 * need following methods
 *    void init(void)
 *    void open(char * filename)
 *    void close()
 *    uint32_t write( uint8_t *buffer, uint32_t nbuf)
 * description of API should be obvious from uSDFS example below
 *
*/
void blink(uint32_t msec);

#define MFS  0
#if MFS == 0

// from generic FAT
//------------------------------------------------------------------------------
/**
 * \struct partitionTable
 * \brief MBR partition table entry
 *
 * A partition table entry for a MBR formatted storage device.
 * The MBR partition table has four entries.
 */
struct partitionTable {
          /**
           * Boot Indicator . Indicates whether the volume is the active
           * partition.  Legal values include: 0X00. Do not use for booting.
           * 0X80 Active partition.
           */
  uint8_t  boot;
          /**
            * Head part of Cylinder-head-sector address of the first block in
            * the partition. Legal values are 0-255. Only used in old PC BIOS.
            */
  uint8_t  beginHead;
          /**
           * Sector part of Cylinder-head-sector address of the first block in
           * the partition. Legal values are 1-63. Only used in old PC BIOS.
           */
  unsigned beginSector : 6;
           /** High bits cylinder for first block in partition. */
  unsigned beginCylinderHigh : 2;
          /**
           * Combine beginCylinderLow with beginCylinderHigh. Legal values
           * are 0-1023.  Only used in old PC BIOS.
           */
  uint8_t  beginCylinderLow;
          /**
           * Partition type. See defines that begin with PART_TYPE_ for
           * some Microsoft partition types.
           */
  uint8_t  type;
          /**
           * head part of cylinder-head-sector address of the last sector in the
           * partition.  Legal values are 0-255. Only used in old PC BIOS.
           */
  uint8_t  endHead;
          /**
           * Sector part of cylinder-head-sector address of the last sector in
           * the partition.  Legal values are 1-63. Only used in old PC BIOS.
           */
  unsigned endSector : 6;
           /** High bits of end cylinder */
  unsigned endCylinderHigh : 2;
          /**
           * Combine endCylinderLow with endCylinderHigh. Legal values
           * are 0-1023.  Only used in old PC BIOS.
           */
  uint8_t  endCylinderLow;
           /** Logical block address of the first block in the partition. */
  uint32_t firstSector;
           /** Length of the partition, in blocks. */
  uint32_t totalSectors;
}__attribute__((packed));
/** Type name for partitionTable */
typedef struct partitionTable part_t;

//------------------------------------------------------------------------------
/**
 * \struct masterBootRecord
 * \brief Master Boot Record
 * The first block of a storage device that is formatted with a MBR.
 */
struct masterBootRecord {
           /** Code Area for master boot program. */
  uint8_t  codeArea[440];
           /** Optional Windows NT disk signature. May contain boot code. */
  uint32_t diskSignature;
           /** Usually zero but may be more boot code. */
  uint16_t usuallyZero;
           /** Partition tables. */
  part_t   part[4];
           /** First MBR signature byte. Must be 0X55 */
  uint8_t  mbrSig0;
           /** Second MBR signature byte. Must be 0XAA */
  uint8_t  mbrSig1;
}__attribute__((packed));
/** Type name for masterBootRecord */
typedef struct masterBootRecord mbr_t;

//------------------------------------------------------------------------------


#include "sdio.h"
#undef WRITE_SYNCHRONIZE
#define WRITE_SYNCHRONIZE  0

// storage to keep master boot record
mbr_t mbr;

extern "C" uint32_t MA,MB;

class c_mFS
{
  private:
	uint32_t sector=0;
	uint32_t size = 0;
    /* Stop with dying message */
    void die(char *str)
    {
#ifdef DO_DEBUG
      Serial.printf("%s: failed.\n\r", str); Serial.flush();
#endif
      for (;;) {yield(); blink(100);}
    }

  public:
    void format(uint8_t *work, uint32_t nb)
    {
    }

    void init(void)
    {
		SDHC_InitCard();
		sector = 0;
		read((uint8_t *) &mbr,512);

		sector=mbr.part[0].firstSector;
		size=mbr.part[0].totalSectors;

		#define DO_DEBUG1
		#ifdef DO_DEBUG1
			for(int ii=0;ii<4;ii++)
				Serial.printf(" %d %d\r\n", mbr.part[ii].firstSector, mbr.part[ii].totalSectors);
			Serial.printf("MA %d; MB %d\n\r",MA,MB);
		#endif
    }

    void open(char * filename)
    {
    }

    void close(void)
    {
    }

    uint32_t write(uint8_t *buffer, uint32_t nbuf)
    {
    	if(sector>size) return 0;

    	uint32_t count = nbuf/SDHC_BLOCK_SIZE;
		SDHC_DMAWait();	// make sure uSD card is not busy
		SDHC_WriteBlocks(buffer, sector, count);
		#if WRITE_SYNCHRONIZE==1
			SDHC_DMAWait();
		#endif
    	sector += count;
    	return nbuf;
    }

    uint32_t read(uint8_t *buffer, uint32_t nbuf)
    {
    	if(sector>size) return 0;

    	uint32_t count = nbuf/SDHC_BLOCK_SIZE;
		SDHC_DMAWait();	// make sure uSD card is not busy
		SDHC_ReadBlocks(buffer, sector, count);
		SDHC_DMAWait();	// wait always when reading
    	sector += count;
    	return nbuf;
    }
};

c_mFS mFS;

#elif MFS == 1

#include "ff.h"
#include "ff_utils.h"

//#define DO_DEBUG
extern "C" uint32_t usd_getError(void);

class c_mFS
{
  private:
    FRESULT rc;     /* Result code */
    FATFS fatfs;    /* File system object */
    FIL fil;        /* File object */

    UINT bw;
    
    TCHAR wfilename[80];

    /* Stop with dying message */
    void die(char *str, FRESULT rc) 
    { 
#ifdef DO_DEBUG
      Serial.printf("%s: Failed with rc=%u.\n\r", str, rc); Serial.flush(); 
#endif
      for (;;) {yield(); blink(100);} 
    }
    
  public:
    void format(uint8_t *work, uint32_t nb)
    {
      rc = f_mkfs((TCHAR *)_T("0:/"), FM_EXFAT, 128*1024, work, nb);
      if (rc) die((char*)"format", rc);
    }
    
    void init(void)
    {
      rc = f_mount (&fatfs, (TCHAR *)_T("0:/"), 0);      /* Mount/Unmount a logical drive */
      if (rc) die((char*)"mount", rc);
    }
    
    void open(char * filename)
    {
      char2tchar(filename,80,wfilename);
      //
      // check status of file
      rc =f_stat(wfilename,0);
#ifdef DO_DEBUG2
      Serial.printf("stat %d %x\n",rc,fil.obj.sclust);
#endif

      rc = f_open(&fil, wfilename, FA_WRITE | FA_CREATE_ALWAYS);
#ifdef DO_DEBUG2
      Serial.printf(" opened %d %x\n\r",rc,fil.obj.sclust);
#endif
      // check if file is Good
      if(rc == FR_INT_ERR)
      { // only option is to close file
        rc = f_close(&fil);
        if(rc == FR_INVALID_OBJECT)
        { 
#ifdef DO_DEBUG
          Serial.println("unlinking file");
#endif
          rc = f_unlink(wfilename);
          if (rc) die((char*)"unlink", rc);
        }
        else
          die((char*)"open-close", rc);
        
      }
      // retry open file
      rc = f_open(&fil, wfilename, FA_WRITE | FA_CREATE_ALWAYS);
      if(rc) die((char*)"open", rc);
    }

    void close(void)
    {
      rc = f_close(&fil);
      if (rc) die((char*)"close", rc);
    }

    uint32_t write(uint8_t *buffer, uint32_t nbuf)
    {      
      rc = f_write(&fil, buffer, nbuf, &bw);
      if (rc== FR_DISK_ERR) // IO error
      { uint32_t usd_error = usd_getError();
#ifdef DO_DEBUG
        Serial.printf(" write FR_DISK_ERR : %x\n\r",usd_error);
#endif
        // only option is to close file
        // force closing file
        return 0;
      }
      else if(rc) die((char*)"write",rc);
      return bw;
    }

    uint32_t read(uint8_t *buffer, uint32_t nbuf)
    {      
      rc = f_read(&fil, buffer, nbuf, &bw);
      if (rc== FR_DISK_ERR) // IO error
      { uint32_t usd_error = usd_getError();
#ifdef DO_DEBUG
        Serial.printf(" read FR_DISK_ERR : %x\n\r",usd_error);
#endif
        // only option is to close file
        // force closing file
        return 0;
      }
      else if(rc) die((char*)"read",rc);
      return bw;
    }
};

c_mFS mFS;

/*
  end of FS specific interface
*/
#endif
/*
 * Some useful feature to protect a variable handling from interrupt
 */
//--------------------------------------------------
class mProtect
{
  public:
    void operator=(const uint32_t x);
    uint32_t operator==(const uint32_t x);
  private:
    uint32_t value;
};
//
inline void mProtect::operator=(const uint32_t x)
{
  __disable_irq();
    value=x;
  __enable_irq();
}
//
inline uint32_t mProtect::operator==(const uint32_t x)
{ uint32_t ret;
  __disable_irq();
    ret = (value == x);
  __enable_irq();
  return ret;
}

