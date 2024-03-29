/******************************************************************************
 *
 *  Copyright 2024 Google LLC
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

#ifndef __LC3_MATH_H
#define __LC3_MATH_H

#define INFINITY __builtin_inff()

#define floorf  __builtin_floorf
#define truncf  __builtin_truncf

static inline float roundf(float x)
{ return x >= 0 ? truncf(x + 0.5f) : truncf(x - 0.5f); }

#define fabsf   __builtin_fabsf
#define fmaxf   __builtin_fmaxf
#define fminf   __builtin_fminf

#define log10f  __builtin_log10f
#define sqrtf   __builtin_sqrtf

#endif /* __LC3_MATH_H */
