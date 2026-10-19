# Reuse Billiard's ICK policy rather than create another compiler gate.
BILLIARD_SOURCE_DIR ?=
ICK_CC ?=
ICK_SOURCE_DIR ?=
ICK_LINK_FLAGS ?= -fno-link-libatomic
ICK_CFLAGS = -O2 -std=c11 -Wall -Wextra -Werror -pedantic -ffp-contract=off -fno-fast-math
BILLIARD_SHA = b2aaed84cb24bf3d3510c6ed5e4b3baa09f3b02a

.PHONY: check-ick test receipt clean
check-ick:
	@test -n "$(BILLIARD_SOURCE_DIR)" || { echo 'BLOCKED: pinned BILLIARD_SOURCE_DIR required' >&2; exit 3; }
	@test "$$(git -C "$(BILLIARD_SOURCE_DIR)" rev-parse HEAD)" = "$(BILLIARD_SHA)" || { echo 'BLOCKED: Billiard source revision mismatch' >&2; exit 3; }
	@test -z "$$(git -C "$(BILLIARD_SOURCE_DIR)" status --porcelain --untracked-files=no)" || { echo 'BLOCKED: dirty Billiard source' >&2; exit 3; }
	$(MAKE) -C "$(BILLIARD_SOURCE_DIR)" check-ick ICK_CC="$(ICK_CC)" ICK_SOURCE_DIR="$(ICK_SOURCE_DIR)"

build:
	mkdir -p build

build/rectangle_modes_test: src/rectangle_modes.c src/rectangle_modes.h tests/rectangle_modes_test.c | check-ick build
	"$(ICK_CC)" $(ICK_LINK_FLAGS) $(ICK_CFLAGS) src/rectangle_modes.c tests/rectangle_modes_test.c -lm -o $@

build/rectangle_experiment_test: src/rectangle_modes.c src/rectangle_modes.h src/rectangle_experiment.c src/rectangle_experiment.h tests/rectangle_experiment_test.c | check-ick build
	"$(ICK_CC)" $(ICK_LINK_FLAGS) $(ICK_CFLAGS) -Isrc -I"$(BILLIARD_SOURCE_DIR)/src" src/rectangle_modes.c src/rectangle_experiment.c "$(BILLIARD_SOURCE_DIR)/src/billiard.c" tests/rectangle_experiment_test.c -lm -o $@

build/chladni: src/rectangle_modes.c src/rectangle_modes.h src/main.c | check-ick build
	"$(ICK_CC)" $(ICK_LINK_FLAGS) $(ICK_CFLAGS) src/rectangle_modes.c src/main.c -lm -o $@

build/cli_test: tests/cli_test.c | check-ick build
	"$(ICK_CC)" $(ICK_LINK_FLAGS) $(ICK_CFLAGS) tests/cli_test.c -o $@

test: build/rectangle_modes_test build/rectangle_experiment_test build/chladni build/cli_test
	./build/rectangle_modes_test > build/rectangle-modes.log
	cat build/rectangle-modes.log
	./build/rectangle_experiment_test > build/rectangle-experiment.log
	cat build/rectangle-experiment.log
	./build/cli_test "$(abspath build/chladni)" > build/cli.log
	cat build/cli.log
	$(MAKE) -C "$(BILLIARD_SOURCE_DIR)" test ICK_CC="$(ICK_CC)" ICK_SOURCE_DIR="$(ICK_SOURCE_DIR)"

receipt: | check-ick build
	git rev-parse HEAD > build/chladni-source.txt
	git -C "$(BILLIARD_SOURCE_DIR)" rev-parse HEAD > build/billiard-source.txt
	git -C "$(ICK_SOURCE_DIR)" rev-parse HEAD > build/ick-source.txt
	git -C "$(ICK_SOURCE_DIR)/gcc" rev-parse HEAD > build/ick-reference.txt
	sha256sum "$(ICK_CC)" > build/compiler.sha256
	sha256sum "$$('$(ICK_CC)' -print-prog-name=cc1)" > build/cc1.sha256
	sha256sum build/rectangle_modes_test build/rectangle_experiment_test build/chladni build/cli_test build/*.log > build/artifacts.sha256

clean:
	rm -rf build
