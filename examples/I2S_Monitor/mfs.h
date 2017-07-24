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

/*
 * Some useful feature to protect a varable from interrupt
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


