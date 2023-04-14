/******************************************************************************
 *
 *  Copyright 2022 Google LLC
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

#include "mdct.h"
#include "tables.h"

#include "mdct_neon.h"


/* ----------------------------------------------------------------------------
 *  FFT processing
 * -------------------------------------------------------------------------- */

/**
 * FFT 5 Points
 * x, y            Input and output coefficients, of size 5xn
 * n               Number of interleaved transform to perform (n % 2 = 0)
 */
#ifndef fft_5
LC3_HOT static inline void fft_5(
    const struct lc3_complex *x, struct lc3_complex *y, int n)
{
    /* cos( -2Pi n/5 ), sin( -2Pi n/5 ) for n = {1, 2} */

    static const lc3_intfloat_t cos1 = LC3_INTFLOAT_C( 0.3090169944, 31);
    static const lc3_intfloat_t cos2 = LC3_INTFLOAT_C(-0.8090169944, 31);

    static const lc3_intfloat_t sin1 = LC3_INTFLOAT_C(-0.9510565163, 31);
    static const lc3_intfloat_t sin2 = LC3_INTFLOAT_C(-0.5877852523, 31);

    for (int i = 0; i < n; i++, x++, y+= 5) {

        struct lc3_complex s14 =
            { x[1*n].re + x[4*n].re, x[1*n].im + x[4*n].im };
        struct lc3_complex d14 =
            { x[1*n].re - x[4*n].re, x[1*n].im - x[4*n].im };

        struct lc3_complex s23 =
            { x[2*n].re + x[3*n].re, x[2*n].im + x[3*n].im };
        struct lc3_complex d23 =
            { x[2*n].re - x[3*n].re, x[2*n].im - x[3*n].im };

        y[0].re = x[0].re + s14.re + s23.re;

        y[0].im = x[0].im + s14.im + s23.im;

        y[1].re = x[0].re + lc3_shr(
          lc3_mul(s14.re, cos1) - lc3_mul(d14.im, sin1) +
          lc3_mul(s23.re, cos2) - lc3_mul(d23.im, sin2), 31);

        y[1].im = x[0].im + lc3_shr(
          lc3_mul(s14.im, cos1) + lc3_mul(d14.re, sin1) +
          lc3_mul(s23.im, cos2) + lc3_mul(d23.re, sin2), 31);

        y[2].re = x[0].re + lc3_shr(
          lc3_mul(s14.re, cos2) - lc3_mul(d14.im, sin2) +
          lc3_mul(s23.re, cos1) + lc3_mul(d23.im, sin1), 31);

        y[2].im = x[0].im + lc3_shr(
          lc3_mul(s14.im, cos2) + lc3_mul(d14.re, sin2) +
          lc3_mul(s23.im, cos1) - lc3_mul(d23.re, sin1), 31);

        y[3].re = x[0].re + lc3_shr(
          lc3_mul(s14.re, cos2) + lc3_mul(d14.im, sin2) +
          lc3_mul(s23.re, cos1) - lc3_mul(d23.im, sin1), 31);

        y[3].im = x[0].im + lc3_shr(
          lc3_mul(s14.im, cos2) - lc3_mul(d14.re, sin2) +
          lc3_mul(s23.im, cos1) + lc3_mul(d23.re, sin1), 31);

        y[4].re = x[0].re + lc3_shr(
          lc3_mul(s14.re, cos1) + lc3_mul(d14.im, sin1) +
          lc3_mul(s23.re, cos2) + lc3_mul(d23.im, sin2), 31);

        y[4].im = x[0].im + lc3_shr(
          lc3_mul(s14.im, cos1) - lc3_mul(d14.re, sin1) +
          lc3_mul(s23.im, cos2) - lc3_mul(d23.re, sin2), 31);
    }
}
#endif /* fft_5 */

/**
 * FFT Butterfly 3 Points
 * x, y            Input and output coefficients
 * twiddles        Twiddles factors, determine size of transform
 * n               Number of interleaved transforms
 */
#ifndef fft_bf3
LC3_HOT static inline void fft_bf3(
    const struct lc3_fft_bf3_twiddles *twiddles,
    const struct lc3_complex *x, struct lc3_complex *y, int n)
{
    int n3 = twiddles->n3;
    const struct lc3_complex (*w0)[2] = twiddles->t;
    const struct lc3_complex (*w1)[2] = w0 + n3, (*w2)[2] = w1 + n3;

    const struct lc3_complex *x0 = x, *x1 = x0 + n*n3, *x2 = x1 + n*n3;
    struct lc3_complex *y0 = y, *y1 = y0 + n3, *y2 = y1 + n3;

    for (int i = 0; i < n; i++, y0 += 3*n3, y1 += 3*n3, y2 += 3*n3)
        for (int j = 0; j < n3; j++, x0++, x1++, x2++) {

            y0[j].re = x0->re + lc3_shr(
              lc3_mul(x1->re, w0[j][0].re) - lc3_mul(x1->im, w0[j][0].im) +
              lc3_mul(x2->re, w0[j][1].re) - lc3_mul(x2->im, w0[j][1].im), 31);

            y0[j].im = x0->im + lc3_shr(
              lc3_mul(x1->im, w0[j][0].re) + lc3_mul(x1->re, w0[j][0].im) +
              lc3_mul(x2->im, w0[j][1].re) + lc3_mul(x2->re, w0[j][1].im), 31);

            y1[j].re = x0->re + lc3_shr(
              lc3_mul(x1->re, w1[j][0].re) - lc3_mul(x1->im, w1[j][0].im) +
              lc3_mul(x2->re, w1[j][1].re) - lc3_mul(x2->im, w1[j][1].im), 31);

            y1[j].im = x0->im + lc3_shr(
              lc3_mul(x1->im, w1[j][0].re) + lc3_mul(x1->re, w1[j][0].im) +
              lc3_mul(x2->im, w1[j][1].re) + lc3_mul(x2->re, w1[j][1].im), 31);

            y2[j].re = x0->re + lc3_shr(
              lc3_mul(x1->re, w2[j][0].re) - lc3_mul(x1->im, w2[j][0].im) +
              lc3_mul(x2->re, w2[j][1].re) - lc3_mul(x2->im, w2[j][1].im), 31);

            y2[j].im = x0->im + lc3_shr(
              lc3_mul(x1->im, w2[j][0].re) + lc3_mul(x1->re, w2[j][0].im) +
              lc3_mul(x2->im, w2[j][1].re) + lc3_mul(x2->re, w2[j][1].im), 31);
        }
}
#endif /* fft_bf3 */

/**
 * FFT Butterfly 2 Points
 * twiddles        Twiddles factors, determine size of transform
 * x, y            Input and output coefficients
 * n               Number of interleaved transforms
 */
#ifndef fft_bf2
LC3_HOT static inline void fft_bf2(
    const struct lc3_fft_bf2_twiddles *twiddles,
    const struct lc3_complex *x, struct lc3_complex *y, int n)
{
    int n2 = twiddles->n2;
    const struct lc3_complex *w = twiddles->t;

    const struct lc3_complex *x0 = x, *x1 = x0 + n*n2;
    struct lc3_complex *y0 = y, *y1 = y0 + n2;

    for (int i = 0; i < n; i++, y0 += 2*n2, y1 += 2*n2) {

        for (int j = 0; j < n2; j++, x0++, x1++) {

            y0[j].re = x0->re + lc3_shr(
              lc3_mul(x1->re, w[j].re) - lc3_mul(x1->im, w[j].im), 31);
            y0[j].im = x0->im + lc3_shr(
              lc3_mul(x1->im, w[j].re) + lc3_mul(x1->re, w[j].im), 31);

            y1[j].re = x0->re - lc3_shr(
              lc3_mul(x1->re, w[j].re) - lc3_mul(x1->im, w[j].im), 31);
            y1[j].im = x0->im - lc3_shr(
              lc3_mul(x1->im, w[j].re) + lc3_mul(x1->re, w[j].im), 31);
        }
    }
}
#endif /* fft_bf2 */

/**
 * Perform FFT
 * x, y0, y1       Input, and 2 scratch buffers of size `n`
 * n               Number of points 30, 40, 60, 80, 90, 120, 160, 180, 240
 * return          The buffer `y0` or `y1` that hold the result
 *
 * Input `x` can be the same as the `y0` second scratch buffer
 */
static struct lc3_complex *fft(const struct lc3_complex *x, int n,
    struct lc3_complex *y0, struct lc3_complex *y1)
{
    struct lc3_complex *y[2] = { y1, y0 };
    int i2, i3, is = 0;

    /* The number of points `n` can be decomposed as :
     *
     *   n = 5^1 * 3^n3 * 2^n2
     *
     *   for n = 40, 80, 160        n3 = 0, n2 = [3..5]
     *       n = 30, 60, 120, 240   n3 = 1, n2 = [1..4]
     *       n = 90, 180            n3 = 2, n2 = [1..2]
     *
     * Note that the expression `n & (n-1) == 0` is equivalent
     * to the check that `n` is a power of 2. */

    fft_5(x, y[is], n /= 5);

    for (i3 = 0; n & (n-1); i3++, is ^= 1)
        fft_bf3(lc3_fft_twiddles_bf3[i3], y[is], y[is ^ 1], n /= 3);

    for (i2 = 0; n > 1; i2++, is ^= 1)
        fft_bf2(lc3_fft_twiddles_bf2[i2][i3], y[is], y[is ^ 1], n >>= 1);

    return y[is];
}


/* ----------------------------------------------------------------------------
 *  MDCT processing
 * -------------------------------------------------------------------------- */

/**
 * Windowing of samples before MDCT
 * dt, sr          Duration and samplerate (size of the transform)
 * x, y            Input current and delayed samples
 * y, d            Output windowed samples, and delayed ones
 */
LC3_HOT static void mdct_window(enum lc3_dt dt, enum lc3_srate sr,
    const lc3_intfloat_t *x, lc3_intfloat_t *d, lc3_intfloat_t *y)
{
    int ns = LC3_NS(dt, sr), nd = LC3_ND(dt, sr);

    const lc3_intfloat_t *w0 = lc3_mdct_win[dt][sr], *w1 = w0 + ns;
    const lc3_intfloat_t *w2 = w1, *w3 = w2 + nd;

    const lc3_intfloat_t *x0 = x + ns-nd, *x1 = x0;
    lc3_intfloat_t *y0 = y + ns/2, *y1 = y0;
    lc3_intfloat_t *d0 = d, *d1 = d + nd;

    while (x1 > x) {
        *(--y0) = lc3_shr( lc3_mul(*(d0  ), *(w0++)) -
                           lc3_mul(*(--x1), *(--w1)), 30 );

        *(y1++) = lc3_shr( lc3_mul(*(d0++) = *(x0++), *(w2++)), 30 );

        *(--y0) = lc3_shr( lc3_mul(*(d0  ), *(w0++)) -
                           lc3_mul(*(--x1), *(--w1)), 30 );

        *(y1++) = lc3_shr( lc3_mul(*(d0++) = *(x0++), *(w2++)), 30 );
    }

    for (x1 += ns; x0 < x1; ) {
        *(--y0) = lc3_shr( lc3_mul(*(d0  ), *(w0++)) -
                           lc3_mul(*(--d1), *(--w1)), 30 );

        *(y1++) = lc3_shr( lc3_mul(*(d0++) = *(x0++), *(w2++)) +
                           lc3_mul(*(d1  ) = *(--x1), *(--w3)), 30 );

        *(--y0) = lc3_shr( lc3_mul(*(d0  ), *(w0++)) -
                           lc3_mul(*(--d1), *(--w1)), 30 );

        *(y1++) = lc3_shr( lc3_mul(*(d0++) = *(x0++), *(w2++)) +
                           lc3_mul(*(d1  ) = *(--x1), *(--w3)), 30 );
    }
}

/**
 * Pre-rotate and scale MDCT of N/2 points, before N/4 points FFT
 * def             Size and twiddles factors
 * x, y            Input and output coefficients
 *
 * `x` and y` can be the same buffer
 */
LC3_HOT static void mdct_pre_fft(const struct lc3_mdct_rot_def *def,
    const lc3_intfloat_t *x, struct lc3_complex *y)
{
    int n4 = def->n4;

    const lc3_intfloat_t *x0 = x, *x1 = x0 + 2*n4;
    const struct lc3_complex *w0 = def->w, *w1 = w0 + n4;
    struct lc3_complex *y0 = y, *y1 = y0 + n4;

    while (x0 < x1) {
        struct lc3_complex u, uw = *(w0++);
        u.re = lc3_shr( - lc3_mul(*(--x1), uw.re) + lc3_mul(*x0, uw.im), 31 );
        u.im = lc3_shr(   lc3_mul(*(x0++), uw.re) + lc3_mul(*x1, uw.im), 31 );

        struct lc3_complex v, vw = *(--w1);
        v.re = lc3_shr( - lc3_mul(*(--x1), vw.im) + lc3_mul(*x0, vw.re), 31 );
        v.im = lc3_shr( - lc3_mul(*(x0++), vw.im) - lc3_mul(*x1, vw.re), 31 );

        *(y0++) = u;
        *(--y1) = v;
    }
}

/**
 * Post-rotate and scale FFT N/4 points, resulting MDCT N points
 * def             Size and twiddles factors
 * x, y            Input and output coefficients
 *
 * `x` and y` can be the same buffer
 */
LC3_HOT static void mdct_post_fft(const struct lc3_mdct_rot_def *def,
    const struct lc3_complex *x, lc3_intfloat_t *y)
{
    int n4 = def->n4, n8 = n4 >> 1;

    const struct lc3_complex *w0 = def->w + n8, *w1 = w0 - 1;
    const struct lc3_complex *x0 = x + n8, *x1 = x0 - 1;

    lc3_intfloat_t *y0 = y + n4, *y1 = y0;

    for ( ; y1 > y; x0++, x1--, w0++, w1--) {
        lc3_intfloat_t u0, u1, v0, v1;

        u0 = lc3_shr( lc3_mul(x0->im, w0->im) + lc3_mul(x0->re, w0->re), 31 );
        u1 = lc3_shr( lc3_mul(x1->re, w1->im) - lc3_mul(x1->im, w1->re), 31 );

        v0 = lc3_shr( lc3_mul(x0->re, w0->im) - lc3_mul(x0->im, w0->re), 31 );
        v1 = lc3_shr( lc3_mul(x1->im, w1->im) + lc3_mul(x1->re, w1->re), 31 );

        *(y0++) = u0;  *(y0++) = u1;
        *(--y1) = v0;  *(--y1) = v1;
    }
}

/**
 * Pre-rotate and scale IMDCT of N points, before FFT N/4 points FFT
 * def             Size and twiddles factors
 * x, y            Input and output coefficients
 *
 * `x` and `y` can be the same buffer
 * The real and imaginary parts of `y` are swapped,
 * to operate on FFT instead of IFFT
 */
LC3_HOT static void imdct_pre_fft(const struct lc3_mdct_rot_def *def,
    const lc3_intfloat_t *x, struct lc3_complex *y)
{
    int n4 = def->n4;

    const lc3_intfloat_t *x0 = x, *x1 = x0 + 2*n4;

    const struct lc3_complex *w0 = def->w, *w1 = w0 + n4;
    struct lc3_complex *y0 = y, *y1 = y0 + n4;

    while (x0 < x1) {
        lc3_intfloat_t u0 = *(x0++), u1 = *(--x1);
        lc3_intfloat_t v0 = *(x0++), v1 = *(--x1);
        struct lc3_complex uw = *(w0++), vw = *(--w1);

        (y0  )->re = lc3_shr( - lc3_mul(u0, uw.re) - lc3_mul(u1, uw.im), 31 );
        (y0++)->im = lc3_shr( - lc3_mul(u1, uw.re) + lc3_mul(u0, uw.im), 31 );

        (--y1)->re = lc3_shr( - lc3_mul(v1, vw.re) - lc3_mul(v0, vw.im), 31 );
        (  y1)->im = lc3_shr( - lc3_mul(v0, vw.re) + lc3_mul(v1, vw.im), 31 );
    }
}

/**
 * Post-rotate and scale FFT N/4 points, resulting IMDCT N points
 * def             Size and twiddles factors
 * x, y            Input and output coefficients
 *
 * `x` and y` can be the same buffer
 * The real and imaginary parts of `x` are swapped,
 * to operate on FFT instead of IFFT
 */
LC3_HOT static void imdct_post_fft(const struct lc3_mdct_rot_def *def,
    const struct lc3_complex *x, lc3_intfloat_t *y)
{
    int n4 = def->n4;

    const struct lc3_complex *w0 = def->w, *w1 = w0 + n4;
    const struct lc3_complex *x0 = x, *x1 = x0 + n4;

    lc3_intfloat_t *y0 = y, *y1 = y0 + 2*n4;

    while (x0 < x1) {
        struct lc3_complex uz = *(x0++), vz = *(--x1);
        struct lc3_complex uw = *(w0++), vw = *(--w1);

        *(y0++) = lc3_shr( lc3_mul(uz.re, uw.im) - lc3_mul(uz.im, uw.re), 31 );
        *(--y1) = lc3_shr( lc3_mul(uz.re, uw.re) + lc3_mul(uz.im, uw.im), 31 );

        *(--y1) = lc3_shr( lc3_mul(vz.re, vw.im) - lc3_mul(vz.im, vw.re), 31 );
        *(y0++) = lc3_shr( lc3_mul(vz.re, vw.re) + lc3_mul(vz.im, vw.im), 31 );
    }
}

/**
 * Apply windowing of samples
 * dt, sr          Duration and samplerate
 * x, d            Middle half of IMDCT coefficients and delayed samples
 * y, d            Output samples and delayed ones
 */
LC3_HOT static void imdct_window(enum lc3_dt dt, enum lc3_srate sr,
    const lc3_intfloat_t *x, lc3_intfloat_t *d, lc3_intfloat_t *y)
{
    /* The full MDCT coefficients is given by symmetry :
     *   T[   0 ..  n/4-1] = -half[n/4-1 .. 0    ]
     *   T[ n/4 ..  n/2-1] =  half[0     .. n/4-1]
     *   T[ n/2 .. 3n/4-1] =  half[n/4   .. n/2-1]
     *   T[3n/4 ..    n-1] =  half[n/2-1 .. n/4  ]  */

    int n4 = LC3_NS(dt, sr) >> 1, nd = LC3_ND(dt, sr);
    const lc3_intfloat_t *w2 = lc3_mdct_win[dt][sr], *w0 = w2 + 3*n4, *w1 = w0;

    const lc3_intfloat_t *x0 = d + nd-n4, *x1 = x0;
    lc3_intfloat_t *y0 = y + nd-n4, *y1 = y0, *y2 = d + nd, *y3 = d;

    while (y0 > y) {
        *(--y0) = *(--x0) - lc3_shr( lc3_mul(*(x  ), *(w1++)), 30 );
        *(y1++) = *(x1++) + lc3_shr( lc3_mul(*(x++), *(--w0)), 30 );

        *(--y0) = *(--x0) - lc3_shr( lc3_mul(*(x  ), *(w1++)), 30 );
        *(y1++) = *(x1++) + lc3_shr( lc3_mul(*(x++), *(--w0)), 30 );
    }

    while (y1 < y + nd) {
        *(y1++) = *(x1++) + lc3_shr( lc3_mul(*(x++), *(--w0)), 30 );
        *(y1++) = *(x1++) + lc3_shr( lc3_mul(*(x++), *(--w0)), 30 );
    }

    while (y1 < y + 2*n4) {
        *(y1++) = lc3_shr( lc3_mul(*(x  ), *(--w0)), 30 );
        *(--y2) = lc3_shr( lc3_mul(*(x++), *(w2++)), 30 );

        *(y1++) = lc3_shr( lc3_mul(*(x  ), *(--w0)), 30 );
        *(--y2) = lc3_shr( lc3_mul(*(x++), *(w2++)), 30 );
    }

    while (y2 > y3) {
        *(y3++) = lc3_shr( lc3_mul(*(x  ), *(--w0)), 30 );
        *(--y2) = lc3_shr( lc3_mul(*(x++), *(w2++)), 30 );

        *(y3++) = lc3_shr( lc3_mul(*(x  ), *(--w0)), 30 );
        *(--y2) = lc3_shr( lc3_mul(*(x++), *(w2++)), 30 );
    }
}

/**
 * Forward MDCT transformation
 */
void lc3_mdct_forward(
    enum lc3_dt dt, enum lc3_srate sr, enum lc3_srate sr_dst,
    const lc3_intfloat_t *x, lc3_intfloat_t *d, lc3_intfloat_t *y)
{
    const struct lc3_mdct_rot_def *rot = lc3_mdct_rot[dt][sr];
    int nf = LC3_NS(dt, sr_dst);
    int ns = LC3_NS(dt, sr);

    struct lc3_complex buffer[ns/2];
    struct lc3_complex *z = (struct lc3_complex *)y;
    union { lc3_intfloat_t *s; struct lc3_complex *z; } u = { .z = buffer };

    mdct_window(dt, sr, x, d, u.s);

    mdct_pre_fft(rot, u.s, u.z);
    u.z = fft(u.z, ns/2, u.z, z);
    mdct_post_fft(rot, u.z, y);

if (ns != nf) abort(); // TODO: Rescale
}

/**
 * Inverse MDCT transformation
 */
void lc3_mdct_inverse(enum lc3_dt dt, enum lc3_srate sr,
    enum lc3_srate sr_src, const lc3_intfloat_t *x,
    lc3_intfloat_t *d, lc3_intfloat_t *y)
{
    const struct lc3_mdct_rot_def *rot = lc3_mdct_rot[dt][sr];
    int nf = LC3_NS(dt, sr_src);
    int ns = LC3_NS(dt, sr);

    struct lc3_complex buffer[ns/2];
    struct lc3_complex *z = (struct lc3_complex *)y;
    union { lc3_intfloat_t *s; struct lc3_complex *z; } u = { .z = buffer };

    imdct_pre_fft(rot, x, z);
    z = fft(z, ns/2, z, u.z);
    imdct_post_fft(rot, z, u.s);

    imdct_window(dt, sr, u.s, d, y);

if (ns != nf) abort(); // TODO: Rescale
}
