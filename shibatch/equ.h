/*
    Vrok - smokin' audio
    (C) 2012 Madura A. released under GPL 2.0. All following copyrights
    hold. Vrok only redistributes this file, without modifications.

    DeaDBeeF - ultimate music player for GNU/Linux systems with X11
    Copyright (C) 2009-2012 Alexey Yakovenko <waker@users.sourceforge.net>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#ifndef __EQU_H
#define __EQU_H

#include "ooura_fft.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef USE_NE10
#include <NE10.h>
#endif

typedef float REAL;

#ifdef USE_NE10
typedef ne10_fft_cpx_float32_t NE_CPX;
typedef ne10_float32_t NE_REAL;
typedef ne10_fft_r2c_cfg_float32_t NE_CPX_CFG;
#define ne10_rfft ne10_fft_r2c_1d_float32
#define ne10_rifft ne10_fft_c2r_1d_float32
#define ne10_alloc ne10_fft_alloc_r2c_float32
#endif

typedef struct {
    REAL *lires,*lires1,*lires2;
    REAL *irest;
    REAL *fsamples;
#ifdef USE_NE10
    NE_CPX_CFG cfg;
    NE_REAL *fsamples_in;
    NE_CPX *fsamples_out;
#endif
    REAL *ditherbuf;
    int ditherptr;
    volatile int chg_ires,cur_ires;
    int winlen,winlenbit,tabsize,tabsizeH,nbufsamples;
    REAL *finbuf;
    REAL *outbuf;
    int dither;
    int channels;
    int enable;
    int fft_bits;
} SuperEqState;

void *paramlist_alloc (void);
void paramlist_free (void *);
void equ_makeTable(SuperEqState *state, REAL *lbc,void *param,REAL fs);
int equ_modifySamples(SuperEqState *state, char *buf,int nsamples,int nch,int bps);
int equ_modifySamples_float (SuperEqState *state, char *buf,int nsamples,int nch);
int equ_modifySamples_double (SuperEqState *state, char *buf,int nsamples,int nch);

void equ_clearbuf(SuperEqState *state);
void equ_init(SuperEqState *state, int wb, int channels);
void equ_quit(SuperEqState *state);

#ifdef __cplusplus
}
#endif

#endif


template <typename REAL_T>
int equ_modifySamples_real (SuperEqState *state, char *buf,int nsamples,int nch);
