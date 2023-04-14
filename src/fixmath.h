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

/**
 * LC3 - Mathematics in fixed point
 */

#ifndef __LC3_FIXMATH_H
#define __LC3_FIXMATH_H

#include <stdint.h>

#undef  CONFIG_FIXED_POINT // TODO


/**
 * Type definition, integer or floating point
 * TODO
 */

#ifdef CONFIG_FIXED_POINT

typedef int32_t lc3_intfloat_t;

#define LC3_INTFLOAT_C(x, q) \
  (int32_t)LC3_CLIP( (x) * (INT64_C(1) << (q)), -0x1p31, 0x1p31-1 )

#else /* CONFIG_FIXED_POINT */

typedef float lc3_intfloat_t;

#define LC3_INTFLOAT_C(x, q) (x)

#endif /* CONFIG_FIXED_POINT */


/**
 * Arithmetic operations
 *   mul    Multiplicaton of 2 x 32 bits values, results on 64 bits
 *   shr    Arithmetic rounded right shift
 */

#ifdef CONFIG_FIXED_POINT

static inline int64_t lc3_mul(int32_t a, int32_t b) {
    return (int64_t)a * b;
}

static inline int32_t lc3_shr(int64_t x, int q) {
    return (x + (INT64_C(1) << (q-1))) >> q;
}

static inline int32_t lc3_rsqrt(int32_t x, int q) {
    return (int32_t)(ldexpf(1.f / sqrtf(x), q) + 0.5f);
}

#else /* CONFIG_FIXED_POINT */

#define lc3_mul(a, b)    ( (a) * (b) )
#define lc3_shr(x, q)    ( (x) )
#define lc3_rsqrt(x, q)  ( 1.f / sqrtf(x) )

#endif /* CONFIG_FIXED_POINT */


#endif /* __LC3_FIXMATH_H */
