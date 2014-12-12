/*!
 
 @file		tubedist~.c
 @brief		Tube distortion external for Pure Data (32-bit).
 @details
 @author	Christian Floisand
 @date		Created: 2014/12/10
            Modified: 2014/12/11
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

static t_class *tubedist_tilde_class;

struct _tubedist_tilde {
    t_object _obj;
    
    t_float _pos;
    t_float _neg;
    t_int _inv;
    t_float _stages;
    t_float _f;
    
    t_inlet *_inStages; /* Inlet for controlling stages. */
    t_outlet *_outSignal; /* Outputs the signal after applying tube distortion. */
};

typedef struct _tubedist_tilde t_tubedist_tilde;

void* tubedist_tilde_new (t_floatarg pos, t_floatarg neg, t_floatarg inv)
{
    t_tubedist_tilde *obj = (t_tubedist_tilde *)pd_new(tubedist_tilde_class);
    obj->_pos = pos;
    obj->_neg = neg;
    obj->_inv = (t_int)inv;
    obj->_stages = 1;
    obj->_inStages = floatinlet_new(&obj->_obj, &obj->_stages);
    obj->_outSignal = outlet_new(&obj->_obj, &s_signal);
    
#if DEBUG
    post("DEBUG: tubedist~ args: %f, %f, %f", obj->_pos, obj->_neg, obj->_inv);
#endif
    
    return (void *)obj;
}

void tubedist_tilde_free (t_tubedist_tilde* obj)
{
    inlet_free(obj->_inStages);
    outlet_free(obj->_outSignal);
}

t_int* tubedist_perform (t_int* args)
{
    t_tubedist_tilde *obj = (t_tubedist_tilde *)args[1];
    t_sample *in = (t_sample *)args[2];
    t_sample *out = (t_sample *)args[3];
    int numSamples = (int)args[4];
    
    t_int stages = (t_int)obj->_stages;
    
    while (numSamples--) {
        t_float pos = obj->_pos;
        t_float neg = obj->_neg;
        t_float outSample = *in++;
        for (int i = 0; i < stages; ++i) {
            if (outSample >= 0.f)
            {
                outSample = (1.f / atan(pos)) * atan(pos * outSample);
            }
            else
            {
                outSample = (1.f / atan(neg)) * atan(neg * outSample);
            }
            
            if ((obj->_inv == 1) && (i % 2 == 0))
            {
                outSample *= -1.;
            }
        }
        
        *out++ = outSample;
    }
    
    /* Return requirement from documentation specifies that the function must return a pointer
     to the memory directly behind the arguments list (in this case, the number of pointer
     arguments given (4) plus 1. */
    return (args + 5);
}

void tubedist_dsp (t_tubedist_tilde* obj, t_signal** sp)
{
    /* Signal pointer (sp) goes clockwise from the left inlet around to the left outlet. The first (0) is the signal inlet, 
     and the next (1) is the signal outlet. */
    dsp_add(tubedist_perform, 4, obj, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void tubedist_tilde_setup (void)
{
    tubedist_tilde_class = class_new(gensym("tubedist~"),
                                     (t_newmethod)tubedist_tilde_new,
                                     (t_method)tubedist_tilde_free,
                                     sizeof(t_tubedist_tilde), CLASS_DEFAULT,
                                     A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0); /* Args = positive, negative, invert */
    
    class_addmethod(tubedist_tilde_class, (t_method)tubedist_dsp, gensym("dsp"), 0);
    /* Float messages to the left inlet are converted into a signal. */
    CLASS_MAINSIGNALIN(tubedist_tilde_class, t_tubedist_tilde, _f);
}
