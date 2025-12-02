// #include <stdio.h>
// #include <stdbool.h>
// #include "pico/stdlib.h"
// #include "rfid_reader_uart.h"

// int main() {
//     stdio_init_all();
    
//     // Give time for USB serial to connect
//     sleep_ms(3000);
    
//     printf("\r\n");
//     printf("=====================================\r\n");qq
//     printf("   PN532 NFC/RFID Reader Test (UART) \r\n");
//     printf("         RP2350 Proton Board         \r\n");
//     printf("=====================================\r\n");
//     printf("\r\n");
    
//     printf("Initializing PN532 over UART...\r\n");
//     printf("UART Configuration:\r\n");
//     printf(" - TX: GPIO 32 (to PN532 RX)\r\n");
//     printf(" - RX: GPIO 33 (from PN532 TX)\r\n");
//     printf(" - Baud: 115200\r\n");
//     printf(" - Module switches: Both OFF\r\n");
//     printf("\r\n");
    
//     pn532_uart_reader_init();
    
//     printf("\r\n");
//     printf("Starting tag detection loop...\r\n");
//     printf("Place an NFC/RFID tag near the reader.\r\n");
//     printf("-------------------------------------\r\n");
    
//     uint8_t uid[10];
//     uint8_t uid_len;
//     uint8_t last_uid[10];
//     uint8_t last_uid_len = 0;
//     bool tag_present = false;
    
//     while (1) {
//         if (pn532_uart_read_uid(uid, &uid_len)) {
//             // ALWAYS print the UID every time we read it
//             printf("TAG - UID: ");
//             for (uint8_t i = 0; i < uid_len; i++) {
//                 if (i > 0) printf(":");
//                 printf("%02X", uid[i]);
//             }
//             printf(" (Length: %d bytes)\r\n", uid_len);
            
//             tag_present = true;
//         } else {
//             if (tag_present) {
//                 printf("Tag removed\r\n");
//                 tag_present = false;
//             }
//         }
        
//         sleep_ms(1000);  // Poll 4 times per second
//     }
// }

#include <stdio.h>
#include "pico/stdlib.h"
#include "buzzer_pwm.h"

// Buzzer GPIO pin configuration
#define BUZZER_PIN 15  // Change this to whatever pin your buzzer is connected to

int main() {
    // Initialize USB serial
    stdio_init_all();
    
    // Wait for USB serial connection
    sleep_ms(2000);
    
    printf("\r\n");
    printf("========================================\r\n");
    printf("    PWM Buzzer Control Test Program    \r\n");
    printf("           RP2350 Proton Board          \r\n");
    printf("========================================\r\n");
    printf("\r\n");
    
    // Initialize PWM buzzer
    printf("Initializing PWM buzzer on GPIO %d...\r\n", BUZZER_PIN);
    buzzer_pwm_init(BUZZER_PIN);
    buzzer_set_volume(100);  // Set to maximum volume
    printf("PWM Buzzer initialized!\r\n\r\n");
    
    // Instructions
    printf("Commands:\r\n");
    printf("Sound Effects:\r\n");
    printf("  1 - Sound Effect 1 (Mario-style melody)\r\n");
    printf("  2 - Sound Effect 2 (Quick two-tone)\r\n");
    printf("  3 - Sound Effect 3 (Error sound)\r\n");
    printf("\r\n");
    printf("Volume:\r\n");
    printf("  + - Increase volume\r\n");
    printf("  - - Decrease volume\r\n");
    printf("\r\n");
    printf("Control:\r\n");
    printf("  o - Turn ON continuous tone (1kHz)\r\n");
    printf("  f - Turn OFF (stop)\r\n");
    printf("  h - Show help\r\n");
    printf("========================================\r\n\r\n");
    
    printf("Ready! Press a key...\r\n");
    
    uint8_t volume = 90;  // Current volume level
    
    while (1) {
        // Check if a character is available
        int c = getchar_timeout_us(0);  // Non-blocking read
        
        if (c != PICO_ERROR_TIMEOUT) {
            char input = (char)c;
            
            switch (input) {
                // Sound effects
                case '1':
                    printf("Playing Victory Sound...\r\n");
                    buzzer_sound_effect_1();
                    printf("Done!\r\n");
                    break;
                    
                case '2':
                    printf("Playing Balloon Pop Sound...\r\n");
                    buzzer_sound_effect_2();
                    printf("Done!\r\n");
                    break;
                    
                case '3':
                    printf("Playing Error Sound...\r\n");
                    buzzer_sound_effect_3();
                    printf("Done!\r\n");
                    break;

                case '4':
                    printf("Playing Ligma Sound...\r\n");
                    buzzer_sound_effect_4();
                    printf("Done!\r\n");
                    break;

                case '5':
                    printf("Playing Wave Start Sound...\r\n");
                    buzzer_sound_effect_5();
                    printf("Done!\r\n");
                    break;
                // Volume control
                case '+':
                case '=':
                    volume += 10;
                    if (volume > 100) volume = 100;
                    buzzer_set_volume(volume);
                    printf("Volume: %d%%\r\n", volume);
                    buzzer_beep(FREQ_MEDIUM, 100);
                    break;
                    
                case '-':
                case '_':
                    if (volume >= 10) volume -= 10;
                    buzzer_set_volume(volume);
                    printf("Volume: %d%%\r\n", volume);
                    buzzer_beep(FREQ_MEDIUM, 100);
                    break;
                
                // Control
                case 'o':
                case 'O':
                    printf("Buzzer ON (1000 Hz continuous, press 'f' to stop)\r\n");
                    buzzer_play_tone(FREQ_MEDIUM, 0);  // 0 = continuous
                    break;
                    
                case 'f':
                case 'F':
                    printf("Buzzer OFF\r\n");
                    buzzer_stop();
                    break;
                    
                case 'h':
                case 'H':
                    printf("\r\nCommands:\r\n");
                    printf("  1: Sound Effect 1\r\n");
                    printf("  2: Sound Effect 2\r\n");
                    printf("  3: Sound Effect 3\r\n");
                    printf("  +/-: Volume control\r\n");
                    printf("  o: ON, f: OFF\r\n\r\n");
                    break;
                    
                case '\r':
                case '\n':
                    // Ignore newlines
                    break;
                    
                default:
                    printf("Unknown: '%c' (press 'h' for help)\r\n", input);
                    break;
            }
        }
        
        // Small delay
        sleep_ms(10);
    }
    
    return 0;
}