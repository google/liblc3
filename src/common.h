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

#ifndef __LC3_COMMON_H
#define __LC3_COMMON_H

#include <lc3.h>
#include "fastmath.h"

#include <stdalign.h>
#include <limits.h>

#ifdef __wasm32__
#define memmove __builtin_memmove
#define memset __builtin_memset
#define memcpy __builtin_memcpy
#define NULL ((void*)0)
#else
#include <string.h>
#endif

#ifdef __ARM_ARCH
#include <arm_acle.h>
#endif


/**
 * Acivation flags for LC3-Plus and LC3-Plus HR features
 */

#ifndef LC3_PLUS
#define LC3_PLUS 1
#endif

#ifndef LC3_PLUS_HR
#define LC3_PLUS_HR 1
#endif

#if LC3_PLUS
#define LC3_IF_PLUS(a, b) (a)
#else
#define LC3_IF_PLUS(a, b) (b)
#endif

#if LC3_PLUS_HR
#define LC3_IF_PLUS_HR(a, b) (a)
#else
#define LC3_IF_PLUS_HR(a, b) (b)
#endif


/**
 * Hot Function attribute
 * Selectively disable sanitizer
 */

#ifdef __clang__

#define LC3_HOT \
    __attribute__((no_sanitize("bounds"))) \
    __attribute__((no_sanitize("integer")))

#else /* __clang__ */

#define LC3_HOT

#endif /* __clang__ */


/**
 * Macros
 * MIN/MAX  Minimum and maximum between 2 values
 * CLIP     Clip a value between low and high limits
 * SATXX    Signed saturation on 'xx' bits
 * ABS      Return absolute value
 */

#define LC3_MIN(a, b)  ( (a) < (b) ?  (a) : (b) )
#define LC3_MAX(a, b)  ( (a) > (b) ?  (a) : (b) )

#define LC3_CLIP(v, min, max)  LC3_MIN(LC3_MAX(v, min), max)
#define LC3_SAT16(v)  LC3_CLIP(v, -(1 << 15), (1 << 15) - 1)
#define LC3_SAT24(v)  LC3_CLIP(v, -(1 << 23), (1 << 23) - 1)

#define LC3_ABS(v)  ( (v) < 0 ? -(v) : (v) )


#if defined(__ARM_FEATURE_SAT) && !(__GNUC__ < 10)

#undef  LC3_SAT16
#define LC3_SAT16(v) __ssat(v, 16)

#undef  LC3_SAT24
#define LC3_SAT24(v) __ssat(v, 24)

#endif /* __ARM_FEATURE_SAT */


/**
 * Return `true` when high-resolution mode
 */
static inline bool lc3_hr(enum lc3_srate sr) {
    return LC3_PLUS_HR && (sr >= LC3_SRATE_48K_HR);
}


/**
 * Bandwidth, mapped to Nyquist frequency of samplerates
 */

enum lc3_bandwidth {
    LC3_BANDWIDTH_NB = LC3_SRATE_8K,
    LC3_BANDWIDTH_WB = LC3_SRATE_16K,
    LC3_BANDWIDTH_SSWB = LC3_SRATE_24K,
    LC3_BANDWIDTH_SWB = LC3_SRATE_32K,
    LC3_BANDWIDTH_FB = LC3_SRATE_48K,

    LC3_BANDWIDTH_FB_HR = LC3_SRATE_48K_HR,
    LC3_BANDWIDTH_UB_HR = LC3_SRATE_96K_HR,

    LC3_NUM_BANDWIDTH,
};


/**
 * Complex floating point number
 */

struct lc3_complex
{
    float re, im;
};


#endif /* __LC3_COMMON_H */
