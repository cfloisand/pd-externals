/*!
 
 @file		foldback~.c
 @brief		Foldback distortion external for Pure Data (32-bit).
 @details
 @author	Christian Floisand
 @date		Created: 2014/12/09
            Modified: 2014/12/09
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
#include <math.h>

static t_class *foldback_tilde_class;

struct _foldback_tilde {
    t_object _obj;
    
    t_float _threshold;
    t_float _f;
    
    t_inlet *_inThreshold; /* Inlet for controlling threshold. */
    t_outlet *_outSignal; /* Outputs the signal after applying foldback distortion. */
};

typedef struct _foldback_tilde t_foldback_tilde;

void* foldback_tilde_new (t_floatarg arg)
{
    t_foldback_tilde *obj = (t_foldback_tilde *)pd_new(foldback_tilde_class);
    obj->_threshold = arg;
    obj->_inThreshold = floatinlet_new(&obj->_obj, &obj->_threshold);
    obj->_outSignal = outlet_new(&obj->_obj, &s_signal);
    
    return (void *)obj;
}

void foldback_tilde_free (t_foldback_tilde* obj)
{
    inlet_free(obj->_inThreshold);
    outlet_free(obj->_outSignal);
}

t_int* foldback_perform (t_int* args)
{
    t_foldback_tilde *obj = (t_foldback_tilde *)args[1];
    t_sample *in = (t_sample *)args[2];
    t_sample *out = (t_sample *)args[3];
    int numSamples = (int)args[4];
    
    t_float threshold = obj->_threshold;
    
    while (numSamples--) {
        t_sample sample = *in++;
        t_sample outSample = sample;

        if ((sample > threshold) || (sample < -threshold)) {
            outSample = fabsf(fabsf(fmodf(sample - threshold, threshold * 4.f)) - threshold * 2.f) - threshold;
        }
        
        *out++ = outSample;
    }
    
    /* Return requirement from documentation specifies that the function must return a pointer
     to the memory directly behind the arguments list (in this case, the number of pointer
     arguments given (4) plus 1. */
    return (args + 5);
}

void foldback_dsp (t_foldback_tilde* obj, t_signal** sp)
{
    /* Signal pointer (sp) goes clockwise from the left inlet around to the left outlet. 
     The first (0) is the signal inlet, and the next (1) is the signal outlet. */
    dsp_add(foldback_perform, 4, obj, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void foldback_tilde_setup (void)
{
    foldback_tilde_class = class_new(gensym("foldback~"),
                                     (t_newmethod)foldback_tilde_new,
                                     (t_method)foldback_tilde_free,
                                     sizeof(t_foldback_tilde), CLASS_DEFAULT, A_DEFFLOAT, 0);
    
    class_addmethod(foldback_tilde_class, (t_method)foldback_dsp, gensym("dsp"), 0);
    /* Float messages to the left inlet modifies the waveform's frequency. */
    CLASS_MAINSIGNALIN(foldback_tilde_class, t_foldback_tilde, _f);
}
