/*
    racketball.c
    Built off of fonts and a little bit of sprite drawing from sgb_pong
    IDGeek

    fonts.c
    Simple example of how to use multiple fonts on the GB
    Michael Hope, 1999.
*/

#include "Export.h"

#include <stdio.h>
#include <gb/font.h>
#include <gb/console.h>
#include <gb/drawing.h>

/* Some macros to help with fixed point notation */
#define TO_FIXED(x) ((x) << 4)
#define TO_INT(x) ((x) >> 4)
#define FIXED_MUL(x, y) (((x) * (y)) >> 4)

joypads_t joypads;

/* Helper for drawing the paddle. Shamelessly ~~stolen~~ borrowed from sgb_pong */
void draw_pad(UINT8 x, UINT8 y) {
    move_sprite(1, x, y);
    move_sprite(2, x, y + 8);
    move_sprite(3, x, y + 16);
}

void main(void) {
    font_t ibm_font;
    UINT32 score = 0;

    /* Ball pos/spd use a fixed point system where the lowest 4 bits containing the fractional part (0x0-0xF) */
    INT16 pos_x = 0;
    INT16 pos_y = 0;
    INT16 spd_x = 0;
    INT16 spd_y = 0;

    UINT8 pad_y = 76;
    UINT8 spd_pad = 3;

    BOOLEAN ball_launched = FALSE;
    joypads_t joypads;
    UINT8 framecount = 0;

    joypad_init(1, &joypads); /* Initialize one joypad */

    /* Load sprite data (Stored in Export.h) */
    set_sprite_data(0, 4, TileLabel);
    set_sprite_tile(0, 0);
    set_sprite_tile(1, 1);
    set_sprite_tile(2, 2);
    set_sprite_tile(3, 3);
    SHOW_SPRITES;

    /* Init the font system */
    font_init();
    ibm_font = font_load(font_ibm);  /* 96 tiles */
    
    /* Load this one with dk grey background and white foreground */
    color(WHITE, DKGREY, SOLID);

    /* Turn scrolling off (why not?) */
    mode(get_mode() | M_NO_SCROLL);
    
    /* IBM font */
    font_set(ibm_font);

    while (TRUE) {
        joypad_ex(&joypads); /* Poll inputs */

        /* Game logic */

        /* These should happen regardless of whether the game has started or not */
        if (joypads.joy0 & J_SELECT)
            ball_launched = FALSE;
        if (joypads.joy0 & J_DOWN) {
            if (joypads.joy0 & J_B)
                pad_y += spd_pad * 2;
            else
                pad_y += spd_pad;
            if (pad_y >= 136)
                pad_y = 136;
        }
        else if (joypads.joy0 & J_UP) {
            if (joypads.joy0 & J_B)
                pad_y -= spd_pad * 2;
            else
                pad_y -= spd_pad;
            if (pad_y <= 24)
                pad_y = 24;
        }

        /* Game started */
        if (ball_launched) {
            pos_x += spd_x;
            pos_y += spd_y;

            /* This mess handles collision on the left side of the screen, be it the paddle or the wall */
            if (spd_x < TO_FIXED(0)) {
                /* Hits top or bottom of paddle. Send it at a steep angle */
                if (pos_x > TO_FIXED(0) && pos_x < TO_FIXED(8)) {
                    /* Hits the top half */
                    if (pos_y >= TO_FIXED(pad_y - 4) && pos_y < TO_FIXED(pad_y + 8)) {
                        if (spd_y < TO_FIXED(0))
                            spd_y -= TO_FIXED(2);
                        else if (spd_y > TO_FIXED(0))
                            spd_y = TO_FIXED(-3);
                        spd_x = spd_x * -1 + 0x1;
                        score++;
                    }
                    /* Hits the bottom half */
                    else if (pos_y >= TO_FIXED(pad_y + 8) && pos_y < TO_FIXED(pad_y + 20)) {
                        if (spd_y > TO_FIXED(0))
                            spd_y += TO_FIXED(2);
                        else if (spd_y < TO_FIXED(0))
                            spd_y = TO_FIXED(3);
                        spd_x = spd_x * -1 + 0x1;
                        score++;
                    }
                }
                /* Hits around the front of the paddle */
                else if (pos_x >= TO_FIXED(8) && pos_x < TO_FIXED(16)) {
                    /* Hits near the top */
                    if (pos_y >= TO_FIXED(pad_y - 4) && pos_y < TO_FIXED(pad_y + 6)) {
                        if (spd_y < TO_FIXED(0))
                            spd_y -= (TO_FIXED(1) + 0x4);
                        else if (spd_y > TO_FIXED(0))
                            spd_y = TO_FIXED(-1);
                        spd_x = spd_x * -1 + 0x1;
                        score++;
                    }
                    /* Hits near the center */
                    else if (pos_y >= TO_FIXED(pad_y + 6) && pos_y < TO_FIXED(pad_y + 10)) {
                        spd_x = spd_x * -1 + 0x1;
                        score++;
                    }
                    /* Hits near the bottom */
                    else if (pos_y >= TO_FIXED(pad_y + 10) && pos_y < TO_FIXED(pad_y + 20)) {
                        if (spd_y > TO_FIXED(0))
                            spd_y += (TO_FIXED(1) + 0x4);
                        else if (spd_y < TO_FIXED(0))
                            spd_y = TO_FIXED(1);
                        spd_x = spd_x * -1 + 0x1;
                        score++;
                    }
                }
                /* It reached the back wall. Game over :( */
                else if (pos_x <= TO_FIXED(0))
                    ball_launched = FALSE;
            }
            /* Right wall collision */
            else if (pos_x >= TO_FIXED(160)) {
                INT16 temp_pos_x = pos_x - TO_FIXED(160);
                pos_x = TO_FIXED(160) - temp_pos_x;
                spd_x *= -1;
            }
            /* Top wall collision */
            if (pos_y <= TO_FIXED(24)) {
                INT16 temp_pos_y = pos_y - TO_FIXED(24);
                pos_y = TO_FIXED(24) - temp_pos_y;
                spd_y *= -1;
            }
            /* Bottom wall collision */
            else if (pos_y >= TO_FIXED(152)) {
                INT16 temp_pos_y = pos_y - TO_FIXED(152);
                pos_y = TO_FIXED(152) - temp_pos_y;
                spd_y *= -1;
            }
        }
        /* Waiting for game to start */
        else {
            /* Lock ball to center of the paddle */
            pos_x = TO_FIXED(16);
            pos_y = TO_FIXED(pad_y + 8);

            if (joypads.joy0 & J_A) {
                if (!ball_launched) {
                    score = 0;
                    spd_x = TO_FIXED(2);
                    spd_y = TO_FIXED(framecount % 2 ? 1 : -1); /* Send ball either up or down depending on the framecount at launch */
                    ball_launched = TRUE;
                    cls();
                }
            }
        }

        /* Rendering */
        /* Text drawing. Had some issues drawing above 5 digits, but I think that's an issue with the SDK. Need to test more */
        gotoxy(0, 0);
        printf("RACKETBALL %u", score);
        draw_pad(8, pad_y);

        move_sprite(0, TO_INT(pos_x), TO_INT(pos_y));

        framecount++;

        /* Done. Wait for VBLank */
        wait_vbl_done();
    }
}
