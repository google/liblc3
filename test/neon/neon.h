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

#if __ARM_NEON

#include <arm_neon.h>

#else
#define __ARM_NEON 1

#include <stdint.h>

typedef struct { int16_t e[4]; } int16x4_t;

typedef struct { int16_t e[8]; } int16x8_t;
typedef struct { int32_t e[4]; } int32x4_t;
typedef struct { int64_t e[2]; } int64x2_t;


/* ----------------------------------------------------------------------------
 *  Load / Store
 * -------------------------------------------------------------------------- */

__attribute__((unused))
static int16x4_t vld1_s16(const int16_t *p)
{
    int16x4_t r;

    for (int i = 0; i < 4; i++)
        r.e[i] = *(p++);

    return r;
}

__attribute__((unused))
static int64x2_t vmovq_n_s64(int64_t v)
{
    int64x2_t r;

    r.e[0] = v;
    r.e[1] = v;

    return r;
}


/* ----------------------------------------------------------------------------
 *  Move
 * -------------------------------------------------------------------------- */

__attribute__((unused))
static int32x4_t vmovq_n_s32(uint32_t v)
{
    int32x4_t r;

    for (int i = 0; i < 4; i++)
        r.e[i] = v;

    return r;
}

__attribute__((unused))
static int16x4_t vext_s16(int16x4_t a, int16x4_t b, const int n)
{
    int16x4_t r;
    int i = 0;

    for (; i < n; i++) r.e[3-i] = b.e[(n-1)-i];
    for (; i < 4; i++) r.e[3-i] = a.e[3-(i-n)];

    return r;
}

/* ----------------------------------------------------------------------------
 *  Arithmetic
 * -------------------------------------------------------------------------- */

__attribute__((unused))
static int32x4_t vmull_s16(int16x4_t a, int16x4_t b)
{
    int32x4_t r;

    for (int i = 0; i < 4; i++)
        r.e[i] = (int32_t)a.e[i] * b.e[i];

    return r;
}

__attribute__((unused))
static int32x4_t vmlal_s16(int32x4_t r, int16x4_t a, int16x4_t b)
{
    for (int i = 0; i < 4; i++)
        r.e[i] += (int32_t)a.e[i] * b.e[i];

    return r;
}

__attribute__((unused))
static int64x2_t vpadalq_s32(int64x2_t a, int32x4_t b)
{
    int64x2_t r;

    r.e[0] = a.e[0] + ((int64_t)b.e[0] + b.e[1]);
    r.e[1] = a.e[1] + ((int64_t)b.e[2] + b.e[3]);

    return r;
}


/* ----------------------------------------------------------------------------
 *  Reduce
 * -------------------------------------------------------------------------- */

__attribute__((unused))
static int32_t vaddvq_s32(int32x4_t v)
{
    return v.e[0] + v.e[1] + v.e[2] + v.e[3];
}

__attribute__((unused))
static int64_t vaddvq_s64(int64x2_t v)
{
    return v.e[0] + v.e[1];
}

#endif /* __ARM_NEON */
