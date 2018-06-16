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
#include <math.h>

static t_class *foldback_tilde_class;

struct _foldback_tilde {
    t_object obj;
    
    t_float threshold;
    t_float f;
    
    t_inlet *inThreshold; /* Inlet for controlling threshold. */
    t_outlet *outSignal; /* Outputs the signal after applying foldback distortion. */
};

typedef struct _foldback_tilde foldback_tilde_t;


void*
foldback_tilde_new (t_floatarg arg) {
    foldback_tilde_t *obj = (foldback_tilde_t *)pd_new(foldback_tilde_class);
    obj->threshold = arg;
    obj->inThreshold = floatinlet_new(&obj->obj, &obj->threshold);
    obj->outSignal = outlet_new(&obj->obj, &s_signal);
    
#if DEBUG
    post("DEBUG: foldback~ args: %f", obj->threshold);
#endif
    
    return (void *)obj;
}

void
foldback_tilde_free (foldback_tilde_t* obj) {
    inlet_free(obj->inThreshold);
    outlet_free(obj->outSignal);
}

t_int*
foldback_perform (t_int* args) {
    foldback_tilde_t *obj = (foldback_tilde_t *)args[1];
    t_sample *in = (t_sample *)args[2];
    t_sample *out = (t_sample *)args[3];
    int numSamples = (int)args[4];
    
    t_float threshold = obj->threshold;
    
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

void
foldback_dsp (foldback_tilde_t* obj, t_signal** sp) {
    /* Signal pointer (sp) goes clockwise from the left inlet around to the left outlet. 
     The first (0) is the signal inlet, and the next (1) is the signal outlet. */
    dsp_add(foldback_perform, 4, obj, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void
foldback_tilde_setup (void) {
    foldback_tilde_class = class_new(gensym("foldback~"),
                                     (t_newmethod)foldback_tilde_new,
                                     (t_method)foldback_tilde_free,
                                     sizeof(foldback_tilde_t), CLASS_DEFAULT, A_DEFFLOAT, 0);
    
    class_addmethod(foldback_tilde_class, (t_method)foldback_dsp, gensym("dsp"), 0);
    /* Float messages to the left inlet modifies the waveform's frequency. */
    CLASS_MAINSIGNALIN(foldback_tilde_class, foldback_tilde_t, f);
}
