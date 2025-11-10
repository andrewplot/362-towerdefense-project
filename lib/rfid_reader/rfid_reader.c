#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"




// In pn532_i2c.c, replace the pn532_i2c_init() function:
#define PN532_SDA_PIN           4    // GPIO 4 has I2C0 SDA (F3 function)
#define PN532_SCL_PIN           1    // GPIO 5 and 1 has I2C0 SCL (F3 function)
#define PN532_I2C_PORT          i2c0


void pn532_i2c_init(void) {
    // Initialize I2C0 at 100kHz
    i2c_init(PN532_I2C_PORT, 100 * 1000);
    
    // Configure GPIO pins for I2C
    gpio_set_function(PN532_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PN532_SCL_PIN, GPIO_FUNC_I2C);
    
    // Enable pull-ups
    gpio_pull_up(PN532_SDA_PIN);
    gpio_pull_up(PN532_SCL_PIN);
    
    printf("PN532 I2C initialized on GPIO%d (SDA) and GPIO%d (SCL)\n", 
           PN532_SDA_PIN, PN532_SCL_PIN);
}

// Replace pn532_reset() function:
void pn532_reset(void) {
    // No hardware reset pin available on 4-pin I2C modules
    // PN532 will auto-wake from I2C communication
    sleep_ms(500);
    printf("PN532 wake-up delay complete\n");
}