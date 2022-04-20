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
 * LC3 - Mathematics function approximation
 */

#ifndef __LC3_FASTMATH_H
#define __LC3_FASTMATH_H

#include <math.h>


/**
 * Fast 2^n approximation
 * x               Operand, range -8 to 8
 * return          2^x approximation (max relative error ~ 7e-6)
 */
static inline float fast_exp2f(float x)
{
    float y;

    /* --- Polynomial approx in range -0.5 to 0.5 --- */

    static const float c[] = { 1.27191277e-09, 1.47415221e-07,
        1.35510312e-05, 9.38375815e-04, 4.33216946e-02 };

    y = (    c[0]) * x;
    y = (y + c[1]) * x;
    y = (y + c[2]) * x;
    y = (y + c[3]) * x;
    y = (y + c[4]) * x;
    y = (y + 1.f);

    /* --- Raise to the power of 16  --- */

    y = y*y;
    y = y*y;
    y = y*y;
    y = y*y;

    return y;
}

/**
 * Fast log2(x) approximation
 * x               Operand, greater than 0
 * return          log2(x) approximation (max absolute error ~ 1e-4)
 */
static inline float fast_log2f(float x)
{
    float y;
    int e;

    /* --- Polynomial approx in range 0.5 to 1 --- */

    static const float c[] = {
        -1.29479677, 5.11769018, -8.42295281, 8.10557963, -3.50567360 };

    x = frexpf(x, &e);

    y = (    c[0]) * x;
    y = (y + c[1]) * x;
    y = (y + c[2]) * x;
    y = (y + c[3]) * x;
    y = (y + c[4]);

    /* --- Add log2f(2^e) and return --- */

    return e + y;
}

/**
 * Fast log10(x) approximation
 * x               Operand, greater than 0
 * return          log10(x) approximation (max absolute error ~ 1e-4)
 */
static inline float fast_log10f(float x)
{
    return log10f(2) * fast_log2f(x);
}


#endif /* __LC3_FASTMATH_H */
