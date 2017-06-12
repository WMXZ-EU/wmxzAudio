/*
 * WMXZ Teensy core library
 * Copyright (c) 2017 Walter Zimmer.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
// AudioTrigger.cpp
// ensures Audio updates for USB streaming
//
#include "AudioTrigger.h"

bool AudioTrigger::update_responsibility = false;

void AudioTrigger::init(void)
{	// check with AudioStream if we are responsable for updates
	update_responsibility = update_setup();

	// assign local function as ISR
	_VectorsRam[IRQ_PIT_CH0 + 16] = isr;
	NVIC_SET_PRIORITY(IRQ_PIT_CH0, prio*16);	
	NVIC_ENABLE_IRQ(IRQ_PIT_CH0);

	// turn on PIT clock
	SIM_SCGC6 |= SIM_SCGC6_PIT;
	// turn on PIT     
	PIT_MCR = 0x00;
	
	// Timer 0     
	PIT_LDVAL0 = PIT_PERIOD;     
	PIT_TCTRL0 = 2; // enable Timer 0 interrupts      
	PIT_TCTRL0 |= 1; // start Timer 0
}

void AudioTrigger::isr(void)
{
	PIT_TFLG0=1;
	if (update_responsibility) AudioStream::update_all();
}

AudioTrigger trigger;

