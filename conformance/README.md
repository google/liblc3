# Conformance test reports

For more detail on conformance, refer to [_Bluetooth Conformance
Documents and scripts_](https://www.bluetooth.com/specifications/specs/low-complexity-communication-codec-1-0/)

Exposed reports were obtained by exercising the release [1.0.4](https://github.com/google/liblc3/releases/tag/v1.0.4), using the [_Test Software 1.0.6_](https://www.bluetooth.com/specifications/specs/low-complexity-communication-codec-1-0/)

## Speech

* Encoding 10ms

```
[speech_encode_10m]

# test modes
test_sqam           = 1
test_band_limiting  = 1
test_low_pass       = 0
test_rate_switching = 0

#         Mode, Samplingrate, Bitrate
configs = encode,  8000, 24000
          encode, 16000, 32000
          encode, 24000, 48000
          encode, 32000, 64000
```

[Report](https://raw.githack.com/google/liblc3/main/conformance/speech_encode_10m.html)

* Decoding 10ms

```
[speech_decode_10m]

# test modes
test_sqam           = 1
test_band_limiting  = 0
test_low_pass       = 0
test_rate_switching = 0

#         Mode, Samplingrate, Bitrate
configs = decode,  8000, 24000
          decode, 16000, 32000
          decode, 24000, 48000
          decode, 32000, 64000
```

[Report](https://raw.githack.com/google/liblc3/main/conformance/speech_decode_10m.html)

* Encoding + Decoding 10ms

```
[speech_encdec_10m]

# test modes
test_sqam           = 1
test_band_limiting  = 1
test_low_pass       = 0
test_rate_switching = 0

#         Mode, Samplingrate, Bitrate
configs = encdec,  8000, 24000
          encdec, 16000, 32000
          encdec, 24000, 48000
          encdec, 32000, 64000
```

[Report](https://raw.githack.com/google/liblc3/main/conformance/speech_encdec_10m.html)

* Encoding 7.5ms

```
[speech_encode_7m5]

# test modes
test_sqam           = 1
test_band_limiting  = 1
test_low_pass       = 0
test_rate_switching = 0

#         Mode, Samplingrate, Bitrate
configs = encode,  8000, 27734
          encode, 16000, 32000
          encode, 24000, 48000
          encode, 32000, 64000
```

[Report](https://raw.githack.com/google/liblc3/main/conformance/speech_encode_7m5.html)

* Decoding 7.5ms

```
[speech_decode_7m5]

# test modes
test_sqam           = 1
test_band_limiting  = 0
test_low_pass       = 0
test_rate_switching = 0

#         Mode, Samplingrate, Bitrate
configs = decode,  8000, 27734
          decode, 16000, 32000
          decode, 24000, 48000
          decode, 32000, 64000
```

[Report](https://raw.githack.com/google/liblc3/main/conformance/speech_decode_7m5.html)

* Encoding + Decoding 7.5ms

```
[speech_encdec_7m5]

# test modes
test_sqam           = 1
test_band_limiting  = 1
test_low_pass       = 0
test_rate_switching = 0

#         Mode, Samplingrate, Bitrate
configs = encdec,  8000, 27734
          encdec, 16000, 32000
          encdec, 24000, 48000
          encdec, 32000, 64000
```

[Report](https://raw.githack.com/google/liblc3/main/conformance/speech_encdec_7m5.html)

## Music

* Encoding 10ms

```
[music_encode_10m]

# test modes
test_sqam           = 1
test_band_limiting  = 1
test_low_pass       = 1
test_rate_switching = 0

#         Mode, Samplingrate, Bitrate
configs = encode, 48000,  80000
          encode, 48000,  96000
          encode, 48000, 124000
```

[Report](https://raw.githack.com/google/liblc3/main/conformance/music_encode_10m.html)

* Decoding 10ms

```
[music_decode_10m]

# test modes
test_sqam           = 1
test_band_limiting  = 0
test_low_pass       = 1
test_rate_switching = 0

#         Mode, Samplingrate, Bitrate
configs = decode, 48000,  80000
          decode, 48000,  96000
          decode, 48000, 124000
```

[Report](https://raw.githack.com/google/liblc3/main/conformance/music_decode_10m.html)

* Encoding + Decoding 10ms

```
[music_encdec_10m]

# test modes
test_sqam           = 1
test_band_limiting  = 1
test_low_pass       = 1
test_rate_switching = 0

#         Mode, Samplingrate, Bitrate
configs = encdec, 48000,  80000
          encdec, 48000,  96000
          encdec, 48000, 124000
```

[Report](https://raw.githack.com/google/liblc3/main/conformance/music_encdec_10m.html)

* Encoding 7.5ms

```
[music_encode_7m5]

# test modes
test_sqam           = 1
test_band_limiting  = 1
test_low_pass       = 1
test_rate_switching = 0

#         Mode, Samplingrate, Bitrate
configs = encode, 48000,  80000
          encode, 48000,  96000
          encode, 48000, 124800
```

[Report](https://raw.githack.com/google/liblc3/main/conformance/music_encode_7m5.html)


* Decoding 7.5ms

```
[music_decode_7m5]

# test modes
test_sqam           = 1
test_band_limiting  = 0
test_low_pass       = 1
test_rate_switching = 0

#         Mode, Samplingrate, Bitrate
configs = decode, 48000,  80000
          decode, 48000,  96000
          decode, 48000, 124800
```

[Report](https://raw.githack.com/google/liblc3/main/conformance/music_decode_7m5.html)

* Encoding + Decoding 7.5ms

```
[music_encdec_7m5]

# test modes
test_sqam           = 1
test_band_limiting  = 1
test_low_pass       = 1
test_rate_switching = 0

#         Mode, Samplingrate, Bitrate
configs = encdec, 48000,  80000
          encdec, 48000,  96000
          encdec, 48000, 124800
```

[Report](https://raw.githack.com/google/liblc3/main/conformance/music_encdec_7m5.html)
