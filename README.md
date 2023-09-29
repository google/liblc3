# Low Complexity Communication Codec (LC3)

The LC3 is an efficient low latency audio codec.

[_Low Complexity Communication Codec_](https://www.bluetooth.org/DocMan/handlers/DownloadDoc.ashx?doc_id=502107&vId=542963)

## Overview

The directory layout is as follows :
- include:      Library interface
- src:          Source files
- tools:        Standalone encoder/decoder tools
- test:         Python implentation, used as reference for unit testing
- fuzz:         Roundtrip fuzz testing harness
- build:        Building outputs
- bin:          Compilation output

## How to build

The default toolchain used is GCC. Invoke `make` to build the library.

```sh
$ make -j
```

Compiled library `liblc3.a` will be found in `bin` directory.

#### Cross compilation

The cc, as, ld and ar can be selected with respective Makefile variables `CC`,
`AS`, `LD` and `AR`. The `AS` and `LD` selections are optionnal, and fallback
to `CC` selection when not defined.

The `LIBC` must be set to `bionic` for android cross-compilation. This switch
prevent link with `pthread` and `rt` libraries, that is included in the
bionic libc.

Following example build for android, using NDK toolset.

```sh
$ make -j CC=path_to_android_ndk_prebuilt/toolchain-prefix-clang LIBC=bionic
```

Compiled library will be found in `bin` directory.

## Tools

Tools can be all compiled, while involking `make` as follows :

```sh
$ make tools
```

The standalone encoder `elc3` take a `wave` file as input and encode it
according given parameter. The LC3 binary file format used is the non
standard format described by the reference encoder / decoder tools.
The standalone decoder `dlc3` do the inverse operation.

Refer to `elc3 -h` or `dlc3 -h` for options.

Note that `elc3` output bitstream to standard output when output file is
omitted. On the other side `dlc3` read from standard input when input output
file are omitted.
In such way you can easly test encoding / decoding loop with :

```sh
$ ./elc3 <in.wav> -b <bitrate> | ./dlc3 > <out.wav>
```

Adding Linux `aplay` tools, you will be able to instant hear the result :

```sh
$ ./elc3 <in.wav> -b <bitrate> | ./dlc3 | aplay
```

## Test

A python implementation of the encoder is provided in `test` diretory.
The C implementation is unitary validated against this implementation and
intermediate values given in Appendix C of the specification.

#### Prerequisite

```sh
# apt install python3 python3-dev python3-pip
$ pip3 install scipy numpy
```

#### Running test suite

```sh
$ make test
```

## Fuzzing

Roundtrip fuzz testing harness is available in `fuzz` directory.
LLVM `clang` and `clang++` compilers are needed to run fuzzing.

The encoder and decoder fuzzers can be run, for 1 million iterations, using
target respectively `dfuzz` and `efuzz`. The `fuzz` target runs both.

```sh
$ make efuzz    # Run encoder fuzzer for 1M iteration
$ make dfuzz    # Run decoder fuzzer for 1M iteration
$ make fuzz -j  # Run encoder and decoder fuzzers in parallel
```

## Qualification / Conformance

The implementation is qualified under the [_QDID 194161_](https://launchstudio.bluetooth.com/ListingDetails/160904) as part of Google Fluoride 1.5.

The conformance reports can be found [here](conformance/README.md)

## Listening Test

The codec was [_here_](https://hydrogenaud.io/index.php/topic,122575.0.html)
subjectively evaluated in a blind listening test.

## Meson build system

Meson build system is also available to build and install lc3 codec in Linux
environment.

```sh
$ meson build
$ cd build
$ ninja
$ sudo ninja install
```

