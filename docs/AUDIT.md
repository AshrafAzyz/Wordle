# M0 Audit — Baseline (2026-11-24)

This records the state of the project when the release work began.
Later milestones fix the issues listed here.

## Two versions of the game

| | Legacy `src/wordle.c` | Modular `main.c` + `game` / `dictionary` / `input` / `ui` / `gfx` |
|---|---|---|
| Features | Menu, How to Play, pixel-art logo, on-screen keyboard, end/exit screens | Keyboard play on the board only |
| Builds on Linux | No (`<windows.h>`, `Sleep()`) | Yes, warning-free with `-Wall -Wextra -Wpedantic` |

The modular version is the base. The legacy screens get ported into `ui.c` (M4), and then `wordle.c` is deleted.

## Bugs in the modular code

1. `ui.c` uses `LETTER_ABSENT` for both "empty tile" and "grey letter", so grey letters render as empty outlines.
2. `ui.c`: clearing a row only redraws outlines, so letters removed with Backspace stay visible.
3. `main.c`: the program exits right after drawing win/loss, so the window closes at once. It never calls `XCloseDisplay`.
4. `main.c`: a guess not in the dictionary is ignored silently.
5. `dictionary.c`: long lines are split by `fgets` (`ABCDEFGHIJK` loads `GHIJK`).
6. `dictionary.c` / `game.c`: no A–Z validation or case normalisation (`"12345"` is accepted).
7. `game.c` stores a pointer to the answer instead of copying it, which is fragile once there is a new-game loop.
8. `gfx.c`: `gfx_toggle_fullscreen` can block forever and discards input events while it waits.
9. `gfx.c`: `gfx_wait` returns `char` (cuts off KeySyms) and reads click positions from `xkey` instead of `xbutton`.
10. `ui.c`: the title and instructions are drawn at fixed x positions, not centred.
11. `main.c`: relative dictionary path; errors printed to stdout.

Duplicate-letter scoring in `game.c` was verified correct
(ABBEY/BABES → `YYGG-`, CRANE/EERIE → `--Y-G`).

## Bugs in the legacy code (do not port)

- `if (c = 0x01 && ...)` assigns instead of comparing.
- `calloc(6)` then `c[6] = '\0'` writes one byte past the end of a heap buffer.
- `memset(p, 0, sizeof(p))` on `malloc(2)` buffers: `sizeof` of a pointer is 8.
- `memset(guess-2, ...)` writes before the start of a stack buffer.
- `strncpy` into `char[5]`: no terminating `'\0'`, so `strcmp` reads past the end.
- Unbounded `fscanf("%s")`; `fopen` result never checked.
- `Sleep()` in milliseconds (about 4 minutes); `sleep(0.5)` becomes `sleep(0)`.
- Duplicate-letter scoring is wrong (CRANE/EERIE → `YYY-G`).
- Word list and score strings leak; layout hard-coded to 1920×1080.

## Build status

- The modular game, `test_game`, `test_dictionary` and `test_gfx` compile with no warnings.
- `tests/test_input.c` does not compile: it uses the old `input_get_guess` API. It will be replaced in M1.
- `test_game` and `test_dictionary` are clean under Valgrind, but they only print results and assert nothing.

## Decisions

- Delete `wordle.c` after the M4 port.
- The UI must be resolution-independent.
- Licence: MIT.
- Goal: a distributable application with its own icon.
