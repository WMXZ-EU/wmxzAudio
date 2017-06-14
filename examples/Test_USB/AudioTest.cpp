/* Audio Library for Teensy 3.X
 * Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com
 *
 * Development of this audio library was funded by PJRC.COM, LLC by sales of
 * Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
 * open source software by purchasing Teensy or other PJRC products.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
 
 // WMXZ modified

#include "AudioInterface.h"
#include "AudioTest.h"

#define AUDIO_NBUF (128*3750/441) // sufficient to hold 128*3750/441 samples
#define TPI 6.2831853072f

static int16_t waveform[2*AUDIO_NBUF];

extern c_buff audioStore;

void AudioTest::setup(int freq)
{
	for(int ii=0; ii<AUDIO_NBUF; ii++)
	{ float phase = TPI*(float) freq/(float)AUDIO_NBUF;
	  waveform[2*ii]=magnitude*sinf(ii*phase);
    waveform[2*ii+1]=waveform[2*ii];
	}
  // pur some data onto audioStore
  audioStore.put((uint8_t *) waveform, 2*2*AUDIO_NBUF);
}

void AudioTest::update(void)
{ // update audioStore in sync with Audio updata
	audioStore.put((uint8_t *) waveform, 2*2*AUDIO_NBUF);
}

