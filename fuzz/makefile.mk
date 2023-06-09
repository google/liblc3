#
# Copyright 2023 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LIB_FUZZING_ENGINE ?= -fsanitize=fuzzer

fuzz_lc3_src += \
    $(FUZZ_DIR)/fuzz_lc3.cc
fuzz_lc3_lib += liblc3
fuzz_lc3_ldflags += -flto $(LIB_FUZZING_ENGINE)
$(eval $(call add-bin,fuzz_lc3))

.PHONY: fuzzers
fuzzers: fuzz_lc3
