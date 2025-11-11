#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "rfid_reader.h"

int main() {
    stdio_init_all();
    
    // Give time for USB serial to connect (RP2350 needs more time)
    // Just use a simple delay instead of stdio_usb_connected
    sleep_ms(3000);
    
    printf("\r\n");
    printf("=====================================\r\n");
    printf("      PN532 NFC/RFID Reader Test    \r\n");
    printf("           RP2350 Proton Board      \r\n");
    printf("=====================================\r\n");
    printf("\r\n");
    
    printf("Initializing PN532 over I2C...\r\n");
    printf("I2C Configuration:\r\n");
    printf(" - SDA: GPIO 4\r\n");
    printf(" - SCL: GPIO 5\r\n");
    printf(" - Address: 0x24\r\n");
    printf(" - Speed: 100kHz\r\n");
    printf("\r\n");
    
    pn532_i2c_init();
    
    printf("\r\n");
    printf("Starting tag detection loop...\r\n");
    printf("Place an NFC/RFID tag near the reader.\r\n");
    printf("-------------------------------------\r\n");
    
    uint8_t uid[10];
    uint8_t uid_len;
    uint8_t last_uid[10];
    uint8_t last_uid_len = 0;
    bool tag_present = false;
    
    while (1) {
        if (pn532_read_uid(uid, &uid_len)) {
            // Check if this is a new tag or the same one
            bool new_tag = false;
            if (!tag_present || uid_len != last_uid_len) {
                new_tag = true;
            } else {
                for (uint8_t i = 0; i < uid_len; i++) {
                    if (uid[i] != last_uid[i]) {
                        new_tag = true;
                        break;
                    }
                }
            }
            
            if (new_tag) {
                printf("TAG DETECTED - UID: ");
                for (uint8_t i = 0; i < uid_len; i++) {
                    if (i > 0) printf(":");
                    printf("%02X", uid[i]);
                }
                printf(" (Length: %d bytes)\r\n", uid_len);
                
                // Save this UID as the last seen
                last_uid_len = uid_len;
                for (uint8_t i = 0; i < uid_len; i++) {
                    last_uid[i] = uid[i];
                }
            }
            tag_present = true;
        } else {
            if (tag_present) {
                printf("Tag removed\r\n");
                tag_present = false;
                last_uid_len = 0;
            }
        }
        
        sleep_ms(250);  // Poll 4 times per second
    }
}