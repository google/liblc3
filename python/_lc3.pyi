import collections.abc

class Decoder:
    def __init__(
        self,
        hrmode: bool,
        dt_us: int,
        sr_hz: int,
        sr_pcm_hz: int,
        nchannels: int,
    ) -> None: ...
    def decode(
        self, data_obj: collections.abc.Buffer | None, pcm_fmt: int
    ) -> bytes: ...
    def get_frame_samples(self) -> int: ...

class Encoder:
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
        pcm_fmt: int,
        num_bytes: int,
    ) -> bytes: ...
    def get_frame_samples(self) -> int: ...

def lc3_hr_delay_samples(
    hrmode: bool,
    dt_us: int,
    sr_hz: int,
) -> int: ...
def lc3_hr_frame_block_bytes(
    hrmode: bool,
    dt_us: int,
    sr_hz: int,
    nchannels: int,
    bitrate: int,
) -> int: ...
def lc3_hr_frame_samples(
    hrmode: bool,
    dt_us: int,
    sr_hz: int,
) -> int: ...
def lc3_hr_resolve_bitrate(
    hrmode: bool,
    dt_us: int,
    sr_hz: int,
    nbytes: int,
) -> int: ...
