//Copyright 2017 by Walter Zimmer
// Version 18-05-17
//
// general teensy includes
#include "kinetis.h"
#include "core_pins.h"
#include "usb_serial.h"
//
// application specifific includes
#include "myApp.h"

/**************************** this would be in *.ino: ****************************/
#include <Audio.h>

#include "AudioTest.h"
#include "AudioInterface.h"
#include "AudioTrigger.h"

AudioTest       test;
AudioInterface  interface(375000);
AudioOutputUSB  usb;
AudioConnection patchCord1(test,interface);
AudioConnection patchCord2(interface,usb);


void c_yApp::setup() {
  // put your setup code here, to run once:
  
  AudioMemory(16);
  test.amplitude(0.1);
  // set test frequency
  // for simplicity is multiple of 0.3444 kHz (128/1089) 
  // must be after amplitude settings
  // 3 => 1.033 kHz
  test.setup(3); 
  
  pinMode(13,OUTPUT);
}

void c_myApp::loop() {
  // put your main code here, to run repeatedly:

  digitalWriteFast(13,!digitalReadFast(13));
  delay(1000);
}

/*
 * use following matlab script to generate PSD estimate
 * 
 * recObj = audiorecorder(44100,16,2);
 * nsec = 1; % number of seconds to read from audio 
 * recordblocking(recObj, nsec);
 * %
 * % Store data in double-precision array.
 * myRecording = getaudiodata(recObj);
 * %
 * % Plot the spectrum of waveform.
 * figure(1),pwelch(myRecording(:,1),[],[],[],44100)
 */

