/*!
 
 @file		polyblep~.c
 @brief		PolyBLEP sawtooth oscillator external for Pure Data (32-bit).
 @details	Based on the PolyBLEP oscillator discussed in this thread on KVR:
			http://www.kvraudio.com/forum/viewtopic.php?t=375517
 @author	Christian Floisand
 @date		Created: 2014/08/23
			Modified: 2014/09/03
 @copyright Copyright (C) 2014 Christian Floisand.
 
 Pure Data, Copyright (c) 1997-1999 Miller Puckette.
 
 This program is free software: you can redistribute it and/or modify it under the terms
 of the GNU General Public License as published by the Free Software Foundation, either
 version 3 of the License, or (at your option) any later version.
 
 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 See the  GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License along with this program.
 If not, see <http://www.gnu.org/licenses/>.
 
 */

#include "m_pd.h" /* Pure Data API */


#define TWOPI (6.2831853f)

static t_class *polyblep_tilde_class;

struct _polyblep_tilde {
	t_object _obj;
	
	t_float _frequency;
	t_float _phase;
	
	t_inlet *_phaseInlet; /* This inlet can be used to reset the phase or offset it.
						   Value is clamped between 0 and TWOPI. */
	t_outlet *_signalOut; /* Outputs the PolyBLEP signal. */
};

typedef struct _polyblep_tilde t_polyblep_tilde;


void* polyblep_tilde_new (t_floatarg arg)
{
	t_polyblep_tilde *obj = (t_polyblep_tilde *)pd_new(polyblep_tilde_class);
	obj->_frequency = arg;
	obj->_phase = 0.f;
	obj->_phaseInlet = floatinlet_new(&obj->_obj, &obj->_phase);
	obj->_signalOut = outlet_new(&obj->_obj, &s_signal);
	
	return (void *)obj;
}

void polyblep_tilde_free (t_polyblep_tilde* obj)
{
	inlet_free(obj->_phaseInlet);
	outlet_free(obj->_signalOut);
}

void polyblep_frequency (t_polyblep_tilde* obj, t_floatarg arg)
{
	obj->_frequency = arg;
}

t_int* polyblep_perform (t_int* args)
{
	t_polyblep_tilde *obj = (t_polyblep_tilde *)args[1];
	t_sample *out = (t_sample *)args[2];
	int numSamples = (int)args[3];
	
	/* Calculate the normalized frequency using the system's sample rate, since output 
	 should be at this sample rate (i.e. as opposed to using the signal pointer's sample rate). */
	t_float normFreq = obj->_frequency / sys_getsr();
	t_float phaseIncr = normFreq * TWOPI;
	
	obj->_phase = (obj->_phase < 0.f ? 0.f : (obj->_phase > TWOPI ? TWOPI : obj->_phase));
	
	while (numSamples--) {
		t_float t = obj->_phase / TWOPI;
		t_sample sample = (2.f * t) - 1.f; /* Calculate naive sawtooth sample. */
		t_sample polyblep_value;
		
		obj->_phase += phaseIncr;
		obj->_phase = (obj->_phase >= TWOPI ? obj->_phase-TWOPI : obj->_phase);
		
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

void polyblep_dsp (t_polyblep_tilde* obj, t_signal** sp)
{
	/* Signal pointer (sp) goes clockwise from the left inlet around to the left outlet,
	 but since there are no signal inlets, the output is at sp[0]. */
	dsp_add(polyblep_perform, 3, obj, sp[0]->s_vec, sp[0]->s_n);
}

void polyblep_tilde_setup (void)
{
	polyblep_tilde_class = class_new(gensym("polyblep~"),
									 (t_newmethod)polyblep_tilde_new,
									 (t_method)polyblep_tilde_free,
									 sizeof(t_polyblep_tilde), CLASS_DEFAULT, A_DEFFLOAT, 0);
	
	class_addmethod(polyblep_tilde_class, (t_method)polyblep_dsp, gensym("dsp"), 0);
	/* Float messages to the left inlet modifies the waveform's frequency. */
	class_addfloat(polyblep_tilde_class, (t_method)polyblep_frequency);
}
