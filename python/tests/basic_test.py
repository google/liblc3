from __future__ import annotations

import array
import math
import struct

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


def _generate_sine(
    num_samples: int, sample_rate_hz: int, freq_hz: float = 440.0, amp: float = 0.5
) -> list[float]:
    """Generates a sinusoidal test signal with floating-point amplitudes."""
    return [
        amp * math.sin(2.0 * math.pi * freq_hz * i / sample_rate_hz)
        for i in range(num_samples)
    ]


def _floats_to_pcm(floats: list[float], bit_depth: int | None) -> bytes | list[float]:
    """Converts a float sample sequence to the appropriate PCM representation."""
    if bit_depth is None:
        return list(floats)
    if bit_depth == 16:
        raw = bytearray()
        for x in floats:
            val = max(-32768, min(32767, int(x * 32767)))
            raw.extend(struct.pack("<h", val))
        return bytes(raw)
    if bit_depth == 24:
        raw = bytearray()
        for x in floats:
            val = max(-8388608, min(8388607, int(x * 8388607)))
            raw.extend(val.to_bytes(3, byteorder="little", signed=True))
        return bytes(raw)
    raise ValueError(f"Unsupported bit_depth: {bit_depth}")


def _pcm_to_floats(
    pcm: bytes | array.array[float] | list[float], bit_depth: int | None
) -> list[float]:
    """Converts decoded PCM output back to normalized float samples."""
    if bit_depth is None:
        return list(pcm)
    if bit_depth == 16:
        ints = struct.unpack(f"<{len(pcm) // 2}h", pcm)
        return [x / 32768.0 for x in ints]
    if bit_depth == 24:
        res = []
        for i in range(0, len(pcm), 3):
            val = int.from_bytes(pcm[i : i + 3], byteorder="little", signed=True)
            res.append(val / 8388608.0)
        return res
    raise ValueError(f"Unsupported bit_depth: {bit_depth}")


def _deinterleave(interleaved: list[float], num_channels: int) -> list[list[float]]:
    """Separates an interleaved multi-channel sample stream into per-channel lists."""
    return [interleaved[ch::num_channels] for ch in range(num_channels)]


def _pearson_correlation(x: list[float], y: list[float]) -> float:
    """Computes the Pearson correlation coefficient between two signals."""
    n = min(len(x), len(y))
    if n == 0:
        return 0.0
    x_s = x[:n]
    y_s = y[:n]
    mx = sum(x_s) / n
    my = sum(y_s) / n
    num = sum((xi - mx) * (yi - my) for xi, yi in zip(x_s, y_s))
    den_x = sum((xi - mx) ** 2 for xi in x_s)
    den_y = sum((yi - my) ** 2 for yi in y_s)
    den = math.sqrt(den_x * den_y)
    return num / den if den > 0 else 0.0


def _calculate_rms(samples: list[float]) -> float:
    """Calculates the root mean square (RMS) amplitude of samples."""
    if not samples:
        return 0.0
    return math.sqrt(sum(x * x for x in samples) / len(samples))


@pytest.mark.parametrize("bit_depth", [None, 16, 24])
def test_single_frame_roundtrip(bit_depth: int | None) -> None:
    """Validates single-frame encoding and decoding roundtrip."""
    encoder = lc3.Encoder(frame_duration_us=10000, sample_rate_hz=48000)
    decoder = lc3.Decoder(frame_duration_us=10000, sample_rate_hz=48000)
    samples_per_frame = encoder.get_frame_samples()
    num_bytes = encoder.get_frame_bytes(64000)

    sine_frame = _generate_sine(samples_per_frame, 48000, freq_hz=440.0)
    pcm_in = _floats_to_pcm(sine_frame, bit_depth)

    encoded = encoder.encode(pcm_in, num_bytes=num_bytes, bit_depth=bit_depth)
    assert isinstance(encoded, bytes)
    assert len(encoded) == num_bytes

    decoded = decoder.decode(encoded, bit_depth=bit_depth)
    if bit_depth is None:
        assert isinstance(decoded, array.array)
        assert len(decoded) == samples_per_frame
    else:
        assert isinstance(decoded, bytes)
        bytes_per_sample = 2 if bit_depth == 16 else 3
        assert len(decoded) == samples_per_frame * bytes_per_sample

    decoded_floats = _pcm_to_floats(decoded, bit_depth)
    assert any(abs(x) > 0.0 for x in decoded_floats)
    assert not any(math.isnan(x) or math.isinf(x) for x in decoded_floats)


@pytest.mark.parametrize("frame_duration_us", [10000, 7500])
@pytest.mark.parametrize("sample_rate_hz", [48000, 16000])
@pytest.mark.parametrize("num_channels", [1, 2])
@pytest.mark.parametrize("bit_depth", [None, 16, 24])
def test_stream_roundtrip(
    frame_duration_us: int,
    sample_rate_hz: int,
    num_channels: int,
    bit_depth: int | None,
) -> None:
    """Validates multi-frame audio streaming roundtrip with delay compensation."""
    encoder = lc3.Encoder(
        frame_duration_us=frame_duration_us,
        sample_rate_hz=sample_rate_hz,
        num_channels=num_channels,
    )
    decoder = lc3.Decoder(
        frame_duration_us=frame_duration_us,
        sample_rate_hz=sample_rate_hz,
        num_channels=num_channels,
    )

    spf = encoder.get_frame_samples()
    delay = encoder.get_delay_samples()
    bitrate = 64000 * num_channels
    num_bytes = encoder.get_frame_bytes(bitrate)
    num_frames = 10

    # Generate test tones: 440 Hz for ch0, 880 Hz for ch1 (if stereo)
    channel_inputs = [
        _generate_sine(num_frames * spf, sample_rate_hz, freq_hz=440.0 * (c + 1))
        for c in range(num_channels)
    ]

    decoded_stream: list[float] = []
    for f in range(num_frames):
        frame_floats: list[float] = []
        for s in range(spf):
            for c in range(num_channels):
                frame_floats.append(channel_inputs[c][f * spf + s])

        pcm_in = _floats_to_pcm(frame_floats, bit_depth)
        encoded = encoder.encode(pcm_in, num_bytes=num_bytes, bit_depth=bit_depth)
        decoded = decoder.decode(encoded, bit_depth=bit_depth)
        decoded_stream.extend(_pcm_to_floats(decoded, bit_depth))

    decoded_channels = _deinterleave(decoded_stream, num_channels)
    for c in range(num_channels):
        in_ch = channel_inputs[c]
        out_ch = decoded_channels[c]

        # Delay compensation: align input with output
        comp_in = in_ch[: len(out_ch) - delay]
        comp_out = out_ch[delay : delay + len(comp_in)]

        # Discard the first frame to allow encoder/decoder filter warm-up
        eval_in = comp_in[spf:]
        eval_out = comp_out[spf:]

        corr = _pearson_correlation(eval_in, eval_out)
        rms = _calculate_rms(eval_out)

        assert corr > 0.95, f"Correlation {corr:.4f} below threshold for channel {c}"
        assert 0.20 <= rms <= 0.60, f"RMS {rms:.4f} outside expected range"
        assert not any(math.isnan(x) or math.isinf(x) for x in eval_out)


def test_stereo_channel_separation() -> None:
    """Verifies that stereo encoding and decoding maintains channel separation."""
    sample_rate_hz = 48000
    frame_duration_us = 10000
    encoder = lc3.Encoder(
        frame_duration_us=frame_duration_us,
        sample_rate_hz=sample_rate_hz,
        num_channels=2,
    )
    decoder = lc3.Decoder(
        frame_duration_us=frame_duration_us,
        sample_rate_hz=sample_rate_hz,
        num_channels=2,
    )

    spf = encoder.get_frame_samples()
    delay = encoder.get_delay_samples()
    num_bytes = encoder.get_frame_bytes(128000)
    num_frames = 10

    # Ch0: 440 Hz, Ch1: 880 Hz
    ch0 = _generate_sine(num_frames * spf, sample_rate_hz, freq_hz=440.0)
    ch1 = _generate_sine(num_frames * spf, sample_rate_hz, freq_hz=880.0)

    decoded_stream: list[float] = []
    for f in range(num_frames):
        frame_floats: list[float] = []
        for s in range(spf):
            frame_floats.append(ch0[f * spf + s])
            frame_floats.append(ch1[f * spf + s])

        pcm_in = _floats_to_pcm(frame_floats, 16)
        encoded = encoder.encode(pcm_in, num_bytes=num_bytes, bit_depth=16)
        decoded = decoder.decode(encoded, bit_depth=16)
        decoded_stream.extend(_pcm_to_floats(decoded, 16))

    decoded_channels = _deinterleave(decoded_stream, 2)
    comp_ch0_in = ch0[spf : len(decoded_channels[0]) - delay]
    comp_ch0_out = decoded_channels[0][spf + delay : spf + delay + len(comp_ch0_in)]
    comp_ch1_in = ch1[spf : len(decoded_channels[1]) - delay]
    comp_ch1_out = decoded_channels[1][spf + delay : spf + delay + len(comp_ch1_in)]

    corr_ch0_match = _pearson_correlation(comp_ch0_in, comp_ch0_out)
    corr_ch0_cross = _pearson_correlation(comp_ch0_in, comp_ch1_out)
    corr_ch1_match = _pearson_correlation(comp_ch1_in, comp_ch1_out)
    corr_ch1_cross = _pearson_correlation(comp_ch1_in, comp_ch0_out)

    assert corr_ch0_match > 0.95
    assert abs(corr_ch0_cross) < 0.15
    assert corr_ch1_match > 0.95
    assert abs(corr_ch1_cross) < 0.15


@pytest.mark.parametrize("bit_depth", [None, 16, 24])
def test_silence_roundtrip(bit_depth: int | None) -> None:
    """Verifies that encoding silence produces silent output without noise."""
    encoder = lc3.Encoder(frame_duration_us=10000, sample_rate_hz=48000)
    decoder = lc3.Decoder(frame_duration_us=10000, sample_rate_hz=48000)
    spf = encoder.get_frame_samples()
    num_bytes = encoder.get_frame_bytes(64000)

    silent_frame = [0.0] * spf
    pcm_in = _floats_to_pcm(silent_frame, bit_depth)

    for _ in range(5):
        encoded = encoder.encode(pcm_in, num_bytes=num_bytes, bit_depth=bit_depth)
        decoded = decoder.decode(encoded, bit_depth=bit_depth)
        decoded_floats = _pcm_to_floats(decoded, bit_depth)
        rms = _calculate_rms(decoded_floats)
        assert rms < 1e-4, f"Expected silence, but got RMS {rms:.6f}"


@pytest.mark.parametrize("bit_depth", [None, 16, 24])
def test_packet_loss_concealment(bit_depth: int | None) -> None:
    """Verifies PLC generates smooth concealed frames when packet loss occurs."""
    encoder = lc3.Encoder(frame_duration_us=10000, sample_rate_hz=48000)
    decoder = lc3.Decoder(frame_duration_us=10000, sample_rate_hz=48000)
    spf = encoder.get_frame_samples()
    num_bytes = encoder.get_frame_bytes(64000)

    audio = _generate_sine(3 * spf, 48000, freq_hz=440.0)

    # Feed 3 good frames
    for f in range(3):
        pcm_in = _floats_to_pcm(audio[f * spf : (f + 1) * spf], bit_depth)
        encoded = encoder.encode(pcm_in, num_bytes=num_bytes, bit_depth=bit_depth)
        decoder.decode(encoded, bit_depth=bit_depth)

    # Feed 2 lost frames (PLC)
    plc1 = decoder.decode(None, bit_depth=bit_depth)
    plc2 = decoder.decode(None, bit_depth=bit_depth)

    plc1_floats = _pcm_to_floats(plc1, bit_depth)
    plc2_floats = _pcm_to_floats(plc2, bit_depth)

    assert len(plc1_floats) == spf
    assert len(plc2_floats) == spf

    rms1 = _calculate_rms(plc1_floats)
    rms2 = _calculate_rms(plc2_floats)

    assert rms1 > 0.05, f"PLC frame 1 should have energy, got {rms1:.4f}"
    assert rms2 <= rms1 + 1e-3, (
        f"PLC frame 2 should decay or stay bounded, got {rms2:.4f} vs {rms1:.4f}"
    )
