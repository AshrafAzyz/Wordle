#ifndef FONT_H
#define FONT_H

/*
 * Scalable pixel-art font: each character is a 5x7 grid of squares
 * drawn with gfx_fillrectangle() in the current gfx colour.
 *
 * `pixel` is the size of one square in screen pixels, so text at
 * pixel = 4 is 20 x 28 pixels per character. Lowercase is drawn as
 * uppercase; unknown characters are drawn as spaces.
 */

#define FONT_GLYPH_WIDTH  5
#define FONT_GLYPH_HEIGHT 7

/* Draw text with its top-left corner at (x, y). */
void font_draw_text(const char *text, int x, int y, int pixel);

/* Width in screen pixels of `text` at the given pixel size. */
int font_text_width(const char *text, int pixel);

/* Height in screen pixels of one line at the given pixel size. */
int font_text_height(int pixel);

/*
 * 1 if square (col, row) of character `ch` is filled, else 0.
 * Lets other code (e.g. the window icon) reuse the glyph shapes.
 */
int font_glyph_pixel(char ch, int col, int row);

#endif
