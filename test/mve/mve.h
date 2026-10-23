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

#if (__ARM_FEATURE_MVE & 1)

#include <arm_mve.h>

#else

#include <stdint.h>


/* ----------------------------------------------------------------------------
 *  Integer
 * -------------------------------------------------------------------------- */

typedef struct { int16_t e[8]; } int16x8_t;


/**
 * Load
 */

__attribute__((unused))
static int16x8_t vldrhq_s16(const int16_t *p)
{
    return (int16x8_t){ { p[0], p[1], p[2], p[3],
                          p[4], p[5], p[6], p[7] } };
}


/**
 * Multiply-accumulate across vector, into a 64-bit accumulator
 */

__attribute__((unused))
static int64_t vmlaldavaq_s16(int64_t a, int16x8_t b, int16x8_t c)
{
    for (int i = 0; i < 8; i++)
        a += (int32_t)b.e[i] * c.e[i];

    return a;
}


#endif /* __ARM_FEATURE_MVE */
