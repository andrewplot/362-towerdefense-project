// #include <stdio.h>
// #include <stdbool.h>
// #include "pico/stdlib.h"
// #include "hardware/gpio.h"
// #include "rfid_reader_uart.h"

// // IRQ pin configuration
// #define PN532_IRQ_PIN 2  // Connect PN532 IRQ to GPIO 2

// // Flag set by interrupt
// volatile bool tag_detected_flag = false;

// // GPIO interrupt callback
// void pn532_irq_callback(uint gpio, uint32_t events) {
//     if (gpio == PN532_IRQ_PIN && (events & GPIO_IRQ_EDGE_FALL)) {
//         // IRQ pin went LOW - tag detected or data ready
//         tag_detected_flag = true;
//     }
// }

// int main() {
//     stdio_init_all();
//     sleep_ms(3000);
    
//     printf("\r\n");
//     printf("=====================================\r\n");
//     printf("   PN532 NFC Reader (INTERRUPT MODE) \r\n");
//     printf("         RP2350 Proton Board         \r\n");
//     printf("=====================================\r\n");
//     printf("\r\n");
    
//     // Initialize PN532
//     printf("Initializing PN532 over UART...\r\n");
//     pn532_uart_reader_init();
    
//     // Setup IRQ pin
//     printf("Setting up IRQ pin on GPIO %d...\r\n", PN532_IRQ_PIN);
//     gpio_init(PN532_IRQ_PIN);
//     gpio_set_dir(PN532_IRQ_PIN, GPIO_IN);
//     gpio_pull_up(PN532_IRQ_PIN);  // IRQ is active LOW
    
//     // Enable interrupt on falling edge (IRQ goes LOW when tag detected)
//     gpio_set_irq_enabled_with_callback(PN532_IRQ_PIN, 
//                                        GPIO_IRQ_EDGE_FALL, 
//                                        true, 
//                                        &pn532_irq_callback);
    
//     printf("IRQ configured. Waiting for tags...\r\n");
//     printf("MCU will sleep until tag detected!\r\n");
//     printf("-------------------------------------\r\n");
    
//     uint8_t uid[10];
//     uint8_t uid_len;
//     uint8_t last_uid[10];
//     uint8_t last_uid_len = 0;
//     bool tag_present = false;
    
//     // Send initial tag detection command to arm the IRQ
//     printf("Arming PN532 for interrupt mode...\r\n");
//     pn532_uart_read_uid(uid, &uid_len);  // This will timeout, but arms IRQ
    
//     while (1) {
//         // Check if interrupt flag was set
//         if (tag_detected_flag) {
//             tag_detected_flag = false;  // Clear flag
            
//             printf("IRQ triggered! Reading tag...\r\n");
            
//             // Try to read the tag
//             if (pn532_uart_read_uid(uid, &uid_len)) {
//                 // Check if this is a new tag
//                 bool new_tag = false;
//                 if (!tag_present || uid_len != last_uid_len) {
//                     new_tag = true;
//                 } else {
//                     for (uint8_t i = 0; i < uid_len; i++) {
//                         if (uid[i] != last_uid[i]) {
//                             new_tag = true;
//                             break;
//                         }
//                     }
//                 }
                
//                 if (new_tag) {
//                     printf("TAG DETECTED - UID: ");
//                     for (uint8_t i = 0; i < uid_len; i++) {
//                         if (i > 0) printf(":");
//                         printf("%02X", uid[i]);
//                     }
//                     printf(" (Length: %d bytes)\r\n", uid_len);
                    
//                     // Save UID
//                     last_uid_len = uid_len;
//                     for (uint8_t i = 0; i < uid_len; i++) {
//                         last_uid[i] = uid[i];
//                     }
//                 }
//                 tag_present = true;
                
//             } else {
//                 if (tag_present) {
//                     printf("Tag removed\r\n");
//                     tag_present = false;
//                     last_uid_len = 0;
//                 }
//             }
            
//             // Re-arm for next detection
//             // (Send another detection command)
//             pn532_uart_read_uid(uid, &uid_len);
//         }
        
//         // Sleep - CPU is idle until interrupt!
//         // This saves A LOT of power compared to polling
//         sleep_ms(100);  // Check flag periodically
        
//         // Could also use WFI (Wait For Interrupt) for even lower power:
//         // __wfi();
//     }
// }


#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "rfid_reader_uart.h"

int main() {
    stdio_init_all();
    
    // Give time for USB serial to connect
    sleep_ms(3000);
    
    printf("\r\n");
    printf("=====================================\r\n");
    printf("   PN532 NFC/RFID Reader Test (UART) \r\n");
    printf("         RP2350 Proton Board         \r\n");
    printf("=====================================\r\n");
    printf("\r\n");
    
    printf("Initializing PN532 over UART...\r\n");
    printf("UART Configuration:\r\n");
    printf(" - TX: GPIO 0 (to PN532 RX)\r\n");
    printf(" - RX: GPIO 1 (from PN532 TX)\r\n");
    printf(" - Baud: 115200\r\n");
    printf(" - Module switches: Both OFF\r\n");
    printf("\r\n");
    
    pn532_uart_reader_init();
    
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
        if (pn532_uart_read_uid(uid, &uid_len)) {
            // ALWAYS print the UID every time we read it
            printf("TAG - UID: ");
            for (uint8_t i = 0; i < uid_len; i++) {
                if (i > 0) printf(":");
                printf("%02X", uid[i]);
            }
            printf(" (Length: %d bytes)\r\n", uid_len);
            
            tag_present = true;
        } else {
            if (tag_present) {
                printf("Tag removed\r\n");
                tag_present = false;
            }
        }
        
        sleep_ms(250);  // Poll 4 times per second
    }
}