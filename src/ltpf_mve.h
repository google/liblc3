/******************************************************************************
 *
 *  Copyright 2026 Google LLC
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

/*
 * Arm Helium (M-Profile Vector Extension, MVE) backend for the LTPF
 * pitch-detection hotspot, targeting Armv8.1-M cores (e.g. Cortex-M55).
 *
 * `dot()` / `correlate()` accumulate int16 x int16 products into a 64-bit
 * integer and apply the rounding `(v + 32) >> 6`. MVE `vmlaldavaq_s16`
 * performs an 8-lane int16 x int16 -> int64 multiply-accumulate across the
 * vector; the integer sum and the subsequent rounding are therefore
 * identical to the scalar reference, i.e. the result is strictly bit-exact.
 *
 * This mirrors the NEON (`ltpf_neon.h`) and Arm DSP (`ltpf_arm.h`) backends
 * and hooks the same `dot` / `correlate` override points. Include this file
 * before the NEON/DSP backends in `ltpf.c` so its definitions take
 * precedence on MVE targets.
 */

#if (__ARM_FEATURE_MVE & 1) && !defined(TEST_ARM) && !defined(TEST_NEON) \
        || defined(TEST_MVE)

#ifndef TEST_MVE
#include <arm_mve.h>
#endif /* TEST_MVE */


/**
 * Return dot product of 2 vectors
 * a, b, n         The 2 vectors of size `n` (> 0 and <= 128)
 * return          round( sum( a[i] * b[i] ) / 64 ), as float
 *
 * The size `n` of vectors is a multiple of 16 (so also of 8).
 */
#ifndef dot

LC3_HOT static inline float mve_dot(const int16_t *a, const int16_t *b, int n)
{
    int64_t v = 0;

    for (int i = 0; i < (n >> 3); i++) {
        int16x8_t va = vldrhq_s16(a); a += 8;
        int16x8_t vb = vldrhq_s16(b); b += 8;
        v = vmlaldavaq_s16(v, va, vb);
    }

    int32_t v32 = (v + (1 << 5)) >> 6;
    return (float)v32;
}

#ifndef TEST_MVE
#define dot mve_dot
#endif

#endif /* dot */

/**
 * Return vector of correlations
 * a, b, n         The 2 vectors of size `n` (> 0 and <= 128)
 * y, nc           Output the correlation vector of size `nc`
 *
 * Sliding dot product: for each output the `b` window is shifted by one
 * sample. Each lag reuses the 8-lane MVE `dot`, matching the scalar
 * reference exactly.
 */
#ifndef correlate

LC3_HOT static void mve_correlate(
    const int16_t *a, const int16_t *b, int n, float *y, int nc)
{
    for (const float *ye = y + nc; y < ye; )
        *(y++) = mve_dot(a, b--, n);
}

#ifndef TEST_MVE
#define correlate mve_correlate
#endif

#endif /* correlate */

#endif /* __ARM_FEATURE_MVE & 1 */
