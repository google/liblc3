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

#include "energy.h"
#include "lc3_private.h"
#include "tables.h"


/**
 * Energy estimation per band
 */
bool lc3_energy_compute(
    enum lc3_dt dt, enum lc3_srate sr, const float *x, float *e)
{
    static const int n1_table[LC3_NUM_DT][LC3_NUM_SRATE] = {
#ifdef INCLUDE_2M5
        [LC3_DT_2M5] = { 20, 30, 26, 24, 23 },
#endif
#ifdef INCLUDE_05M
        [LC3_DT_05M] = { 38, 30, 24, 22, 21 },
#endif
        [LC3_DT_7M5] = { 56, 34, 27, 24, 22 },
        [LC3_DT_10M] = { 49, 28, 23, 20, 18 },
    };

    /* First bands are 1 coefficient width */

    int n1 = n1_table[dt][sr];
    float e_sum[2] = { 0, 0 };
    int iband;

    for (iband = 0; iband < n1; iband++) {
        *e = x[iband] * x[iband];
        e_sum[0] += *(e++);
    }

    /* Mean the square of coefficients within each band,
     * note that 7.5ms 8KHz frame has more bands than samples */

    int nb = LC3_MIN((unsigned) lc3_bands_number[dt][sr], LC3_NS(dt, sr));
    int iband_h = nb - 2*(2 - (dt != LC3_DT_7M5));
    const int *lim = lc3_band_lim[dt][sr];

    for (int i = lim[iband]; iband < nb; iband++) {
        int ie = lim[iband+1];
        int n = ie - i;

        float sx2 = x[i] * x[i];
        for (i++; i < ie; i++)
            sx2 += x[i] * x[i];

        *e = sx2 / n;
        e_sum[iband >= iband_h] += *(e++);
    }

    for (; iband < LC3_NUM_BANDS; iband++)
        *(e++) = 0;

    /* Return the near nyquist flag */

    return e_sum[1] > 30 * e_sum[0];
}
