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

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <lc3.h>
#include <vector>
#include <stdexcept>
#include <iostream>

namespace py = pybind11;

class Encoder {
    std::vector<lc3_encoder_t> encoders;
    std::vector<std::vector<uint8_t>> mem;
    int nchannels;
    int frame_samples;

public:
    Encoder(bool hrmode, int dt_us, int sr_hz, int sr_pcm_hz, int nchannels)
        : nchannels(nchannels) {
        frame_samples = lc3_hr_frame_samples(hrmode, dt_us, sr_pcm_hz);
        if (frame_samples < 0) {
            throw std::invalid_argument("Invalid parameters for frame samples");
        }
        
        unsigned size = lc3_hr_encoder_size(hrmode, dt_us, sr_pcm_hz);
        if (size == 0) {
            throw std::invalid_argument("Invalid parameters for encoder size");
        }

        for (int i = 0; i < nchannels; ++i) {
            mem.emplace_back(size);
            lc3_encoder_t enc = lc3_hr_setup_encoder(hrmode, dt_us, sr_hz, sr_pcm_hz, mem.back().data());
            if (!enc) {
                throw std::runtime_error("Failed to setup encoder");
            }
            encoders.push_back(enc);
        }
    }

    py::bytes encode(py::buffer pcm, int pcm_fmt, int num_bytes) {
        auto info = pcm.request();
        uint8_t* pcm_ptr = static_cast<uint8_t*>(info.ptr);
        
        int sample_size = 0;
        switch(pcm_fmt) {
            case LC3_PCM_FORMAT_S16: sample_size = 2; break;
            case LC3_PCM_FORMAT_S24_3LE: sample_size = 3; break;
            case LC3_PCM_FORMAT_FLOAT: sample_size = 4; break;
            default: throw std::invalid_argument("Unsupported PCM format");
        }

        int expected_samples = nchannels * frame_samples;
        if (info.size * info.itemsize < expected_samples * sample_size) {
            throw std::invalid_argument("PCM buffer too small");
        }

        std::vector<uint8_t> out_buf(num_bytes);
        int data_offset = 0;

        for (int ich = 0; ich < nchannels; ++ich) {
            int data_size = num_bytes / nchannels + (ich < (num_bytes % nchannels) ? 1 : 0);
            uint8_t* ch_pcm = pcm_ptr + ich * sample_size;
            
            int ret = lc3_encode(encoders[ich], static_cast<lc3_pcm_format>(pcm_fmt), 
                                 ch_pcm, nchannels, data_size, out_buf.data() + data_offset);
            
            if (ret < 0) {
                throw std::runtime_error("lc3_encode failed");
            }
            
            data_offset += data_size;
        }
        
        return py::bytes(reinterpret_cast<char*>(out_buf.data()), num_bytes);
    }

    int get_frame_samples() const { return frame_samples; }
};

class Decoder {
    std::vector<lc3_decoder_t> decoders;
    std::vector<std::vector<uint8_t>> mem;
    int nchannels;
    int frame_samples;

public:
    Decoder(bool hrmode, int dt_us, int sr_hz, int sr_pcm_hz, int nchannels)
        : nchannels(nchannels) {
        frame_samples = lc3_hr_frame_samples(hrmode, dt_us, sr_pcm_hz);
        if (frame_samples < 0) {
            throw std::invalid_argument("Invalid parameters for frame samples");
        }

        unsigned size = lc3_hr_decoder_size(hrmode, dt_us, sr_pcm_hz);
        if (size == 0) {
            throw std::invalid_argument("Invalid parameters for decoder size");
        }

        for (int i = 0; i < nchannels; ++i) {
            mem.emplace_back(size);
            lc3_decoder_t dec = lc3_hr_setup_decoder(hrmode, dt_us, sr_hz, sr_pcm_hz, mem.back().data());
            if (!dec) {
                throw std::runtime_error("Failed to setup decoder");
            }
            decoders.push_back(dec);
        }
    }

    py::bytes decode(py::object data_obj, int pcm_fmt) {
        int sample_size = 0;
        switch(pcm_fmt) {
            case LC3_PCM_FORMAT_S16: sample_size = 2; break;
            case LC3_PCM_FORMAT_S24_3LE: sample_size = 3; break;
            case LC3_PCM_FORMAT_FLOAT: sample_size = 4; break;
            default: throw std::invalid_argument("Unsupported PCM format");
        }

        int out_len = nchannels * frame_samples * sample_size;
        std::vector<uint8_t> out_buf(out_len);

        if (data_obj.is_none()) {
            for (int ich = 0; ich < nchannels; ++ich) {
                uint8_t* ch_pcm = out_buf.data() + ich * sample_size;
                int ret = lc3_decode(decoders[ich], nullptr, 0, static_cast<lc3_pcm_format>(pcm_fmt), 
                                     ch_pcm, nchannels);
                if (ret < 0) {
                    throw std::runtime_error("lc3_decode failed");
                }
            }
        } else {
            py::buffer data_buf(data_obj);
            auto info = data_buf.request();
            uint8_t* data_ptr = static_cast<uint8_t*>(info.ptr);
            int data_len = info.size * info.itemsize;

            int data_offset = 0;
            for (int ich = 0; ich < nchannels; ++ich) {
                int data_size = data_len / nchannels + (ich < (data_len % nchannels) ? 1 : 0);
                uint8_t* ch_pcm = out_buf.data() + ich * sample_size;
                
                int ret = lc3_decode(decoders[ich], data_ptr + data_offset, data_size, 
                                     static_cast<lc3_pcm_format>(pcm_fmt), ch_pcm, nchannels);
                
                if (ret < 0) {
                    throw std::runtime_error("lc3_decode failed");
                }
                
                data_offset += data_size;
            }
        }

        return py::bytes(reinterpret_cast<char*>(out_buf.data()), out_len);
    }

    int get_frame_samples() const { return frame_samples; }
};

PYBIND11_MODULE(_lc3, m) {
    m.doc() = "pybind11 wrapper for liblc3";

    py::class_<Encoder>(m, "Encoder")
        .def(py::init<bool, int, int, int, int>())
        .def("encode", &Encoder::encode)
        .def("get_frame_samples", &Encoder::get_frame_samples);

    py::class_<Decoder>(m, "Decoder")
        .def(py::init<bool, int, int, int, int>())
        .def("decode", &Decoder::decode)
        .def("get_frame_samples", &Decoder::get_frame_samples);

    m.def("lc3_hr_frame_samples", &lc3_hr_frame_samples);
    m.def("lc3_hr_frame_block_bytes", &lc3_hr_frame_block_bytes);
    m.def("lc3_hr_resolve_bitrate", &lc3_hr_resolve_bitrate);
    m.def("lc3_hr_delay_samples", &lc3_hr_delay_samples);
}
