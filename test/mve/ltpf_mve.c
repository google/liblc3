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

#include "mve.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* -------------------------------------------------------------------------- */

#define TEST_MVE
#include <ltpf.c>

void lc3_put_bits_generic(lc3_bits_t *a, unsigned b, int c)
{ (void)a, (void)b, (void)c; }

unsigned lc3_get_bits_generic(struct lc3_bits *a, int b)
{ return (void)a, (void)b, 0; }

/* -------------------------------------------------------------------------- */

static int check_dot()
{
    int16_t x[200];
    for (int i = 0; i < 200; i++)
        x[i] = rand() & 0xffff;

    float y = dot(x, x+3, 128);
    float y_mve = mve_dot(x, x+3, 128);
    if (y != y_mve)
        return -1;

    return 0;
}

static int check_correlate()
{
    int16_t alignas(4) a[500], b[500];
    float y[100], y_mve[100];

    for (int i = 0; i < 500; i++) {
        a[i] = rand() & 0xffff;
        b[i] = rand() & 0xffff;
    }

    correlate(a, b+200, 128, y, 100);
    mve_correlate(a, b+200, 128, y_mve, 100);
    if (memcmp(y, y_mve, 100 * sizeof(*y)) != 0)
        return -1;

    correlate(a, b+199, 128, y, 99);
    mve_correlate(a, b+199, 128, y_mve, 99);
    if (memcmp(y, y_mve, 99 * sizeof(*y)) != 0)
        return -1;

    return 0;
}

int check_ltpf(void)
{
    int ret;

    if ((ret = check_dot()) < 0)
        return ret;

    if ((ret = check_correlate()) < 0)
        return ret;

    return 0;
}
