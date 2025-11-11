#ifndef MATRIX_H
#define MATRIX_H

/*  NOTES:

    Row select; DCBA (meaning D = 8, C = 4, etc.)
        - This means 1011 = Row 11, 1001 = Row 9, etc 

*/

/**
 * @brief initializes matrix pins and buffer to have grass
 */
void init_matrix();

/**
 * @brief completely renders one frame
 */
void render();

/**
 * @brief completely renders one frame
 * 
 * @param towers pointer to first element of Tower array
 */
void set_towers(Tower* towers);

/**
 * @brief completely renders one frame
 * 
 * @param towers pointer to first element of Tower array
 */
void set_path();

/**
 * @brief completely renders one frame
 * 
 * @param x top left x value of tree
 * @param y top left y value of tree
 */
void set_tree(int x, int y);

/**
 * @brief completely renders one frame
 * 
 * @param x top left x value of tree
 * @param y top left y value of tree
 * @param color color to set the pixel to
 */
void set_pixel(int x, int y, Color color);

#endif // MATRIX_H