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
 // WMXZ: modified for use of floating point biquad

#include "filter_biquad_f.h"
#include "utility/dspinst.h"


#if defined(KINETISK)

// WMXZ: following adapted from CMSIS (slight name changes)
void AudioFilterBiquad_F::biquad_df1_init_f32(
  uint8_t numStages,  const float * pCoeffs,  float * pState)
{
  /* Assign filter stages */
  biquadS.numStages = numStages;
  /* Assign coefficient pointer */
  biquadS.pCoeffs = pCoeffs;
  /* Clear state buffer and size is always 4 * numStages */
  memset(pState, 0, (4u * (uint32_t) numStages) * sizeof(float));
  /* Assign state pointer */
  biquadS.pState = pState;
}

// WMXZ: following adapted from CMSIS (slight name changes)
// modified to be completely in-place
// removed cortex M0 code
void AudioFilterBiquad_F::biquad_df1_f32(float * pDst,  uint32_t blockSize)
{
  float *pIn = pDst;                         /*  source pointer            */
  float *pOut = pDst;                        /*  destination pointer       */
  float acc;                                 /*  Simulates the accumulator */
  float b0, b1, b2, a1, a2;                  /*  Filter coefficients       */
  float Xn1, Xn2, Yn1, Yn2;                  /*  Filter pState variables   */
  float Xn;                                  /*  temporary input           */
  uint32_t sample, stage = biquadS.numStages;         /*  loop counters             */
  float *pState = biquadS.pState;                 /*  pState pointer            */
  const float *pCoeffs = biquadS.pCoeffs;               /*  coefficient pointer       */

  do
  {
    /* Reading the coefficients */
    b0 = *pCoeffs++;
    b1 = *pCoeffs++;
    b2 = *pCoeffs++;
    a1 = *pCoeffs++;
    a2 = *pCoeffs++;

    /* Reading the pState values */
    Xn1 = pState[0];
    Xn2 = pState[1];
    Yn1 = pState[2];
    Yn2 = pState[3];

    sample = blockSize >> 2u;

    while(sample > 0u)
    {
      Xn = *pIn++;
      Yn2 = (b0 * Xn) + (b1 * Xn1) + (b2 * Xn2) - (a1 * Yn1) - (a2 * Yn2);
      *pOut++ = Yn2;

      Xn2 = *pIn++;
      Yn1 = (b0 * Xn2) + (b1 * Xn) + (b2 * Xn1) - (a1 * Yn2) - (a2 * Yn1);
      *pOut++ = Yn1;

      Xn1 = *pIn++;
      Yn2 = (b0 * Xn1) + (b1 * Xn2) + (b2 * Xn) - (a1 * Yn1) - (a2 * Yn2);
      *pOut++ = Yn2;

      Xn = *pIn++;
      Yn1 = (b0 * Xn) + (b1 * Xn1) + (b2 * Xn2) - (a1 * Yn2) - (a2 * Yn1);
      *pOut++ = Yn1;

      Xn2 = Xn1;
      Xn1 = Xn;

      /* decrement the loop counter */
      sample--;
    }

    /* If the blockSize is not a multiple of 4, compute any remaining output samples here.    
     ** No loop unrolling is used. */
    sample = blockSize & 0x3u;

    while(sample > 0u)
    {
      Xn = *pIn++;
      acc = (b0 * Xn) + (b1 * Xn1) + (b2 * Xn2) - (a1 * Yn1) - (a2 * Yn2);
      *pOut++ = acc;
      Xn2 = Xn1;
      Xn1 = Xn;
      Yn2 = Yn1;
      Yn1 = acc;
      sample--;
    }

    /*  Store the updated state variables back into the pState array */
    *pState++ = Xn1;
    *pState++ = Xn2;
    *pState++ = Yn1;
    *pState++ = Yn2;

    /*  The first stage goes from the input buffer to the output buffer. */
    /*  Subsequent numStages  occur in-place in the output buffer */
    pIn = pDst;
    /* Reset the output pointer */
    pOut = pDst;

    /* decrement the loop counter */
    stage--;

  } while(stage > 0u);
}


void AudioFilterBiquad_F::update(void)
{
	audio_block_t *block;

	block = receiveWritable();
	if (!block) return;

  
  for(int ii=0; ii<AUDIO_BLOCK_SAMPLES; ii++) 
  { wrk[ii]=block->data[ii]; ii++;
    wrk[ii]=block->data[ii]; ii++;
    wrk[ii]=block->data[ii]; ii++;
    wrk[ii]=block->data[ii];
  }

  biquad_df1_f32(wrk, AUDIO_BLOCK_SAMPLES);

  for(int ii=0; ii<AUDIO_BLOCK_SAMPLES; ii++) 
  { block->data[ii] = wrk[ii]; ii++;
    block->data[ii] = wrk[ii]; ii++;
    block->data[ii] = wrk[ii]; ii++;
    block->data[ii] = wrk[ii];
  }

	transmit(block);
	release(block);
}

void AudioFilterBiquad_F::setCoefficients(uint32_t stages, const float *coefficients)
{
	if (stages > 4) return;

 biquad_df1_init_f32(4,coefficients, pState);

}

#elif defined(KINETISL)

void AudioFilterBiquad_F::update(void)
{
  audio_block_t *block;

	block = receiveReadOnly();
	if (block) release(block);
}

#endif
