//Copyright 2017 by Walter Zimmer
// Version 11-06-17
// in addition to teensy core
// this application needs support from
// wmxzCore
// wmxzDevices
// wmxzAudio

//
// general teensy includes
#include "kinetis.h"
#include "core_pins.h"
#include "usb_serial.h"
//
// application specifific includes
#include "myApp.h"

/****************************************************************************************/
#include <AudioStream.h>
//
#include "AudioInterface.h"
#include "AudioTrigger.h"

AudioInterface  interface(48000);
AudioOutputUSB  usb;
AudioConnection patchCord1(interface,0,usb,0);
AudioConnection patchCord2(interface,1,usb,1); // comment for mono

/***************************************************************/
#include "ICS43432.h"

c_ICS43432 ICS43432;

extern "C" void i2sInProcessing(void * s, void * d);

#define N_CHAN 4    // number of channels
#define N_DAT 139   // number of samples per received DMA interrupt  (128*480/441)
#define N_BUF 2 * N_CHAN * N_DAT    // dual buffer size for DMA 
int32_t i2s_rx_buffer[N_BUF];       // buffer for DMA

void c_myApp::setup()
{
   AudioMemory(8);
   
  // initalize and start ICS43432 interface
  ICS43432.init(i2s_rx_buffer, N_BUF);
  ICS43432.start();
}

void c_myApp::loop()
{ 
}

#define AUDIO_NBUF N_DAT
#define AUDIO_SHIFT 8 // for 24 bit to 16 bit conversion (should be larger/equal of 8
#define ICHAN_LEFT  0
#define ICHAN_RIGHT 1
static int16_t waveform[2*AUDIO_NBUF];

// circular storage where audio data are stored 
extern c_buff audioStore;

// following is called from I2S ISR
void i2sInProcessing(void * s, void * d)
{
	static uint16_t is_I2S=0;
	if(is_I2S) return;
	is_I2S=1;

	// extract data from I2S buffer
	int32_t *src = (int32_t *) d;
	for(int ii=0; ii<AUDIO_NBUF; ii++)
	{  
	    waveform[2*ii]  =(int16_t)(src[ICHAN_LEFT +ii*N_CHAN]>>AUDIO_SHIFT);
		waveform[2*ii+1]=(int16_t)(src[ICHAN_RIGHT+ii*N_CHAN]>>AUDIO_SHIFT);
      
//      float arg=2.0f*3.1415926535f*3.0f*(float)ii/(float) AUDIO_NBUF;
//      float amp=1<<3;
//      waveform[2*ii]  =(int16_t)(amp*sinf(arg));
//      waveform[2*ii+1]=(int16_t)(amp*sinf(arg));
	}
	// put data onto audioStore
	audioStore.put((uint8_t *) waveform, 2*2*AUDIO_NBUF);
	//
	is_I2S=0;
}