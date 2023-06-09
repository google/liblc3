#include "lc3.h"
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <vector>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  FuzzedDataProvider fdp(data, size);
  auto new_samplerate = [&] {
    return fdp.PickValueInArray({8000, 16000, 24000, 32000, 48000});
  };


  auto new_pcm_format = [&] {
    return fdp.PickValueInArray({LC3_PCM_FORMAT_S16, LC3_PCM_FORMAT_S24,
                                 LC3_PCM_FORMAT_S24_3LE, LC3_PCM_FORMAT_FLOAT});
  };

  const int dt_us = fdp.PickValueInArray({7500, 10000});
  const int sr_hz = new_samplerate();

  assert(LC3_CHECK_SR_HZ(sr_hz));

  int sr_pcm_hz = new_samplerate();
  assert(LC3_CHECK_SR_HZ(sr_pcm_hz));

  if (sr_pcm_hz > sr_hz) {
    sr_pcm_hz = 0;
  }
  const lc3_pcm_format format = new_pcm_format();

  int encoder_size = lc3_encoder_size(dt_us, sr_hz);
  void *mem = malloc(lc3_encoder_size(dt_us, sr_hz));
  lc3_encoder_t enc = lc3_setup_encoder(dt_us, sr_hz, sr_pcm_hz, mem);

  if (enc == NULL) {
    free(mem);
    return -1;
  }

  alignas(alignof(uint32_t)) std::array<uint8_t, LC3_MAX_FRAME_BYTES> out = {0};
  alignas(alignof(uint32_t)) std::array<uint8_t, LC3_MAX_FRAME_BYTES> in = {0};
  std::generate(in.begin(), in.end(),
                [&]() { return fdp.ConsumeIntegral<uint8_t>(); });

  int frame_bytes = lc3_frame_bytes(dt_us, sr_hz);
  assert(frame_bytes < LC3_MAX_FRAME_BYTES);

  if (lc3_encode(enc, new_pcm_format(), in.data(), 0, frame_bytes, out.data()) == -1) {
    return -1;
  };

  int decoder_size = lc3_decoder_size(dt_us, sr_hz);
  void *decoder_mem = malloc(lc3_decoder_size(dt_us, sr_hz));
  lc3_decoder_t dec = lc3_setup_decoder(dt_us, sr_hz, sr_pcm_hz, decoder_mem);
  if (enc == NULL) {
    free(decoder_mem);
    return -1;
  }

  free(decoder_mem);
  free(mem);
  return 0;
}