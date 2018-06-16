//  Copyright (c) 2018 Flyingsand
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.
//
//  Created by Christian Floisand on 2018-02-13.
//
//  ----------------------------------------------------------------------------------------
//
//  Pure Data, Copyright (c) 1997-1999 Miller Puckette.
//
//  This program is free software: you can redistribute it and/or modify it under the terms
//  of the GNU General Public License as published by the Free Software Foundation, either
//  version 3 of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
//  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//  See the  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along with this program.
//  If not, see <http://www.gnu.org/licenses/>.
//

#include "m_pd.h" /* Pure Data API */


#define TWOPI (6.2831853f)

static t_class *polyblep_tilde_class;

struct _polyblep_tilde {
	t_object obj;
	
	t_float frequency;
    t_float sampleRate;
	t_float phase;
	
	t_inlet *phaseInlet; /* This inlet can be used to reset the phase or offset it. Value is clamped between 0 and TWOPI. */
	t_outlet *signalOut; /* Outputs the PolyBLEP signal. */
};

typedef struct _polyblep_tilde polyblep_tilde_t;


void*
polyblep_tilde_new (t_floatarg freq_arg, t_floatarg sr_arg) {
	polyblep_tilde_t *obj = (polyblep_tilde_t *)pd_new(polyblep_tilde_class);
	obj->frequency = freq_arg;
    obj->sampleRate = sr_arg;
	obj->phase = 0.f;
	obj->phaseInlet = floatinlet_new(&obj->obj, &obj->phase);
	obj->signalOut = outlet_new(&obj->obj, &s_signal);
    
    if (obj->sampleRate == 0.f) {
        obj->sampleRate = sys_getsr();
    }
	
#if DEBUG
    post("DEBUG: polyblep~ args: %f, %f", obj->frequency, obj->sampleRate);
#endif
    
	return (void *)obj;
}

void
polyblep_tilde_free (polyblep_tilde_t* obj) {
	inlet_free(obj->phaseInlet);
	outlet_free(obj->signalOut);
}

void
polyblep_frequency (polyblep_tilde_t* obj, t_floatarg arg) {
	obj->frequency = arg;
}

t_int*
polyblep_perform (t_int* args) {
	polyblep_tilde_t *obj = (polyblep_tilde_t *)args[1];
	t_sample *out = (t_sample *)args[2];
	int numSamples = (int)args[3];
	
    t_float normFreq = obj->frequency / obj->sampleRate;
	t_float phaseIncr = normFreq * TWOPI;
	
	obj->phase = (obj->phase < 0.f ? 0.f : (obj->phase > TWOPI ? TWOPI : obj->phase));
	
	while (numSamples--) {
		t_float t = obj->phase / TWOPI;
		t_sample sample = (2.f * t) - 1.f; /* Calculate naive sawtooth sample. */
		t_sample polyblep_value;
		
		obj->phase += phaseIncr;
		obj->phase = (obj->phase >= TWOPI ? obj->phase-TWOPI : obj->phase);
		
		polyblep_value = 0.f;
		{
			if (t < normFreq) {
				t /= normFreq;
				polyblep_value = t+t - t*t - 1.f;
			} else if (t > 1.f - normFreq) {
				t = (t - 1.f) / normFreq;
				polyblep_value = t*t + t+t + 1.f;
			}
		}
		
		*out++ = sample - polyblep_value;
	}
	
	/* Return requirement from documentation specifies that the function must return a pointer
	 to the memory directly behind the arguments list (in this case, the number of pointer 
	 arguments given (3) plus 1. */
	return (args + 4);
}

void
polyblep_dsp (polyblep_tilde_t* obj, t_signal** sp) {
	/* Signal pointer (sp) goes clockwise from the left inlet around to the left outlet,
	 but since there are no signal inlets, the output is at sp[0]. */
	dsp_add(polyblep_perform, 3, obj, sp[0]->s_vec, sp[0]->s_n);
}

void
polyblep_tilde_setup (void) {
	polyblep_tilde_class = class_new(gensym("polyblep~"),
									 (t_newmethod)polyblep_tilde_new,
									 (t_method)polyblep_tilde_free,
									 sizeof(polyblep_tilde_t), CLASS_DEFAULT,
                                     A_DEFFLOAT, A_DEFFLOAT, 0);
	
	class_addmethod(polyblep_tilde_class, (t_method)polyblep_dsp, gensym("dsp"), 0);
	/* Float messages to the left inlet modifies the waveform's frequency. */
	class_addfloat(polyblep_tilde_class, (t_method)polyblep_frequency);
}
