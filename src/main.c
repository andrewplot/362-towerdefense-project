#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"

#include "joystick.h"
#include "pn532_lib.h"
#include "pn532_i2c.h"

// I2C Configuration
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5
#define I2C_FREQ    100000

// Global PN532 objects
static pn532_i2c_t pn532_i2c;
static pn532_t pn532;

void setup_i2c_and_pn532(void) {
    // Initialize I2C
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
    i2c_init(i2c0, I2C_FREQ);
    
    printf("\r\n=== PN532 NFC/RFID Reader ===\r\n");
    
    // Scan I2C bus
    printf("Scanning I2C bus...\r\n");
    int found = 0;
    for (uint8_t addr = 0x00; addr < 0x80; addr++) {
        uint8_t dummy;
        int ret = i2c_read_blocking(i2c0, addr, &dummy, 1, false);
        if (ret >= 0) {
            printf("  Device found at 0x%02X\r\n", addr);
            found++;
        }
    }
    
    if (found == 0) {
        printf("  No I2C devices found!\r\n");
        printf("  Check wiring and PN532 mode switches\r\n");
        printf("  Mode should be: Channel 1=ON, Channel 2=OFF (I2C mode)\r\n");
        return;
    }
    
    printf("\r\n");
    
    // Initialize PN532 I2C interface
    pn532_i2c_init(&pn532_i2c, i2c0, PN532_I2C_ADDRESS);
    
    // Initialize PN532 library with the I2C interface
    pn532_init(&pn532, pn532_i2c_get_interface(&pn532_i2c));
    pn532_begin(&pn532);
    
    // Check firmware version
    uint32_t ver = pn532_get_firmware_version(&pn532);
    if (ver == 0) {
        printf("PN532 not found!\r\n");
        printf("Possible issues:\r\n");
        printf("  - Wrong I2C address (try 0x24 or 0x48)\r\n");
        printf("  - PN532 not in I2C mode\r\n");
        printf("  - Wiring problem\r\n");
        return;
    }
    
    printf("PN532 found!\r\n");
    printf("  Chip: 0x%02X\r\n", (unsigned)((ver >> 24) & 0xFF));
    printf("  Firmware: v%u.%u\r\n", 
           (unsigned)((ver >> 16) & 0xFF),
           (unsigned)((ver >> 8) & 0xFF));
    
    // Configure PN532
    if (!pn532_sam_config(&pn532)) {
        printf("SAM configuration failed!\r\n");
        return;
    }
    
    printf("PN532 ready for card reading!\r\n\r\n");
}

int main(void) {
    stdio_init_all();
    sleep_ms(2000);  // Wait for USB serial
    
    // Initialize peripherals
    joystick_init();
    setup_i2c_and_pn532();
    
    uint32_t last_card_read = 0;
    
    while (1) {
        // Read joystick
        adc_select_input(0);
        uint16_t raw_x = 4095 - adc_read();
        
        adc_select_input(1);
        uint16_t raw_y = adc_read();
        
        bool sel = joystick_select();
        
        printf("Joystick: X=%4u Y=%4u SEL=%d  ", raw_x, raw_y, sel);
        
        // Try to read NFC/RFID card
        uint8_t uid[7];
        uint8_t uid_len;
        
        bool success = pn532_read_passive_target_id(&pn532, 
                                                     PN532_MIFARE_ISO14443A, 
                                                     uid, 
                                                     &uid_len, 
                                                     100);  // 100ms timeout
        
        if (success) {
            // Prevent spamming when card stays in field
            uint32_t now = to_ms_since_boot(get_absolute_time());
            if (now - last_card_read > 1000) {  // Minimum 1 second between reads
                printf("Card detected! UID (%u bytes): ", uid_len);
                for (uint8_t i = 0; i < uid_len; i++) {
                    printf("%02X", uid[i]);
                    if (i < uid_len - 1) printf(":");
                }
                printf("\r\n");
                last_card_read = now;
            } else {
                printf("[Card still present]\r\n");
            }
        } else {
            printf("[No card]\r\n");
        }
        
        sleep_ms(200);
    }
    
    return 0;
}