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
// AudioInterface.cpp
// interfaces to Audio streaming
//

#include "AudioInterface.h"

uint16_t c_buff::put(uint8_t * data, uint16_t len)
{ int nn;
   // consider overrun if top is len bytes below bot
  nn=(top+len-bot+mbuf) % mbuf;
  if( nn < len) return 0;
  //
  if((top+len)<mbuf)
  {  nn=len;
     for(int ii=0; ii<nn; ii++) buffer[top++]=*data++;
  }
  else
  {  nn=mbuf-top;
     for(int ii=0; ii<nn; ii++) buffer[top++]=*data++;
     top=0;
     for(int ii=0; ii<len-nn; ii++) buffer[top++]=*data++;
  }
  return len;
}

//
uint16_t c_buff::get(uint8_t * data, uint16_t len)
{  int nn;
  nn=(top-bot+mbuf) % mbuf;
  if( nn < len) return 0;
  //
  if((bot+len)<mbuf)
  {  nn=len;
     for(int ii=0; ii<nn; ii++) *data++ = buffer[bot++];
  }
  else
  {  nn=mbuf-bot;
     for(int ii=0; ii<nn; ii++) *data++ = buffer[bot++];
     bot=0;
     for(int ii=0; ii<len-nn; ii++) *data++ = buffer[bot++];
  }
 
  return len;
}

static uint8_t audioBuffer[4*4*139*4];

c_buff audioStore(audioBuffer,sizeof(audioBuffer));

/************************ AudioInterface **************************************************************/

void AudioInterface::init(int fsamp)
{ 	jfs1=fsamp;
	jfs2=44100;
	while( ((jfs1 % 10) ==0) && ((jfs2 % 10))==0) { jfs1 /= 10; jfs2 /= 10; } 
	jfs1 *= 10; jfs2 *= 10;
	isc = jfs1 % jfs2;
	n_src=139;//(uint16_t)((AUDIO_BLOCK_SAMPLES*jfs1)/jfs2); // number od samples in src_buffer
}

void AudioInterface::interpolate(int16_t *dst, const int16_t *src, int dj)
{
	for(int ii=0; ii<AUDIO_BLOCK_SAMPLES; ii++) dst[ii]=0;
	for(int ii=0; ii<AUDIO_BLOCK_SAMPLES; ii++)
	{	
		int32_t i3 = (ii*jfs1 + dj+ (1+jfs2)/2)/jfs2;
		
		int32_t idt1 = (ii*jfs1 + dj) % jfs2; 
		int32_t idt2, P1,P2;
		
		int j0,j1,j2;
		if(ii==0) // interpolate relative to left end
		{	j0=i3; j1=j0+1; j2=j0+2;
			idt2 = idt1-jfs2;
			P1 = (src[2*j1] - src[2*j0]);
			P2 = (src[2*j2] - 2*src[2*j1] + src[2*j0])/2;
		}
		else if(ii==AUDIO_BLOCK_SAMPLES-1)// extrapolate to right 
		{
			j0=i3-2; j1=j0+1; j2=j0+2;
			idt2 = idt1+jfs2;
			P1 = (src[2*j2] - src[2*j1]);
			P2 = (src[2*j2] - 2*src[2*j1] + src[2*j0])/2;
		}
		else // interpolate right of middle point
		{
			j0=i3-1; j1=j0+1; j2=j0+2;
			idt2 = idt1-jfs2;
			P1 = (src[2*j2] - src[2*j1]);
			P2 = (src[2*j2] - 2*src[2*j1] + src[2*j0])/2;
		}
		//
		dst[ii] = src[2*i3] + (idt1 * (P1 + (idt2*P2)/jfs2))/jfs2;		
	}
}

void AudioInterface::update(void)
{	const int16_t *src;
	int16_t *dst;
	audio_block_t *left, *right;
	//
	static int nn=0;
	//
	uint32_t dj =(nn*(128*jfs1-n_src*jfs2)) % jfs2; // sample offset cross src buffer

	int n2=audioStore.get((uint8_t *)src_buffer,4*n_src); // number of bytes for stereo
	
	// check if we have sufficient data
	if(n2 < 4*n_src) return;
	
	left = allocate();  if (!left) return;
	right = allocate(); if (!right) return;
	
	src = &src_buffer[0];
	dst = left->data;
	//
	interpolate(dst, src, dj);
	
	src = &src_buffer[1];
	dst = right->data;
	//
	interpolate(dst, src, dj);
	
	nn++;
	nn %= jfs2;
	transmit(left,0);
	transmit(right,1);
	release(left);
	release(right);
}
