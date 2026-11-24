# Wordle - build configuration
#
#   make               build the game          -> build/wordle
#   make run           build and start the game
#   make test          build and run automated tests (no display needed)
#   make memcheck      run the automated tests under Valgrind
#   make memcheck-game play the game under Valgrind (strict, interactive)
#   make manual-tests  build interactive X11 tests (test_gfx, test_input)
#   make debug         build with AddressSanitizer/UBSan -> build/debug/
#   make clean         delete everything under build/

CC := gcc

# Compiler settings, kept in separate variables so `make debug`
# can replace just the OPT part.
CSTD := -std=c11
WARN := -Wall -Wextra -Wpedantic -Wshadow
OPT  := -O2 -g

# CPPFLAGS go to the preprocessor:
#   -Iinclude  look for "header.h" in include/
#   -MMD -MP   write a .d file listing the headers each .c uses, so
#              editing a header rebuilds the files that include it
CPPFLAGS := -Iinclude -MMD -MP
CFLAGS   := $(CSTD) $(WARN) $(OPT)
LDFLAGS  :=
LDLIBS   := -lX11

BUILD_DIR := build
TARGET    := $(BUILD_DIR)/wordle

# Core modules have no X11 dependency, so they can be unit tested.
CORE_SRC := src/game.c src/dictionary.c
# Graphical front end.
APP_SRC  := src/main.c src/ui.c src/font.c src/input.c src/gfx.c

CORE_OBJ := $(CORE_SRC:src/%.c=$(BUILD_DIR)/%.o)
APP_OBJ  := $(APP_SRC:src/%.c=$(BUILD_DIR)/%.o)

TESTS        := $(BUILD_DIR)/test_game $(BUILD_DIR)/test_dictionary
MANUAL_TESTS := $(BUILD_DIR)/test_gfx $(BUILD_DIR)/test_input

.PHONY: all run test memcheck memcheck-game manual-tests debug clean

all: $(TARGET)

# ---- the game -------------------------------------------------------

$(TARGET): $(APP_OBJ) $(CORE_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS)

# Compile any src/X.c into build/X.o.
# "| $(BUILD_DIR)" is an order-only prerequisite: the directory must
# exist, but its timestamp changing does not force a rebuild.
$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: tests/%.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

run: $(TARGET)
	./$(TARGET)

# ---- tests ----------------------------------------------------------

$(BUILD_DIR)/test_game: $(BUILD_DIR)/test_game.o $(BUILD_DIR)/game.o $(BUILD_DIR)/dictionary.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/test_dictionary: $(BUILD_DIR)/test_dictionary.o $(BUILD_DIR)/dictionary.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/test_gfx: $(BUILD_DIR)/test_gfx.o $(BUILD_DIR)/gfx.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD_DIR)/test_input: $(BUILD_DIR)/test_input.o $(BUILD_DIR)/input.o $(BUILD_DIR)/gfx.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS)

# Run each test from the project root (they read data/words.txt).
# Stop at the first failing test and make `make test` fail too.
test: $(TESTS)
	@for t in $(TESTS); do \
		echo "== $$t"; \
		./$$t || { echo "FAILED: $$t"; exit 1; }; \
	done
	@echo "All tests passed."

# Run every test under Valgrind. Fails on any invalid read/write,
# use of uninitialised memory, or leaked allocation.
#   --leak-check=full            report each leak with a stack trace
#   --show-leak-kinds=all        include "still reachable" memory
#   --errors-for-leak-kinds=all  ...and count those as errors too
#   --error-exitcode=1           exit non-zero if anything was found
VALGRIND := valgrind -q --leak-check=full --show-leak-kinds=all \
            --errors-for-leak-kinds=all --error-exitcode=1

memcheck: $(TESTS)
	@for t in $(TESTS); do \
		echo "== valgrind $$t"; \
		$(VALGRIND) ./$$t > /dev/null || { echo "MEMCHECK FAILED: $$t"; exit 1; }; \
	done
	@echo "No memory errors or leaks."

# Play the real game under the same strict Valgrind settings. The game
# frees everything (including X11 resources via XCloseDisplay), so
# any report here is a real problem. Expect it to run slowly.
memcheck-game: $(TARGET)
	@echo "Play, then quit with EXIT or the window close button..."
	@$(VALGRIND) ./$(TARGET) && echo "No memory errors or leaks."

manual-tests: $(MANUAL_TESTS)

# ---- debug build ----------------------------------------------------

# Rebuild everything into build/debug with no optimisation, full debug
# info and runtime checks for memory errors and undefined behaviour.
debug:
	$(MAKE) BUILD_DIR=$(BUILD_DIR)/debug \
		OPT="-O0 -g3 -fno-omit-frame-pointer -fsanitize=address,undefined" \
		LDFLAGS="-fsanitize=address,undefined" \
		all $(TESTS:$(BUILD_DIR)/%=$(BUILD_DIR)/debug/%)

clean:
	rm -rf $(BUILD_DIR)

-include $(wildcard $(BUILD_DIR)/*.d $(BUILD_DIR)/debug/*.d)
