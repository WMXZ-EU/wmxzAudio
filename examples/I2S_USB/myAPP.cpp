//Copyright 2017 by Walter Zimmer
// Version 11-06-17
// in addition to teensy core
// this application needs support from
// wmxzCore
// wmxzDevices
// wmxzAudio

// 02-july-2017: added quad ICS connections
// T3.6 		Mic1 	Mic2 	Mic3 	Mic4
// GND			GND		GND		GND		GND
// 3.3V			VCC		VCC		VCC		VCC
// Pin11		CLK		CLK		CLK		CLK
// Pin12		WS		WS		WS		WS
// Pin13		SD		SD		--		--
// Pin38		--		--		SD		SD
// 		 L/R	GND		VCC		GND		VCC
//
// for T3.2 replace Pin38 by Pin30
//
// general teensy includes
#include "kinetis.h"
#include "core_pins.h"
#include "usb_serial.h"
//
// application specifific includes
#include "myApp.h"

/****************************************************************************************/
#include "ICS43432.h"

#define F_SAMP 48000

c_ICS43432 ICS43432;

extern "C" void i2sInProcessing(void * s, void * d);

#define N_CHAN 4    // number of channels
#define N_SAMP 128  // number of samples per aquisition

#define N_DAT (128*(F_SAMP/100)/441)   // number of samples per USB buffer 
#define N_BUF 2 * N_CHAN * N_SAMP    // dual buffer size for DMA 
int32_t i2s_rx_buffer[N_BUF];       // buffer for DMA

/****************************************************************************************/
#include <AudioStream.h>
//
#include "AudioInterface.h"
#include "AudioTrigger.h"

static uint32_t audioBuffer[4*N_DAT]; // 4 buffers for 2 int16 channels
c_buff audioStore(audioBuffer,sizeof(audioBuffer)/4);

AudioInterface  interface(&audioStore, F_SAMP);
AudioOutputUSB  usb;
AudioConnection patchCord1(interface,0,usb,0);
AudioConnection patchCord2(interface,1,usb,1); // comment for mono

/****************************************************************************************/

void c_myApp::setup()
{
   AudioMemory(8);
   
  // initalize and start ICS43432 interface
  if(ICS43432.init(F_SAMP, i2s_rx_buffer, N_BUF))
	ICS43432.start();
}

void c_myApp::loop()
{ 
}

#define AUDIO_SHIFT 8 // for 24 bit to 16 bit conversion (should be larger/equal of 8
#define ICHAN_LEFT  0
#define ICHAN_RIGHT 1
static int16_t waveform[2*N_SAMP];

// following is called from I2S ISR
void i2sInProcessing(void * s, void * d)
{
	static uint16_t is_I2S=0;
	if(is_I2S) return;
	is_I2S=1;

	int32_t *src = (int32_t *) d;
	// for ICS43432 need to shift left to get correct MSB 
	for(int ii=0; ii<N_CHAN*N_SAMP;) 
	{ src[ii++]<<=1; src[ii++]<<=1;src[ii++]<<=1; src[ii++]<<=1;}

	// extract data from I2S buffer
	for(int ii=0; ii<N_SAMP; ii++)
	{  
	    waveform[2*ii]  =(int16_t)(src[ICHAN_LEFT +ii*N_CHAN]>>AUDIO_SHIFT);
	    waveform[2*ii+1]=(int16_t)(src[ICHAN_RIGHT+ii*N_CHAN]>>AUDIO_SHIFT);
/*      
      float arg=2.0f*3.1415926535f*3.0f*(float)ii/(float) N_SAMP;
      float amp=1<<12;
      waveform[2*ii]  =(int16_t)(amp*sinf(arg));
      waveform[2*ii+1]=(int16_t)(amp*sinf(arg));
*/
	}
	// put data onto audioStore
	audioStore.put((uint32_t *) waveform, N_SAMP);
	//
	is_I2S=0;
}
