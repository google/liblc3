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

TEST_DIR := test

.PHONY: test test-clean

test:
	$(V)cd $(TEST_DIR) && python3 setup.py && python3 run.py

test-clean:
	$(V)cd $(TEST_DIR) && python3 setup.py clean > /tmp/zero

clean-all: test-clean
