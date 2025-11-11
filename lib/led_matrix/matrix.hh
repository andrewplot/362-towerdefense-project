#ifndef MATRIX_H
#define MATRIX_H

#include "../tower/tower.hh"
#include "color.hh"


/*  NOTES:

    Row select; DCBA (meaning D = 8, C = 4, etc.)
        - This means 1011 = Row 11, 1001 = Row 9, etc 

*/

/**
 * @brief initializes matrix pins and buffer to have grass
 */
void init_matrix();

/**
 * @brief infinitely renders frames
 */
void render_frame();

/**
 * @brief adds all towers to framebuffer at repective (x, y)
 * 
 * @param towers pointer to first element of Tower array
 */
void set_towers(Tower* towers);

/**
 * @brief adds single tower to framebuffer at respective (x, y)
 * 
 * @param towers pointer to first element of Tower array
 */
void set_tower(Tower tower);

/**
 * @brief adds predefined path to framebuffer
 * 
 * @param towers pointer to first element of Tower array
 */
void set_path();

/**
 * @brief adds tree to framebuffer at pos (x, y)
 * 
 * @param x top left x value of tree
 * @param y top left y value of tree
 */
void set_tree(int x, int y);

/**
 * @brief set pixel to 'color' at pos (x, y)
 * 
 * @param x top left x value of tree
 * @param y top left y value of tree
 * @param color color to set the pixel to
 */
void set_pixel(int x, int y, Color color);

#endif // MATRIX_H