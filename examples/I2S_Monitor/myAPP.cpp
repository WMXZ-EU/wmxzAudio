//Copyright 2017 by Walter Zimmer
// Version 03-07-17
//========================= DO NOT USE THIS EXAMPLE, AS IT IS NOT TESTED =====================================
//
// in addition to teensy core
// this application needs to be set for 
//  Serial (logger only)
//  Audio  (usb-audio only)
//  Serial + Midi + Audio (logger + usb-audio)
//
// this application needs support from
// uSDFS        // for logger
// wmxzCore     // for I2S, DMA
// wmxzDevices  // for ICS43432
// wmxzAudio    // for USB-monitor
// wmxzDSP      // for use of dsp processor
//
// 02-july-2017: added quad ICS connections
// for ICS4343x microphones
// T3.6 		Mic1 	Mic2 	Mic3 	Mic4
// GND			GND		GND		GND		GND
// 3.3V			VCC		VCC		VCC		VCC
// Pin11		CLK		CLK		CLK		CLK
// Pin12		WS		WS		WS		WS
// Pin13		SD		SD		--		--
// Pin38		--		--		SD		SD
// GND		 	L/R		--		L/R		--
// 3.3      --    L/R   --    L/R

// general teensy includes
#include "kinetis.h"
#include "core_pins.h"
#include "usb_serial.h"
//
#include "AudioStream.h"
#include "usb_audio.h"
//
// application specifific includes
#include "myApp.h"
#include "config.h"

// enable either logger
// or USB_AUDIO
//#define DO_LOGGER
#ifndef DO_LOGGER
  #define DO_USB_AUDIO
  #define DO_DSP
#endif

/***********************************************************************/
#include "ICS43432.h" // defines also N_BITS

// Note: 
// change either F_CPU or F_SAMP if I2S setup fails to configure
// i.e. there are no DMA interrupts and 'i2sInProcessing' is not running
// typically this happens if the clock generation has a too high multiplier
// e.g. F_CPU=168 MHz and F_SAMP =  96000 fails to run (cannot find proper dividers)
//  but F_CPU=168 MHz and F_SAMP = 100000 runs fine
//

#define N_CHAN 4    // number of channels

#ifdef DO_USB_AUDIO
  #define AUDIO_SHIFT 0 // shift to right (or attenuation)
  #define ICHAN_LEFT  0 // index for left usb_audio channel
  #define ICHAN_RIGHT 0 // index for right usb_audio channel
#endif

/********************** I2S parameters *******************************/
c_ICS43432 ICS43432;

extern "C" void i2sInProcessing(void * s, void * d);

#define N_SAMP 128
#define N_BUF (2 * N_CHAN * N_SAMP)    // dual buffer size for DMA 
int32_t i2s_rx_buffer[N_BUF];         // buffer for DMA

#ifdef DO_DSP
  #define N_FFT 256
  #define N_FILT 1
  #define MM (N_FFT-N_SAMP)  // aka NN-LL
#endif

//
#ifdef DO_USB_AUDIO
/******************************USB-Audio Interface**********************************************************/
  #include <AudioStream.h>
  //
  #include "AudioInterface.h"
  #include "AudioTrigger.h"

  #define N_DAT (128*(F_SAMP/100)/441)  // number of samples per received DMA interrupt  

  static uint32_t audioBuffer[3*N_DAT]; // 3 buffers for audio xfer (need at least 2)
  c_buff audioStore(audioBuffer,sizeof(audioBuffer)/4);
  
  AudioInterface  interface(&audioStore,F_SAMP);
  AudioOutputUSB  usb;
  AudioConnection patchCord1(interface,0,usb,0);
  AudioConnection patchCord2(interface,1,usb,1); 
#endif

//======================== Asynchronous Blink ======================================
void blink(uint32_t msec)
{ static uint32_t to=0;
  uint32_t t1 = millis();
  if(t1-to<msec) {yield(); return;}
  digitalWriteFast(13,!digitalReadFast(13)); 
  to=t1;
}

/************************** logger prototypes *************************************/
#ifdef DO_LOGGER
  #include "logger.h"
  header_s header;
#endif

/************************** dsp forward prototypes *************************************/
#ifdef DO_DSP
  void dsp_init();
  void dsp_exec(int32_t *dst, int32_t *src);
#endif
  int32_t dst[N_CHAN*N_SAMP];

//======================== I2S processing ======================================

#ifdef DO_USB_AUDIO
  static int16_t waveform[2*N_SAMP]; // store for stereo usb-audio data
#endif

uint32_t i2sProcCount=0;
uint32_t i2sErrCount=0;

void i2sInProcessing(void * s, void * d)
{
  static uint16_t is_I2S=0;
  
  i2sProcCount++;
  if(is_I2S) {i2sErrCount++; return;}
  is_I2S=1;
  digitalWriteFast(1,HIGH);
  int32_t *src = (int32_t *) d;
  // for ICS43432 need first shift left to get correct MSB 
  // shift 8bit to right to get data-LSB to bit 0
  for(int ii=0; ii<N_CHAN*N_SAMP;ii++) { src[ii]<<=1; src[ii]>>=8;}
  
/*
  for(int ii=0; ii<N_SAMP;ii++)
  {
      float arg=2.0f*3.1415926535f*30.0f*(float)ii/(float) N_DAT;
      float amp=1<<12;
      src[N_CHAN*ii]  =(int32_t)(amp*sinf(arg));
      src[N_CHAN*ii+1]=0;
      src[N_CHAN*ii+2]=0;
      src[N_CHAN*ii+3]=0;
  }
*/

#ifdef DO_DSP
  dsp_exec(dst, src);
#else
  for(int ii=0;ii<N_CHAN*N_SAMP;ii++) dst[ii]=src[ii];
#endif

#ifdef DO_LOGGER
  // first logger
  logger_write((uint8_t *) src, N_CHAN*N_SAMP*sizeof(int32_t));
#endif

#ifdef DO_USB_AUDIO
  // prepare data for USB-Audio
  // extract data from I2S buffer
  int32_t *out = dst;
  for(int ii=0; ii<N_SAMP; ii++)
  {  
      waveform[2*ii]  =(int16_t)(out[ICHAN_LEFT +ii*N_CHAN]>>AUDIO_SHIFT);
      waveform[2*ii+1]=(int16_t)(out[ICHAN_RIGHT+ii*N_CHAN]>>AUDIO_SHIFT);
/*      
      float arg=2.0f*3.1415926535f*15.0f*(float)ii/(float) N_SAMP;
      float amp=(float)(1<<12);
      waveform[2*ii]   = (int16_t)(amp*sinf(arg));
      waveform[2*ii+1] = waveform[2*ii];
*/
  }
  // put data onto audioStore
  if(!audioStore.put((uint32_t *) waveform, N_SAMP)); //  2x 16-bit channels 
#endif
  digitalWriteFast(1,LOW);
  is_I2S=0;
}

#ifdef DO_DSP
/**
 *  fft_filt defines NCH (number of channels) and LL (number of samples per block)
 *  NF is number of sub-filters
 *  Let MM = 128 a 128 tab FIR filter (129 sample filter length)
 *  and use of 512 point RFFT
 *  then ACQ block size should be 384 samples (LL = NN-MM)
 *  
 *  alternatively
 *  let AcQ block size to be 217 (128*750/441)
 *  then takeing 1 buffer i.e. LL = 217 samples and MM = 39 for NN = 256 point FFT 
 *  using NF = 4 partitioned filters total FIR length is 160 = 4*40 
 *  
 *  let AcQ block size to be 435 (128*1500/441)
 *  then taking 1 buffer i.e. LL = 435 samples and MM = 77 for NN = 512 point FFT 
 *  using NF = 2 partitioned filters results in total FIR length is 156 = 2*78 
 *  using NF = 4 partitioned filters results in total FIR length is 312 = 4*78 
 */
#include "fft_conv.h"
#include "fft_filt.h"
C_CONV mConv; 

/*
 * from fft_conv.cpp
  uu = buff;    // input buffer;            size: uu[nn]
  vv = uu + nn; // spectrum result;         size: vv[nn]
  ww = vv + nn; // spectrum accumulator;    size: ww[nn]
  bb = ww + nn; // filter store;            size: bb[nch*nf*nn]
  zz = bb + nch*nf*nn;  // spectrum store;  size: zz[nch*nf*nn]
  ov = zz + nch*nf*nn;  // overlap store;   size: ov[nch*nf*(nn-ll)]
  nj = (int32_t *)ov + nch*nf*(nn-ll); // index to FDL
 */
float imp[N_FILT*(MM+1)];
float dsp_buffer[3*N_FFT + 2*N_CHAN*N_FILT*N_FFT + N_CHAN*N_FILT*(N_FFT-N_SAMP) + N_FILT];

float pwr[N_CHAN*N_FFT];

void dsp_init()
{ 
  float fc  =  5.0f/(F_SAMP/2000.0f);
  float dfc =  5.0f/(F_SAMP/2000.0f); 
  calc_FIR_coeffs(imp,N_FILT*(MM+1), fc, ASTOP, LPF, dfc);
  //
  mConv.init(imp, dsp_buffer, N_CHAN, N_FILT, N_SAMP, N_FFT, MM);
}

void dsp_exec(int32_t * dst, int32_t *src)
{
    mConv.exec_upos(dst,pwr,src);
}
#endif

/*************************** Arduino compatible Setup ************************************/
void c_myApp::setup()
{
#ifdef DO_USB_AUDIO
  AudioMemory(8);
#endif

  // wait for serial line to come up
  pinMode(13,OUTPUT);
  pinMode(13,HIGH);

#ifdef DO_DEBUG
  while(!Serial) blink(100);
  Serial.println("I2S Logger and Monitor");
#endif
  pinMode(1,OUTPUT);
  pinMode(2,OUTPUT);
  
#ifdef DO_DSP
  dsp_init();
#endif

#ifdef DO_LOGGER
  header.nch = N_CHAN;
  header.fsamp = F_SAMP;
  logger_init(&header);
#endif

//  Serial.printf("%d %d %d %d\n\r",N_DAT,N_FFT,N_FILT,MM);
  //
  // initalize and start ICS43432 interface
  uint32_t fs = ICS43432.init(F_SAMP, i2s_rx_buffer, N_BUF);
  if(fs>0)
  {
    #ifdef DO_DEBUG
      Serial.printf("Fsamp requested: %.3f kHz  got %.3f kHz\n\r" ,
            F_SAMP/1000.0f, fs/1000.0f);
    #endif
    ICS43432.start();
  }
}

/*************************** Arduino loop ************************************/
void c_myApp::loop()
{ 
  digitalWriteFast(2,HIGH);
#ifdef DO_LOGGER
  static uint16_t appState = 0;
  
  // if we are free running, then blink
  if(appState) {  blink(1000); return;}

  // otherwise run logger save
  if(logger_save() == INF)
  { ICS43432.stop();
    #ifdef DO_DEBUG
      Serial.println("stopped");
    #endif
    pinMode(13,OUTPUT); appState=1; return;
  }
#else
  static uint32_t t0=0;
  uint32_t t1=millis();
  static uint32_t loopCount=0;

  if (t1-t0>1000) 
  { static uint32_t icount=0;
  #ifdef DO_DEBUG
    Serial.printf("%4d %d %d %d %d %.3f kHz\n\r",
        icount, loopCount, i2sProcCount,i2sErrCount, N_SAMP, 
        ((float)N_SAMP*(float)i2sProcCount/1000.0f));
  #endif
    i2sProcCount=0;
    i2sErrCount=0;
    loopCount=0;
    t0=t1;
    icount++;
  }
  loopCount++;
#endif
  digitalWriteFast(2,LOW);
}


