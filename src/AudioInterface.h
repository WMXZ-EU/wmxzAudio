/*
 * WMXZ Teensy audio library
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
// AudioInterface.h
// interfaces to Audio streaming
//

#ifndef AUDIOINTERFACE_H
#define AUDIOINTERFACE_H

//#include "kinetis.h"
//#include "core_pins.h"
//#include "usb_audio.h"
#include <Arduino.h>
#include "AudioStream.h"

class c_buff
{
	uint16_t top;
	uint16_t bot;
	uint8_t * buffer;
	uint32_t mbuf;
public:
	//
	c_buff(uint8_t *data, uint32_t len) 
	{	buffer=data; mbuf=len; top=0; bot=0; for(int ii=0; ii<mbuf; ii++) buffer[ii]=0; };
	uint16_t put(uint8_t * data, uint16_t len);
	uint16_t get(uint8_t * data, uint16_t len);
	uint16_t get_top() {return top;}
	uint16_t get_bot() {return bot;}
};

/************************ AudioInterface **************************************************************/
//
class AudioInterface : public AudioStream
{
public:
	AudioInterface(c_buff * store, int fsamp) : AudioStream(0, NULL) { init(store, fsamp);}
	virtual void update(void);
private:
	float sc, dx1;

	c_buff * audioStore;
	int32_t jfs1, jfs2;
	int32_t n_src;
	//
	void init(c_buff * store, int fsamp);
	void interpolate(int16_t *dst, const int16_t *src);
};

#endif
