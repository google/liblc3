#
# Copyright 2024 Google LLC
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
from __future__ import annotations

import array
import enum
import typing
from collections.abc import Iterable

import _lc3


class BaseError(Exception):
    """Base error raised by liblc3."""


class InitializationError(RuntimeError, BaseError):
    """Error raised when liblc3 cannot be initialized."""


class InvalidArgumentError(ValueError, BaseError):
    """Error raised when a bad argument is given."""


class _PcmFormat(enum.IntEnum):
    S16 = 0
    S24 = 1
    S24_3LE = 2
    FLOAT = 3


class _Base:

    def __init__(
        self,
        frame_duration_us: int,
        sample_rate_hz: int,
        num_channels: int,
        hrmode: bool = False,
        pcm_sample_rate_hz: int | None = None,
        libpath: str | None = None,
    ) -> None:

        self.hrmode = hrmode
        self.frame_duration_us = frame_duration_us
        self.sample_rate_hz = sample_rate_hz
        self.pcm_sample_rate_hz = pcm_sample_rate_hz or self.sample_rate_hz
        self.num_channels = num_channels

        if self.frame_duration_us not in [2500, 5000, 7500, 10000]:
            raise InvalidArgumentError(
                f"Invalid frame duration: {self.frame_duration_us} us ({self.frame_duration_us / 1000:.1f} ms)"
            )

        allowed_samplerate = (
            [8000, 16000, 24000, 32000, 48000] if not self.hrmode else [48000, 96000]
        )

        if self.sample_rate_hz not in allowed_samplerate:
            raise InvalidArgumentError(f"Invalid sample rate: {sample_rate_hz} Hz")

        if libpath is not None:
            pass

    def get_frame_samples(self) -> int:
        """
        Returns the number of PCM samples in an LC3 frame.
        """
        try:
            return _lc3.lc3_hr_frame_samples(
                self.hrmode, self.frame_duration_us, self.pcm_sample_rate_hz
            )
        except ValueError as e:
            raise InvalidArgumentError("Bad parameters") from e

    def get_frame_bytes(self, bitrate: int) -> int:
        """
        Returns the size of LC3 frame blocks, from bitrate in bit per seconds.
        A target `bitrate` equals 0 or `INT32_MAX` returns respectively
        the minimum and maximum allowed size.
        """
        try:
            return _lc3.lc3_hr_frame_block_bytes(
                self.hrmode,
                self.frame_duration_us,
                self.sample_rate_hz,
                self.num_channels,
                bitrate,
            )
        except ValueError as e:
            raise InvalidArgumentError("Bad parameters") from e

    def resolve_bitrate(self, num_bytes: int) -> int:
        """
        Returns the bitrate in bits per seconds, from the size of LC3 frames.
        """
        try:
            return _lc3.lc3_hr_resolve_bitrate(
                self.hrmode, self.frame_duration_us, self.sample_rate_hz, num_bytes
            )
        except ValueError as e:
            raise InvalidArgumentError("Bad parameters") from e

    def get_delay_samples(self) -> int:
        """
        Returns the algorithmic delay, as a number of samples.
        """
        try:
            return _lc3.lc3_hr_delay_samples(
                self.hrmode, self.frame_duration_us, self.pcm_sample_rate_hz
            )
        except ValueError as e:
            raise InvalidArgumentError("Bad parameters") from e

    @classmethod
    def _resolve_pcm_format(cls, bit_depth: int | None) -> _PcmFormat:
        match bit_depth:
            case 16:
                return _PcmFormat.S16
            case 24:
                return _PcmFormat.S24_3LE
            case None:
                return _PcmFormat.FLOAT
            case _:
                raise InvalidArgumentError("Could not interpret PCM bit_depth")


class Encoder(_Base):
    """
    LC3 Encoder wrapper.

    The `frame_duration_us`, in microsecond, is any of 2500, 5000, 7500, or 10000.
    The `sample_rate_hz`, in Hertz, is any of 8000, 16000, 24000, 32000
    or 48000, unless High-Resolution mode is enabled. In High-Resolution mode,
    the `sample_rate_hz` is 48000 or 96000.

    By default, one channel is processed. When `num_channels` is greater than one,
    the PCM input stream is read interleaved and consecutives LC3 frames are
    output, for each channel.

    Optional arguments:
        hrmode               : Enable High-Resolution mode, default is `False`.
        input_sample_rate_hz : Input PCM samplerate, enable downsampling of input.
        libpath              : LC3 library path and name
    """

    def __init__(
        self,
        frame_duration_us: int,
        sample_rate_hz: int,
        num_channels: int = 1,
        hrmode: bool = False,
        input_sample_rate_hz: int | None = None,
        libpath: str | None = None,
    ) -> None:

        super().__init__(
            frame_duration_us,
            sample_rate_hz,
            num_channels,
            hrmode,
            input_sample_rate_hz,
            libpath,
        )

        try:
            self._impl = _lc3.Encoder(
                self.hrmode,
                self.frame_duration_us,
                self.sample_rate_hz,
                self.pcm_sample_rate_hz,
                self.num_channels,
            )
        except Exception as e:
            raise InitializationError("Failed to initialize encoder") from e

    @typing.overload
    def encode(
        self,
        pcm: bytes | bytearray | memoryview | Iterable[float],
        num_bytes: int,
        bit_depth: None = None,
    ) -> bytes: ...

    @typing.overload
    def encode(
        self, pcm: bytes | bytearray | memoryview, num_bytes: int, bit_depth: int
    ) -> bytes: ...

    def encode(self, pcm, num_bytes: int, bit_depth: int | None = None) -> bytes:
        """
        Encodes LC3 frame(s), for each channel.

        The `pcm` input is given in two ways. When no `bit_depth` is defined,
        it's a vector of floating point values from -1 to 1, coding the sample
        levels. When `bit_depth` is defined, `pcm` is interpreted as a byte-like
        object, each sample coded on `bit_depth` bits (16 or 24).
        The machine endianness, or little endian, is used for 16 or 24 bits
        width, respectively.
        In both cases, the `pcm` vector data is padded with zeros when
        its length is less than the required input samples for the encoder.
        Channels concatenation of encoded LC3 frames, of `nbytes`, is returned.
        """
        pcm_fmt = self._resolve_pcm_format(bit_depth)
        frame_samples = self.get_frame_samples()
        pcm_len = self.num_channels * frame_samples

        if bit_depth is None:
            pcm_buffer: typing.Any = array.array("f", pcm)
            # Invert test to catch NaN
            if not abs(sum(pcm_buffer)) / frame_samples < 2:
                raise InvalidArgumentError("Out of range PCM input")

            padding = max(pcm_len - frame_samples, 0)
            pcm_buffer.extend(array.array("f", [0.0] * padding))
        else:
            sample_size = 2 if pcm_fmt == _PcmFormat.S16 else 3
            expected_bytes = pcm_len * sample_size

            if len(pcm) < expected_bytes:
                pcm_buffer = bytearray(pcm) + bytearray(expected_bytes - len(pcm))
            else:
                pcm_buffer = pcm

        try:
            return self._impl.encode(pcm_buffer, int(pcm_fmt), num_bytes)
        except Exception as e:
            raise InvalidArgumentError("Bad parameters or encoding failed") from e


class Decoder(_Base):
    """
    LC3 Decoder wrapper.

    The `frame_duration_us`, in microsecond, is any of 2500, 5000, 7500, or 10000.
    The `sample_rate_hz`, in Hertz, is any of 8000, 16000, 24000, 32000
    or 48000, unless High-Resolution mode is enabled. In High-Resolution mode,
    the `sample_rate_hz` is 48000 or 96000.

    By default, one channel is processed. When `num_chanels` is greater than one,
    the PCM input stream is read interleaved and consecutives LC3 frames are
    output, for each channel.

    Optional arguments:
        hrmode                : Enable High-Resolution mode, default is `False`.
        output_sample_rate_hz : Output PCM sample_rate_hz, enable upsampling of output.
        libpath               : LC3 library path and name
    """

    def __init__(
        self,
        frame_duration_us: int,
        sample_rate_hz: int,
        num_channels: int = 1,
        hrmode: bool = False,
        output_sample_rate_hz: int | None = None,
        libpath: str | None = None,
    ) -> None:

        super().__init__(
            frame_duration_us,
            sample_rate_hz,
            num_channels,
            hrmode,
            output_sample_rate_hz,
            libpath,
        )

        try:
            self._impl = _lc3.Decoder(
                self.hrmode,
                self.frame_duration_us,
                self.sample_rate_hz,
                self.pcm_sample_rate_hz,
                self.num_channels,
            )
        except Exception as e:
            raise InitializationError("Failed to initialize decoder") from e

    @typing.overload
    def decode(
        self, data: bytes | bytearray | memoryview | None, bit_depth: None = None
    ) -> array.array[float]: ...

    @typing.overload
    def decode(
        self, data: bytes | bytearray | memoryview | None, bit_depth: int
    ) -> bytes: ...

    def decode(
        self, data: bytes | bytearray | memoryview | None, bit_depth: int | None = None
    ) -> bytes | array.array[float]:
        """
        Decodes an LC3 frame.

        The input `data` is the channels concatenation of LC3 frames in a
        byte-like object. Interleaved PCM samples are returned according to
        the `bit_depth` indication.
        Setting `data` to `None` enables PLC (Packet Loss Concealment)
        reconstruction for the block of LC3 frames.
        When no `bit_depth` is defined, it's a vector of floating point values
        from -1 to 1, coding the sample levels. When `bit_depth` is defined,
        it returns a byte array, each sample coded on `bit_depth` bits.
        The machine endianness, or little endian, is used for 16 or 24 bits
        width, respectively.
        """
        pcm_fmt = self._resolve_pcm_format(bit_depth)

        try:
            decoded_bytes = self._impl.decode(data, int(pcm_fmt))
        except Exception as e:
            raise InvalidArgumentError("Bad parameters or decoding failed") from e

        if bit_depth is None:
            return array.array("f", decoded_bytes)
        else:
            return decoded_bytes
