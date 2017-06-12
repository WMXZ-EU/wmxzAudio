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
// AudioTrigger.h
// ensures Audio updates for USB streaming
//

#ifndef AudioTrigger_h_
#define AudioTrigger_h_

#include "kinetis.h"
#include "core_pins.h"
//
#include "AudioStream.h"

// from Audio/utility/pdb.h
#if F_BUS == 120000000
  #define PDB_PERIOD (2720-1)
#elif F_BUS == 108000000
  #define PDB_PERIOD (2448-1)
#elif F_BUS == 96000000
  #define PDB_PERIOD (2176-1)
#elif F_BUS == 90000000
  #define PDB_PERIOD (2040-1)
#elif F_BUS == 80000000
  #define PDB_PERIOD (1813-1)  // small ?? error
#elif F_BUS == 72000000
  #define PDB_PERIOD (1632-1)
#elif F_BUS == 64000000
  #define PDB_PERIOD (1451-1)  // small ?? error
#elif F_BUS == 60000000
  #define PDB_PERIOD (1360-1)
#elif F_BUS == 56000000
  #define PDB_PERIOD (1269-1)  // 0.026% error
#elif F_BUS == 54000000
  #define PDB_PERIOD (1224-1)
#elif F_BUS == 48000000
  #define PDB_PERIOD (1088-1)
#elif F_BUS == 40000000
  #define PDB_PERIOD (907-1)  // small ?? error
#elif F_BUS == 36000000
  #define PDB_PERIOD (816-1)
#elif F_BUS == 24000000
  #define PDB_PERIOD (544-1)
#elif F_BUS == 16000000
  #define PDB_PERIOD (363-1)  // 0.092% error
#else
  #error "Unsupported F_BUS speed"
#endif

// correct PDB_Period to PIT_PERIOD
#define PIT_PERIOD ((PDB_PERIOD+1)*128 -1)

class AudioTrigger : public AudioStream
{
public:
        AudioTrigger() : AudioStream(0, NULL) {init(); prio = 8;}
		void init(void);
        virtual void update(void) {;}
private:
        static bool update_responsibility;
		static void isr(void);
		int prio;
};

#endif

