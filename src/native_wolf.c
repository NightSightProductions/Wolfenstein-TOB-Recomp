// Native replacements specific to Wolfenstein: The Old Blood (addresses in tools/native.txt).
#include <math.h>
#include "cpu.h"
#include "rt.h"

// float fmodf(float x, float y): the MSVC CRT's x87 version (fld/fxam/fprem loop), which the translator does
// not support. Args in xmm0/xmm1, result in xmm0 (movss: upper lanes zeroed). libm matches its NaN/Inf rules.
void N_fmodf(CPU *c) {
    float r = fmodf(c->xmm[0].f[0], c->xmm[1].f[0]);
    c->xmm[0].q[0] = 0; c->xmm[0].q[1] = 0;
    c->xmm[0].f[0] = r;
}
