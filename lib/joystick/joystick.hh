#ifndef JOYSTICK_HH
#define JOYSTICK_HH

#define ADC_MAX 4095
#define CENTER 2048
#define DEADZONE_PERCENT 50

/**
 * @brief initialize joystick pins and adc
 */

void init_js(void);

/**
 * @brief return 1 if right, -1 if left, 0 if neither
 * 
 */
int sample_js_x(void);

/**
 * @brief return 1 if up, -1 if down, 0 if neither
 * 
 */
int sample_js_y(void);

/**
 * @brief return 1 if pressed, 0 if not 
 */
bool sample_js_select(void);

#endif // JOYSTICK_HH