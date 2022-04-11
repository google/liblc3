#!/usr/bin/env python3
#
# Copyright 2022 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import numpy as np


def print_table(t, m=4):

    for (i, v) in enumerate(t):
        print('{:14.8e},'.format(v), end = '\n' if i%m == m-1 else ' ')

    if len(t) % 4:
        print('')


def mdct_fft_twiddles():

    for n in (10, 20, 30, 40, 60, 80, 90, 120, 160, 180, 240):

        print('\n--- fft bf2 twiddles {:3d} ---'.format(n))

        kv = -2 * np.pi * np.arange(n // 2) / n
        for (i, k) in enumerate(kv):
            print('{{ {:14.7e}, {:14.7e} }},'.format(np.cos(k), np.sin(k)),
                  end = '\n' if i%2 == 1 else ' ')

    for n in (15, 45):

        print('\n--- fft bf3 twiddles {:3d} ---'.format(n))

        kv = -2 * np.pi * np.arange(n) / n
        for k in kv:
            print(('{{ {{ {:14.7e}, {:14.7e} }},' +
                     ' {{ {:14.7e}, {:14.7e} }} }},').format(
                np.cos(k), np.sin(k), np.cos(2*k), np.sin(2*k)))


def mdct_rot_twiddles():

    for n in (120, 160, 240, 320, 360, 480, 640, 720, 960):

        print('\n--- mdct rot twiddles {:3d} ---'.format(n))

        kv = 2 * np.pi * (np.arange(n // 4) + 1/8) / n
        for (i, k) in enumerate(kv):
            print('{{ {:14.7e}, {:14.7e} }},'.format(np.cos(k), np.sin(k)),
                  end = '\n' if i%2 == 1 else ' ')


def mdct_scaling():

    print('\n--- mdct scaling ---')
    ns = np.array([ [ 60, 120, 180, 240, 360], [ 80, 160, 240, 320, 480] ])
    print_table(np.sqrt(2 / ns[0]))
    print_table(np.sqrt(2 / ns[1]))


def tns_lag_window():

    print('\n--- tns lag window ---')
    print_table(np.exp(-0.5 * (0.02 * np.pi * np.arange(9)) ** 2))


def tns_quantization_table():

    print('\n--- tns quantization table ---')
    print_table(np.sin((np.arange(8) + 0.5) * (np.pi / 17)))
    print_table(np.sin((np.arange(8)) * (np.pi / 17)))


def quant_iq_table():

    print('\n--- quantization iq table ---')
    print_table(10 ** (np.arange(65) / 28))


def sns_ge_table():

    g_tilt_table = [ 14, 18, 22, 26, 30 ]

    for (sr, g_tilt) in enumerate(g_tilt_table):
        print('\n--- sns ge table, sr:{} ---'.format(sr))
        print_table(10 ** ((np.arange(64) * g_tilt) / 630))


def inv_table():

    print('\n--- inv table ---')
    print_table(np.append(np.zeros(1), 1 / np.arange(1, 28)))


if __name__ == '__main__':

    mdct_fft_twiddles()
    mdct_rot_twiddles()
    mdct_scaling()

    inv_table()
    sns_ge_table()
    tns_lag_window()
    tns_quantization_table()
    quant_iq_table()

    print('')
