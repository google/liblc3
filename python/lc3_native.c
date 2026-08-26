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

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "lc3.h"

/* -------------------------------------------------------------------------
 * Helper functions
 * ------------------------------------------------------------------------- */

static PyObject *py_hr_frame_samples(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"hrmode", "dt_us", "sr_hz", NULL};
    int hrmode = 0, dt_us = 0, sr_hz = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "pii", kwlist, &hrmode, &dt_us, &sr_hz))
        return NULL;

    int ret = lc3_hr_frame_samples(hrmode != 0, dt_us, sr_hz);
    if (ret < 0) {
        PyErr_SetString(PyExc_ValueError, "Bad parameters");
        return NULL;
    }

    return PyLong_FromLong(ret);
}

static PyObject *py_hr_frame_block_bytes(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"hrmode", "dt_us", "sr_hz", "nchannels", "bitrate", NULL};
    int hrmode = 0, dt_us = 0, sr_hz = 0, nchannels = 0, bitrate = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "piiii", kwlist,
                                     &hrmode, &dt_us, &sr_hz, &nchannels, &bitrate))
        return NULL;

    int ret = lc3_hr_frame_block_bytes(hrmode != 0, dt_us, sr_hz, nchannels, bitrate);
    if (ret < 0) {
        PyErr_SetString(PyExc_ValueError, "Bad parameters");
        return NULL;
    }

    return PyLong_FromLong(ret);
}

static PyObject *py_hr_resolve_bitrate(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"hrmode", "dt_us", "sr_hz", "nbytes", NULL};
    int hrmode = 0, dt_us = 0, sr_hz = 0, nbytes = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "piii", kwlist, &hrmode, &dt_us, &sr_hz, &nbytes))
        return NULL;

    int ret = lc3_hr_resolve_bitrate(hrmode != 0, dt_us, sr_hz, nbytes);
    if (ret < 0) {
        PyErr_SetString(PyExc_ValueError, "Bad parameters");
        return NULL;
    }

    return PyLong_FromLong(ret);
}

static PyObject *py_hr_delay_samples(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"hrmode", "dt_us", "sr_hz", NULL};
    int hrmode = 0, dt_us = 0, sr_hz = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "pii", kwlist, &hrmode, &dt_us, &sr_hz))
        return NULL;

    int ret = lc3_hr_delay_samples(hrmode != 0, dt_us, sr_hz);
    if (ret < 0) {
        PyErr_SetString(PyExc_ValueError, "Bad parameters");
        return NULL;
    }

    return PyLong_FromLong(ret);
}

static PyObject *py_hr_encoder_size(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"hrmode", "dt_us", "sr_hz", NULL};
    int hrmode = 0, dt_us = 0, sr_hz = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "pii", kwlist, &hrmode, &dt_us, &sr_hz))
        return NULL;

    unsigned ret = lc3_hr_encoder_size(hrmode != 0, dt_us, sr_hz);
    return PyLong_FromUnsignedLong(ret);
}

static PyObject *py_hr_decoder_size(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"hrmode", "dt_us", "sr_hz", NULL};
    int hrmode = 0, dt_us = 0, sr_hz = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "pii", kwlist, &hrmode, &dt_us, &sr_hz))
        return NULL;

    unsigned ret = lc3_hr_decoder_size(hrmode != 0, dt_us, sr_hz);
    return PyLong_FromUnsignedLong(ret);
}


/* -------------------------------------------------------------------------
 * EncoderContext Object
 * ------------------------------------------------------------------------- */

typedef struct {
    PyObject_HEAD
    bool hrmode;
    int dt_us;
    int sr_hz;
    int sr_pcm_hz;
    int nchannels;
    int frame_samples;
    size_t enc_size;
    lc3_encoder_t *handles;
    void **mem_blocks;
} EncoderContextObject;

static void EncoderContext_dealloc(EncoderContextObject *self)
{
    if (self->mem_blocks) {
        for (int ich = 0; ich < self->nchannels; ich++) {
            if (self->mem_blocks[ich]) {
                PyMem_Free(self->mem_blocks[ich]);
            }
        }
        PyMem_Free(self->mem_blocks);
        self->mem_blocks = NULL;
    }
    if (self->handles) {
        PyMem_Free(self->handles);
        self->handles = NULL;
    }
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static int EncoderContext_init(EncoderContextObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"hrmode", "dt_us", "sr_hz", "sr_pcm_hz", "nchannels", NULL};
    int hrmode_int = 0;
    int dt_us = 0, sr_hz = 0, sr_pcm_hz = 0, nchannels = 1;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "piiii", kwlist,
                                     &hrmode_int, &dt_us, &sr_hz, &sr_pcm_hz, &nchannels))
        return -1;

    if (nchannels <= 0 || nchannels > 16) {
        PyErr_SetString(PyExc_ValueError, "Invalid number of channels");
        return -1;
    }

    bool hrmode = (hrmode_int != 0);
    unsigned enc_size = lc3_hr_encoder_size(hrmode, dt_us, sr_pcm_hz);
    if (enc_size == 0) {
        PyErr_SetString(PyExc_ValueError, "Invalid encoder parameters");
        return -1;
    }

    int frame_samples = lc3_hr_frame_samples(hrmode, dt_us, sr_pcm_hz);
    if (frame_samples <= 0) {
        PyErr_SetString(PyExc_ValueError, "Invalid encoder frame samples");
        return -1;
    }

    self->hrmode = hrmode;
    self->dt_us = dt_us;
    self->sr_hz = sr_hz;
    self->sr_pcm_hz = sr_pcm_hz;
    self->nchannels = nchannels;
    self->frame_samples = frame_samples;
    self->enc_size = enc_size;

    self->handles = (lc3_encoder_t *)PyMem_Calloc(nchannels, sizeof(lc3_encoder_t));
    self->mem_blocks = (void **)PyMem_Calloc(nchannels, sizeof(void *));

    if (!self->handles || !self->mem_blocks) {
        PyErr_NoMemory();
        return -1;
    }

    for (int ich = 0; ich < nchannels; ich++) {
        self->mem_blocks[ich] = PyMem_Malloc(enc_size);
        if (!self->mem_blocks[ich]) {
            PyErr_NoMemory();
            return -1;
        }
        self->handles[ich] = lc3_hr_setup_encoder(
            hrmode, dt_us, sr_hz, sr_pcm_hz, self->mem_blocks[ich]
        );
        if (!self->handles[ich]) {
            PyErr_SetString(PyExc_RuntimeError, "Failed to initialize LC3 encoder");
            return -1;
        }
    }

    return 0;
}

static PyObject *EncoderContext_encode(EncoderContextObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"pcm", "num_bytes", "pcm_format", NULL};
    Py_buffer pcm_buf;
    Py_ssize_t num_bytes = 0;
    int pcm_fmt = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "y*ni", kwlist,
                                     &pcm_buf, &num_bytes, &pcm_fmt))
        return NULL;

    if (num_bytes <= 0) {
        PyBuffer_Release(&pcm_buf);
        PyErr_SetString(PyExc_ValueError, "Invalid num_bytes");
        return NULL;
    }

    int sample_size = 2;
    if (pcm_fmt == LC3_PCM_FORMAT_S16) sample_size = 2;
    else if (pcm_fmt == LC3_PCM_FORMAT_S24_3LE) sample_size = 3;
    else if (pcm_fmt == LC3_PCM_FORMAT_FLOAT) sample_size = 4;
    else if (pcm_fmt == LC3_PCM_FORMAT_S24) sample_size = 4;
    else {
        PyBuffer_Release(&pcm_buf);
        PyErr_SetString(PyExc_ValueError, "Invalid PCM format");
        return NULL;
    }

    int nchannels = self->nchannels;
    int frame_samples = self->frame_samples;
    Py_ssize_t required_pcm_bytes = (Py_ssize_t)nchannels * frame_samples * sample_size;

    const uint8_t *pcm_ptr = (const uint8_t *)pcm_buf.buf;
    uint8_t *padded_pcm = NULL;

    if (pcm_buf.len < required_pcm_bytes) {
        padded_pcm = (uint8_t *)PyMem_Calloc(1, required_pcm_bytes);
        if (!padded_pcm) {
            PyBuffer_Release(&pcm_buf);
            return PyErr_NoMemory();
        }
        memcpy(padded_pcm, pcm_buf.buf, pcm_buf.len);
        pcm_ptr = padded_pcm;
    }

    PyObject *out_bytes = PyBytes_FromStringAndSize(NULL, num_bytes);
    if (!out_bytes) {
        if (padded_pcm) PyMem_Free(padded_pcm);
        PyBuffer_Release(&pcm_buf);
        return NULL;
    }

    uint8_t *out_ptr = (uint8_t *)PyBytes_AsString(out_bytes);
    int ret = 0;

    Py_BEGIN_ALLOW_THREADS

    int data_offset = 0;
    for (int ich = 0; ich < nchannels; ich++) {
        int pcm_offset = ich * sample_size;
        const void *ch_pcm = (const void *)(pcm_ptr + pcm_offset);
        int data_size = (int)(num_bytes / nchannels + (ich < (num_bytes % nchannels)));
        void *ch_out = (void *)(out_ptr + data_offset);
        data_offset += data_size;

        int ch_ret = lc3_encode(
            self->handles[ich],
            (enum lc3_pcm_format)pcm_fmt,
            ch_pcm,
            nchannels,
            data_size,
            ch_out
        );
        if (ch_ret < 0) {
            ret = -1;
            break;
        }
    }

    Py_END_ALLOW_THREADS

    if (padded_pcm) PyMem_Free(padded_pcm);
    PyBuffer_Release(&pcm_buf);

    if (ret < 0) {
        Py_DECREF(out_bytes);
        PyErr_SetString(PyExc_ValueError, "Bad parameters in lc3_encode");
        return NULL;
    }

    return out_bytes;
}

static PyObject *EncoderContext_disable_ltpf(EncoderContextObject *self, PyObject *Py_UNUSED(ignored))
{
    for (int ich = 0; ich < self->nchannels; ich++) {
        lc3_encoder_disable_ltpf(self->handles[ich]);
    }
    Py_RETURN_NONE;
}

static PyMethodDef EncoderContext_methods[] = {
    {"encode", (PyCFunction)EncoderContext_encode, METH_VARARGS | METH_KEYWORDS, "Encode a PCM frame"},
    {"disable_ltpf", (PyCFunction)EncoderContext_disable_ltpf, METH_NOARGS, "Disable LTPF analysis"},
    {NULL}
};

static PyTypeObject EncoderContextType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "_lc3.EncoderContext",
    .tp_doc = "LC3 Native Encoder Context",
    .tp_basicsize = sizeof(EncoderContextObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc)EncoderContext_init,
    .tp_dealloc = (destructor)EncoderContext_dealloc,
    .tp_methods = EncoderContext_methods,
};


/* -------------------------------------------------------------------------
 * DecoderContext Object
 * ------------------------------------------------------------------------- */

typedef struct {
    PyObject_HEAD
    bool hrmode;
    int dt_us;
    int sr_hz;
    int sr_pcm_hz;
    int nchannels;
    int frame_samples;
    size_t dec_size;
    lc3_decoder_t *handles;
    void **mem_blocks;
} DecoderContextObject;

static void DecoderContext_dealloc(DecoderContextObject *self)
{
    if (self->mem_blocks) {
        for (int ich = 0; ich < self->nchannels; ich++) {
            if (self->mem_blocks[ich]) {
                PyMem_Free(self->mem_blocks[ich]);
            }
        }
        PyMem_Free(self->mem_blocks);
        self->mem_blocks = NULL;
    }
    if (self->handles) {
        PyMem_Free(self->handles);
        self->handles = NULL;
    }
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static int DecoderContext_init(DecoderContextObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"hrmode", "dt_us", "sr_hz", "sr_pcm_hz", "nchannels", NULL};
    int hrmode_int = 0;
    int dt_us = 0, sr_hz = 0, sr_pcm_hz = 0, nchannels = 1;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "piiii", kwlist,
                                     &hrmode_int, &dt_us, &sr_hz, &sr_pcm_hz, &nchannels))
        return -1;

    if (nchannels <= 0 || nchannels > 16) {
        PyErr_SetString(PyExc_ValueError, "Invalid number of channels");
        return -1;
    }

    bool hrmode = (hrmode_int != 0);
    unsigned dec_size = lc3_hr_decoder_size(hrmode, dt_us, sr_pcm_hz);
    if (dec_size == 0) {
        PyErr_SetString(PyExc_ValueError, "Invalid decoder parameters");
        return -1;
    }

    int frame_samples = lc3_hr_frame_samples(hrmode, dt_us, sr_pcm_hz);
    if (frame_samples <= 0) {
        PyErr_SetString(PyExc_ValueError, "Invalid decoder frame samples");
        return -1;
    }

    self->hrmode = hrmode;
    self->dt_us = dt_us;
    self->sr_hz = sr_hz;
    self->sr_pcm_hz = sr_pcm_hz;
    self->nchannels = nchannels;
    self->frame_samples = frame_samples;
    self->dec_size = dec_size;

    self->handles = (lc3_decoder_t *)PyMem_Calloc(nchannels, sizeof(lc3_decoder_t));
    self->mem_blocks = (void **)PyMem_Calloc(nchannels, sizeof(void *));

    if (!self->handles || !self->mem_blocks) {
        PyErr_NoMemory();
        return -1;
    }

    for (int ich = 0; ich < nchannels; ich++) {
        self->mem_blocks[ich] = PyMem_Malloc(dec_size);
        if (!self->mem_blocks[ich]) {
            PyErr_NoMemory();
            return -1;
        }
        self->handles[ich] = lc3_hr_setup_decoder(
            hrmode, dt_us, sr_hz, sr_pcm_hz, self->mem_blocks[ich]
        );
        if (!self->handles[ich]) {
            PyErr_SetString(PyExc_RuntimeError, "Failed to initialize LC3 decoder");
            return -1;
        }
    }

    return 0;
}

static PyObject *DecoderContext_decode(DecoderContextObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"data", "pcm_format", NULL};
    PyObject *data_obj = NULL;
    int pcm_fmt = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "Oi", kwlist, &data_obj, &pcm_fmt))
        return NULL;

    Py_buffer in_buf;
    bool has_in_buf = false;
    const uint8_t *in_ptr = NULL;
    Py_ssize_t in_len = 0;

    if (data_obj != Py_None) {
        if (PyObject_GetBuffer(data_obj, &in_buf, PyBUF_SIMPLE) != 0) {
            return NULL;
        }
        has_in_buf = true;
        in_ptr = (const uint8_t *)in_buf.buf;
        in_len = in_buf.len;
    }

    int sample_size = 2;
    if (pcm_fmt == LC3_PCM_FORMAT_S16) sample_size = 2;
    else if (pcm_fmt == LC3_PCM_FORMAT_S24_3LE) sample_size = 3;
    else if (pcm_fmt == LC3_PCM_FORMAT_FLOAT) sample_size = 4;
    else if (pcm_fmt == LC3_PCM_FORMAT_S24) sample_size = 4;
    else {
        if (has_in_buf) PyBuffer_Release(&in_buf);
        PyErr_SetString(PyExc_ValueError, "Invalid PCM format");
        return NULL;
    }

    int nchannels = self->nchannels;
    int frame_samples = self->frame_samples;
    Py_ssize_t out_len = (Py_ssize_t)nchannels * frame_samples * sample_size;

    PyObject *out_bytes = PyBytes_FromStringAndSize(NULL, out_len);
    if (!out_bytes) {
        if (has_in_buf) PyBuffer_Release(&in_buf);
        return NULL;
    }

    uint8_t *pcm_out = (uint8_t *)PyBytes_AsString(out_bytes);
    memset(pcm_out, 0, out_len);

    int ret = 0;

    Py_BEGIN_ALLOW_THREADS

    int data_offset = 0;
    for (int ich = 0; ich < nchannels; ich++) {
        int pcm_offset = ich * sample_size;
        void *ch_pcm = (void *)(pcm_out + pcm_offset);

        if (!has_in_buf) {
            int ch_ret = lc3_decode(
                self->handles[ich],
                NULL,
                0,
                (enum lc3_pcm_format)pcm_fmt,
                ch_pcm,
                nchannels
            );
            if (ch_ret < 0) {
                ret = -1;
                break;
            }
        } else {
            int data_size = (int)(in_len / nchannels + (ich < (in_len % nchannels)));
            const void *ch_in = (const void *)(in_ptr + data_offset);
            data_offset += data_size;

            int ch_ret = lc3_decode(
                self->handles[ich],
                ch_in,
                data_size,
                (enum lc3_pcm_format)pcm_fmt,
                ch_pcm,
                nchannels
            );
            if (ch_ret < 0) {
                ret = -1;
                break;
            }
        }
    }

    Py_END_ALLOW_THREADS

    if (has_in_buf) PyBuffer_Release(&in_buf);

    if (ret < 0) {
        Py_DECREF(out_bytes);
        PyErr_SetString(PyExc_ValueError, "Bad parameters in lc3_decode");
        return NULL;
    }

    return out_bytes;
}

static PyMethodDef DecoderContext_methods[] = {
    {"decode", (PyCFunction)DecoderContext_decode, METH_VARARGS | METH_KEYWORDS, "Decode an LC3 frame"},
    {NULL}
};

static PyTypeObject DecoderContextType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "_lc3.DecoderContext",
    .tp_doc = "LC3 Native Decoder Context",
    .tp_basicsize = sizeof(DecoderContextObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc)DecoderContext_init,
    .tp_dealloc = (destructor)DecoderContext_dealloc,
    .tp_methods = DecoderContext_methods,
};


/* -------------------------------------------------------------------------
 * Module Definition
 * ------------------------------------------------------------------------- */

static PyMethodDef module_methods[] = {
    {"hr_frame_samples", (PyCFunction)py_hr_frame_samples, METH_VARARGS | METH_KEYWORDS, "Return PCM samples in frame"},
    {"hr_frame_block_bytes", (PyCFunction)py_hr_frame_block_bytes, METH_VARARGS | METH_KEYWORDS, "Return frame block size"},
    {"hr_resolve_bitrate", (PyCFunction)py_hr_resolve_bitrate, METH_VARARGS | METH_KEYWORDS, "Resolve bitrate from size"},
    {"hr_delay_samples", (PyCFunction)py_hr_delay_samples, METH_VARARGS | METH_KEYWORDS, "Return algorithmic delay samples"},
    {"hr_encoder_size", (PyCFunction)py_hr_encoder_size, METH_VARARGS | METH_KEYWORDS, "Return encoder size"},
    {"hr_decoder_size", (PyCFunction)py_hr_decoder_size, METH_VARARGS | METH_KEYWORDS, "Return decoder size"},
    {NULL}
};

static struct PyModuleDef module_def = {
    PyModuleDef_HEAD_INIT,
    .m_name = "_lc3",
    .m_doc = "Native C extension for liblc3",
    .m_size = -1,
    .m_methods = module_methods,
};

PyMODINIT_FUNC PyInit__lc3(void)
{
    if (PyType_Ready(&EncoderContextType) < 0)
        return NULL;
    if (PyType_Ready(&DecoderContextType) < 0)
        return NULL;

    PyObject *m = PyModule_Create(&module_def);
    if (!m)
        return NULL;

    Py_INCREF(&EncoderContextType);
    if (PyModule_AddObject(m, "EncoderContext", (PyObject *)&EncoderContextType) < 0) {
        Py_DECREF(&EncoderContextType);
        Py_DECREF(m);
        return NULL;
    }

    Py_INCREF(&DecoderContextType);
    if (PyModule_AddObject(m, "DecoderContext", (PyObject *)&DecoderContextType) < 0) {
        Py_DECREF(&DecoderContextType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}
