#
# Copyright 2026 Google LLC
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
import collections.abc

class EncoderContext:
    def __init__(
        self,
        hrmode: bool,
        dt_us: int,
        sr_hz: int,
        sr_pcm_hz: int,
        nchannels: int,
    ) -> None: ...
    def encode(
        self,
        pcm: collections.abc.Buffer,
        num_bytes: int,
        pcm_format: int,
    ) -> bytes: ...
    def disable_ltpf(self) -> None: ...

class DecoderContext:
    def __init__(
        self,
        hrmode: bool,
        dt_us: int,
        sr_hz: int,
        sr_pcm_hz: int,
        nchannels: int,
    ) -> None: ...
    def decode(
        self,
        data: collections.abc.Buffer | None,
        pcm_format: int,
    ) -> bytes: ...

def hr_frame_samples(
    hrmode: bool,
    dt_us: int,
    sr_hz: int,
) -> int: ...

def hr_frame_block_bytes(
    hrmode: bool,
    dt_us: int,
    sr_hz: int,
    nchannels: int,
    bitrate: int,
) -> int: ...

def hr_resolve_bitrate(
    hrmode: bool,
    dt_us: int,
    sr_hz: int,
    nbytes: int,
) -> int: ...

def hr_delay_samples(
    hrmode: bool,
    dt_us: int,
    sr_hz: int,
) -> int: ...

def hr_encoder_size(
    hrmode: bool,
    dt_us: int,
    sr_hz: int,
) -> int: ...

def hr_decoder_size(
    hrmode: bool,
    dt_us: int,
    sr_hz: int,
) -> int: ...
