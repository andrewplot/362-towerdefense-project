#ifndef MATRIX_H
#define MATRIX_H

#include "../color.h"
#include "../pin-definitions.h"

#define MATRIX_ROWS 32
#define MATRIX_COLS 64

/**
 * @brief initializes led matrix pins for proton
 */
void init_matrix_pins();

/**
 * @brief initializes matrix buffer to grass color
 */
void init_framebuffer();

/**
 * @brief initializes matrix pins and buffer to have grass
 */
void init_matrix();

void init_color_test();

void color_test();

/**
 * @brief completely renders one frame
 */
void render();

/**
 * @brief resets row select pins to row 0
 */
void reset_row_sel();

/**
 * @brief pulses pin high and then back to low
 * @param pin the pin to be pulsed
 * @param us the amount of microseconds to be kept high
 */
void pulse_pin(int pin, int us);

/**
 * @brief sets the color select pins for both top 
 *        and bottom half of the matrix
 * @param row the top half row that is selected
 * @param col the col that is selected
 * @param plane the bitplane that is currently being cycled
 */
void set_rgb_pins(int row, int col, int plane);

/**
 * @brief sets the row select pins to the corresponding row
 * @param row the top half row that is selected
 */
void set_row_pins(int row);

#endif // MATRIX_H