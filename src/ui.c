#include "ui.h"
#include "gfx.h"

#include <stdio.h>


#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 700

#define TILE_SIZE 70
#define TILE_GAP 10

#define BOARD_WIDTH \
    (WORD_LENGTH * TILE_SIZE + \
     (WORD_LENGTH - 1) * TILE_GAP)

#define BOARD_HEIGHT \
    (MAX_GUESSES * TILE_SIZE + \
     (MAX_GUESSES - 1) * TILE_GAP)

#define BOARD_Y 120

static int board_x(void)
{
    return (gfx_xsize() - BOARD_WIDTH) / 2;
}

/*
 * Wordle colors.
 */
#define COLOR_BACKGROUND_R 247
#define COLOR_BACKGROUND_G 241
#define COLOR_BACKGROUND_B 227

#define COLOR_BORDER_R 120
#define COLOR_BORDER_G 120
#define COLOR_BORDER_B 120

#define COLOR_CORRECT_R 83
#define COLOR_CORRECT_G 141
#define COLOR_CORRECT_B 78

#define COLOR_PRESENT_R 181
#define COLOR_PRESENT_G 159
#define COLOR_PRESENT_B 59

#define COLOR_ABSENT_R 128
#define COLOR_ABSENT_G 128
#define COLOR_ABSENT_B 128

#define COLOR_TEXT_R 20
#define COLOR_TEXT_G 20
#define COLOR_TEXT_B 20


static void draw_empty_board(void);

static void draw_tile(
    int x,
    int y,
    LetterResult result
);

static void draw_letter(
    char letter,
    int x,
    int y
);


void ui_init(void)
{
    gfx_open(
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        "Wordle"
    );

    gfx_clear_color(
        COLOR_BACKGROUND_R,
        COLOR_BACKGROUND_G,
        COLOR_BACKGROUND_B
    );

    gfx_clear();

    ui_display_title();
    ui_display_instructions();

    gfx_flush();
}


void ui_display_title(void)
{
    gfx_color(
        COLOR_TEXT_R,
        COLOR_TEXT_G,
        COLOR_TEXT_B
    );

    gfx_text(
        "WORDLE",
        335,
        60
    );
}


void ui_display_instructions(void)
{
    gfx_color(
        80,
        80,
        80
    );

    gfx_text(
        "Guess the 5-letter word",
        285,
        90
    );

    draw_empty_board();
}


static void draw_empty_board(void)
{
    for (int row = 0; row < MAX_GUESSES; row++) {

        for (int col = 0; col < WORD_LENGTH; col++) {

            int x =
                board_x() +
                col * (TILE_SIZE + TILE_GAP);

            int y =
                BOARD_Y +
                row * (TILE_SIZE + TILE_GAP);

            draw_tile(
                x,
                y,
                LETTER_ABSENT
            );
        }
    }
}


static void draw_tile(
    int x,
    int y,
    LetterResult result
)
{
    /*
     * Empty tiles use only an outline.
     */
    if (result == LETTER_ABSENT) {

        gfx_color(
            COLOR_BORDER_R,
            COLOR_BORDER_G,
            COLOR_BORDER_B
        );

        gfx_rectangle(
            x,
            y,
            TILE_SIZE,
            TILE_SIZE
        );

        return;
    }

    /*
     * Correct letter.
     */
    if (result == LETTER_CORRECT) {

        gfx_color(
            COLOR_CORRECT_R,
            COLOR_CORRECT_G,
            COLOR_CORRECT_B
        );
    }

    /*
     * Present letter.
     */
    else if (result == LETTER_PRESENT) {

        gfx_color(
            COLOR_PRESENT_R,
            COLOR_PRESENT_G,
            COLOR_PRESENT_B
        );
    }

    /*
     * Letter absent.
     */
    else {

        gfx_color(
            COLOR_ABSENT_R,
            COLOR_ABSENT_G,
            COLOR_ABSENT_B
        );
    }

    gfx_fillrectangle(
        x,
        y,
        TILE_SIZE,
        TILE_SIZE
    );
}


static void draw_letter(
    char letter,
    int x,
    int y
)
{
    char text[2];

    text[0] = letter;
    text[1] = '\0';

    gfx_color(
        COLOR_TEXT_R,
        COLOR_TEXT_G,
        COLOR_TEXT_B
    );

    gfx_text(
        text,
        x,
        y
    );
}


void ui_draw_game(const Game *game)
{
    if (game == NULL) {
        return;
    }

    /*
     * Redraw the complete board.
     */
    for (int row = 0; row < MAX_GUESSES; row++) {

        for (int col = 0; col < WORD_LENGTH; col++) {

            int x =
                board_x() +
                col * (TILE_SIZE + TILE_GAP);

            int y =
                BOARD_Y +
                row * (TILE_SIZE + TILE_GAP);

            /*
             * Empty row.
             */
            if (row >= game->current_guess) {

                draw_tile(
                    x,
                    y,
                    LETTER_ABSENT
                );

                continue;
            }

            /*
             * Draw the submitted guess.
             */
            GuessResult *guess =
                (GuessResult *)&game->guesses[row];

            draw_tile(
                x,
                y,
                guess->results[col]
            );

            draw_letter(
                guess->letters[col],
                x + 27,
                y + 45
            );
        }
    }

    gfx_flush();
}


void ui_display_win(int guesses)
{
    char message[64];

    snprintf(
        message,
        sizeof(message),
        "YOU WON!  %d GUESSES",
        guesses
    );

    gfx_color(
        COLOR_CORRECT_R,
        COLOR_CORRECT_G,
        COLOR_CORRECT_B
    );

    gfx_text(
        message,
        285,
        660
    );

    gfx_flush();
}


void ui_display_game_over(const char *answer)
{
    char message[64];

    if (answer == NULL) {
        return;
    }

    snprintf(
        message,
        sizeof(message),
        "GAME OVER - %s",
        answer
    );

    gfx_color(
        200,
        60,
        60
    );

    gfx_text(
        message,
        300,
        660
    );

    gfx_flush();
}

void ui_draw_current_guess(
    const Game *game,
    const char *guess
)
{
    if (game == NULL || guess == NULL) {
        return;
    }

    int row = game->current_guess;

    if (row >= MAX_GUESSES) {
        return;
    }

    /*
     * Clear the current row.
     */
    for (int col = 0; col < WORD_LENGTH; col++) {

        int x =
            board_x() +
            col * (TILE_SIZE + TILE_GAP);

        int y =
            BOARD_Y +
            row * (TILE_SIZE + TILE_GAP);

        draw_tile(
            x,
            y,
            LETTER_ABSENT
        );
    }

    /*
     * Draw letters currently typed.
     */
    for (int col = 0; guess[col] != '\0'; col++) {

        int x =
            board_x() +
            col * (TILE_SIZE + TILE_GAP);

        int y =
            BOARD_Y +
            row * (TILE_SIZE + TILE_GAP);

        draw_letter(
            guess[col],
            x + 27,
            y + 45
        );
    }

    gfx_flush();
}

void ui_shutdown(void)
{
    /*
     * X11 cleanup will be added to gfx.c
     * when the graphics backend is expanded.
     */
}
