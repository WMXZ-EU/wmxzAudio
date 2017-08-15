#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include "filter_biquad_f.h"

// GUItool: begin automatically generated code
AudioSynthNoiseWhite     noise1;         //xy=165,99
AudioSynthNoiseWhite     noise2;         //xy=173,205
AudioSynthWaveformSine   sine1;          //xy=175,298
AudioSynthNoiseWhite     noise3;         //xy=176,390
AudioFilterBiquad_F        biquad3;        //xy=342,389
AudioFilterBiquad        biquad1;        //xy=352,98
AudioFilterBiquad        biquad2;        //xy=352,203
AudioEffectMultiply      multiply1;      //xy=366,291
AudioMixer4              mixer1;         //xy=568,152
AudioOutputI2S           i2s1;           //xy=749,154
AudioConnection          patchCord1(noise1, biquad1);
AudioConnection          patchCord2(noise2, biquad2);
AudioConnection          patchCord3(sine1, 0, multiply1, 0);
AudioConnection          patchCord4(noise3, biquad3);
AudioConnection          patchCord5(biquad3, 0, multiply1, 1);
AudioConnection          patchCord6(biquad1, 0, mixer1, 0);
AudioConnection          patchCord7(biquad2, 0, mixer1, 1);
AudioConnection          patchCord8(multiply1, 0, mixer1, 2);
AudioConnection          patchCord9(mixer1, 0, i2s1, 0);
AudioConnection          patchCord10(mixer1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=620,442
// GUItool: end automatically generated code

float Coeff[4*5];

void setup() {
  // Audio requires memory to work.
  AudioMemory(16);

  AudioNoInterrupts();

    for(int ii=0;ii<4;ii++)
    {
      biquad1.setBandpass(ii, 400.0f, 5.0f);
      biquad2.setBandpass(ii, 500.0f, 5.0f);
    }
    for(int ii=0;ii<4;ii++)
      biquad3.setBandpass(&Coeff[ii*5],ii, 100.0f, 10.0f);
    biquad3.setCoefficients(4,(const float*)Coeff);
  //
  noise1.amplitude(0.8);
  noise2.amplitude(0.8);
  noise3.amplitude(0.8);
  
  sine1.amplitude(0.8);
  sine1.frequency(800.0);

  mixer1.gain(0,1.0);
  mixer1.gain(1,1.0);
  mixer1.gain(2,1.0);
  mixer1.gain(3,0.0);

  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);
  AudioInterrupts();
}


void loop() {
}


