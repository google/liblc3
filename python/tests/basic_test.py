import array
import lc3
import pytest


@pytest.mark.parametrize("bit_depth,decoded_length", [(16, 960), (24, 1440)])
def test_decode_with_bit_depth(bit_depth, decoded_length) -> None:
    decoder = lc3.Decoder(frame_duration_us=10000, sample_rate_hz=48000)
    decoded_frame = decoder.decode(bytes(120), bit_depth=bit_depth)
    assert isinstance(decoded_frame, bytes)
    assert len(decoded_frame) == decoded_length


def test_decode_without_bit_depth() -> None:
    decoder = lc3.Decoder(frame_duration_us=10000, sample_rate_hz=48000)
    decoded_frame = decoder.decode(bytes(120))
    assert isinstance(decoded_frame, array.array)
    assert len(decoded_frame) == 480
    assert all(isinstance(e, float) for e in decoded_frame)


def test_decode_with_bad_bit_depth() -> None:
    decoder = lc3.Decoder(frame_duration_us=10000, sample_rate_hz=48000)
    with pytest.raises(lc3.InvalidArgumentError):
        decoder.decode(bytes(120), bit_depth=128)


@pytest.mark.parametrize("bit_depth", [16, 24])
def test_encode_with_bit_depth(bit_depth) -> None:
    encoder = lc3.Encoder(frame_duration_us=10000, sample_rate_hz=48000)
    encoded_frame = encoder.encode(bytes(1920), num_bytes=120, bit_depth=bit_depth)
    assert isinstance(encoded_frame, bytes)
    assert len(encoded_frame) == 120


@pytest.mark.parametrize("pcm", [bytes(1920), [0.0] * 1920])
def test_encode_without_bit_depth(pcm) -> None:
    encoder = lc3.Encoder(frame_duration_us=10000, sample_rate_hz=48000)
    encoded_frame = encoder.encode(pcm, num_bytes=120, bit_depth=None)
    assert isinstance(encoded_frame, bytes)
    assert len(encoded_frame) == 120


def test_encode_with_bad_bit_depth() -> None:
    encoder = lc3.Encoder(frame_duration_us=10000, sample_rate_hz=48000)
    with pytest.raises(lc3.InvalidArgumentError):
        encoder.encode(bytes(1920), num_bytes=120, bit_depth=128)
