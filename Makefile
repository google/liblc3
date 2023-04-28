#
# Copyright 2022 Google LLC
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

V ?= @

BUILD_DIR := build
BIN_DIR := bin


#
# Set `gcc` as default compiler
#

CC := $(if $(CC)=cc,gcc,$(CC))
AS := $(if $(AS)=as,$(CC),$(AS))
LD := $(if $(LD)=ld,$(CC),$(LD))

CFLAGS := $(if $(DEBUG),-O0 -g,-O3)
CFLAGS += -std=c11 -Wall -Wextra -Wdouble-promotion -Wvla


#
# Declarations
#

lib_list :=
bin_list :=

define add-lib
    $(eval $(1)_bin ?= $(1).a)
    $(eval $(1)_bin := $(addprefix $(BIN_DIR)/,$($(1)_bin)))

    lib_list += $(1)
    LIB += $($(1)_bin)
endef

define add-bin
    $(eval $(1)_bin ?= $(1))
    $(eval $(1)_bin := $(addprefix $(BIN_DIR)/,$($(1)_bin)))

    $($(1)_bin): LDLIBS += $(if $(filter $(LIBC),bionic),\
      $(filter-out rt pthread,$($(1)_ldlibs)),$($(1)_ldlibs))
    $($(1)_bin): LDFLAGS += $($(1)_ldflags)

    bin_list += $(1)
    BIN += $($(1)_bin)
endef

define set-target
    $(eval $(1)_obj ?= $(patsubst %.c,%.o,$(filter %.c,$($(1)_src))) \
                       $(patsubst %.s,%.o,$(filter %.s,$($(1)_src))))
    $(eval $(1)_obj := $(addprefix $(BUILD_DIR)/,$($(1)_obj)))
    $(eval $(1)_lib := $(foreach lib, $($(1)_lib), $($(lib)_bin)))

    $($(1)_obj): INCLUDE += $($(1)_include)
    $($(1)_obj): DEFINE  += $($(1)_define)
    $($(1)_obj): CFLAGS  += $($(1)_cflags)

    -include $($(1)_obj:.o=.d)

    $($(1)_bin): $($(1)_lib)
    $($(1)_bin): $($(1)_obj)
    $($(1)_bin): $($(1)_dependencies)

    .PHONY: $(1)
    $(1): $($(1)_bin)
endef

.PHONY: default
default:


INCLUDE += include

SRC_DIR = src
include $(SRC_DIR)/makefile.mk

TOOLS_DIR = tools
-include $(TOOLS_DIR)/makefile.mk

TEST_DIR := test
-include $(TEST_DIR)/makefile.mk


#
# Rules
#

MAKEFILE_DEPS := $(MAKEFILE_LIST)

$(foreach lib, $(lib_list), $(eval $(call set-target,$(lib))))
$(foreach bin, $(bin_list), $(eval $(call set-target,$(bin))))

$(BUILD_DIR)/%.o: %.c $(MAKEFILE_DEPS)
	@echo "  CC      $(notdir $<)"
	$(V)mkdir -p $(dir $@)
	$(V)$(CC) $< -c $(CFLAGS) \
	    $(addprefix -I,$(INCLUDE)) \
	    $(addprefix -D,$(DEFINE)) -MMD -MF $(@:.o=.d) -o $@

$(BUILD_DIR)/%.o: %.s $(MAKEFILE_DEPS)
	@echo "  AS      $(notdir $<)"
	$(V)mkdir -p $(dir $@)
	$(V)$(AS) $< -c $(CFLAGS) \
	    $(addprefix -I,$(INCLUDE)) \
	    $(addprefix -D,$(DEFINE)) -MMD -MF $(@:.o=.d) -o $@

$(LIB): $(MAKEFILE_DEPS)
	@echo "  AR      $(notdir $@)"
	$(V)mkdir -p $(dir $@)
	$(V)$(AR) rcs $@ $(filter %.o,$^)

$(BIN): $(MAKEFILE_DEPS)
	@echo "  LD      $(notdir $@)"
	$(V)mkdir -p $(dir $@)
	$(V)$(LD) $(filter %.o,$^) $(filter %.a,$^) $(LDFLAGS) \
	    $(addprefix -l,$(LDLIBS)) -o $@

clean:
	$(V)rm -rf $(BUILD_DIR)
	$(V)rm -rf $(BIN_DIR)

clean-all: clean
