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

#ifndef filter_biquad_f_h_
#define filter_biquad_f_h_

#include "Arduino.h"
#include "AudioStream.h"

typedef struct
{   uint32_t numStages;      /**< number of 2nd order stages in the filter.  Overall order is 2*numStages. */
    float *pState;       /**< Points to the array of state coefficients.  The array is of length 4*numStages. */
    const float *pCoeffs;      /**< Points to the array of coefficients.  The array is of length 5*numStages. */
} biquad_df1_inst_f32;

class AudioFilterBiquad_F : public AudioStream
{
public:
	AudioFilterBiquad_F(void) : AudioStream(1, inputQueueArray) 
	{
	    for (int i=0; i<4*4; i++) pState[i] = 0;
	}
	virtual void update(void);

	// Set the biquad coefficients directly
	void setCoefficients(uint32_t stage, const float *coefficients);

  // Compute common filter functions
  // http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
  // wmxz: similar to filter_biquad.h but modified for float
  void setLowpass(float *coef, uint32_t stage, float frequency, float q = 0.7071) {
    double w0 = frequency * (2 * 3.141592654 / AUDIO_SAMPLE_RATE_EXACT);
    double sinW0 = sin(w0);
    double alpha = sinW0 / ((double)q * 2.0);
    double cosW0 = cos(w0);
    double scale = 1.0 / (1.0 + alpha);
    /* b0 */ coef[0] = ((1.0 - cosW0) / 2.0) * scale;
    /* b1 */ coef[1] = (1.0 - cosW0) * scale;
    /* b2 */ coef[2] = coef[0];
    /* a1 */ coef[3] = (-2.0 * cosW0) * scale;
    /* a2 */ coef[4] = (1.0 - alpha) * scale;
  }
  void setHighpass(float *coef, uint32_t stage, float frequency, float q = 0.7071) {
    double w0 = frequency * (2 * 3.141592654 / AUDIO_SAMPLE_RATE_EXACT);
    double sinW0 = sin(w0);
    double alpha = sinW0 / ((double)q * 2.0);
    double cosW0 = cos(w0);
    double scale = 1.0 / (1.0 + alpha);
    /* b0 */ coef[0] = ((1.0 + cosW0) / 2.0) * scale;
    /* b1 */ coef[1] = -(1.0 + cosW0) * scale;
    /* b2 */ coef[2] = coef[0];
    /* a1 */ coef[3] = (-2.0 * cosW0) * scale;
    /* a2 */ coef[4] = (1.0 - alpha) * scale;
  }
  void setBandpass(float *coef, uint32_t stage, float frequency, float q = 1.0) {
    double w0 = frequency * (2 * 3.141592654 / AUDIO_SAMPLE_RATE_EXACT);
    double sinW0 = sin(w0);
    double alpha = sinW0 / ((double)q * 2.0);
    double cosW0 = cos(w0);
    double scale = 1.0 / (1.0 + alpha);
    /* b0 */ coef[0] = alpha * scale;
    /* b1 */ coef[1] = 0;
    /* b2 */ coef[2] = (-alpha) * scale;
    /* a1 */ coef[3] = (-2.0 * cosW0) * scale;
    /* a2 */ coef[4] = (1.0 - alpha) * scale;
  }
  void setNotch(float *coef, uint32_t stage, float frequency, float q = 1.0) {
    double w0 = frequency * (2 * 3.141592654 / AUDIO_SAMPLE_RATE_EXACT);
    double sinW0 = sin(w0);
    double alpha = sinW0 / ((double)q * 2.0);
    double cosW0 = cos(w0);
    double scale = 1.0 / (1.0 + alpha);
    /* b0 */ coef[0] = scale;
    /* b1 */ coef[1] = (-2.0 * cosW0) * scale;
    /* b2 */ coef[2] = coef[0];
    /* a1 */ coef[3] = (-2.0 * cosW0) * scale;
    /* a2 */ coef[4] = (1.0 - alpha) * scale;
  }
  void setLowShelf(float *coef, uint32_t stage, float frequency, float gain, float slope = 1.0f) {
    double a = pow(10.0, gain/40.0);
    double w0 = frequency * (2 * 3.141592654 / AUDIO_SAMPLE_RATE_EXACT);
    double sinW0 = sin(w0);
    //double alpha = (sinW0 * sqrt((a+1/a)*(1/slope-1)+2) ) / 2.0;
    double cosW0 = cos(w0);
    //generate three helper-values (intermediate results):
    double sinsq = sinW0 * sqrt( (pow(a,2.0)+1.0)*(1.0/slope-1.0)+2.0*a );
    double aMinus = (a-1.0)*cosW0;
    double aPlus = (a+1.0)*cosW0;
    double scale = 1.0 / ( (a+1.0) + aMinus + sinsq);
    /* b0 */ coef[0] =    a * ( (a+1.0) - aMinus + sinsq  ) * scale;
    /* b1 */ coef[1] =  2.0*a * ( (a-1.0) - aPlus       ) * scale;
    /* b2 */ coef[2] =    a * ( (a+1.0) - aMinus - sinsq  ) * scale;
    /* a1 */ coef[3] = -2.0*  ( (a-1.0) + aPlus     ) * scale;
    /* a2 */ coef[4] =        ( (a+1.0) + aMinus - sinsq  ) * scale;
  }
  void setHighShelf(float *coef, uint32_t stage, float frequency, float gain, float slope = 1.0f) {
    double a = pow(10.0, gain/40.0);
    double w0 = frequency * (2 * 3.141592654 / AUDIO_SAMPLE_RATE_EXACT);
    double sinW0 = sin(w0);
    //double alpha = (sinW0 * sqrt((a+1/a)*(1/slope-1)+2) ) / 2.0;
    double cosW0 = cos(w0);
    //generate three helper-values (intermediate results):
    double sinsq = sinW0 * sqrt( (pow(a,2.0)+1.0)*(1.0/slope-1.0)+2.0*a );
    double aMinus = (a-1.0)*cosW0;
    double aPlus = (a+1.0)*cosW0;
    double scale = 1.0 / ( (a+1.0) - aMinus + sinsq);
    /* b0 */ coef[0] =    a * ( (a+1.0) + aMinus + sinsq  ) * scale;
    /* b1 */ coef[1] = -2.0*a * ( (a-1.0) + aPlus       ) * scale;
    /* b2 */ coef[2] =    a * ( (a+1.0) + aMinus - sinsq  ) * scale;
    /* a1 */ coef[3] =  2.0*  ( (a-1.0) - aPlus     ) * scale;
    /* a2 */ coef[4] =      ( (a+1.0) - aMinus - sinsq  ) * scale;
  }

private:

  float pState[4*4];

	audio_block_t *inputQueueArray[1];

  biquad_df1_inst_f32 biquadS;

  float wrk[AUDIO_BLOCK_SAMPLES]; // working buffer for biquad

  // 
  void biquad_df1_init_f32(
        uint8_t numStages,  const float * pCoeffs, float * pState);
    
  void biquad_df1_f32( float * pDst, uint32_t blockSize);

};

#endif
