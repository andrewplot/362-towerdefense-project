#include <stdio.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "hardware/adc.h"

#include "joystick.h"
#include "rfid_reader.h"

int main(void) {
    stdio_init_all();
    sleep_ms(2000);  // let USB serial come up

    // Init your peripherals
    joystick_init();     // your existing joystick setup (ADC + button)
    pn532_i2c_init();    // sets up I2C + PN532 (prints status to serial)

    while (1) {
        // --- Joystick readings ---
        // assumes:
        //  - adc_select_input(0) -> X axis
        //  - adc_select_input(1) -> Y axis
        // adjust if your joystick_init maps differently

        adc_select_input(0);
        uint16_t raw_x = 4095 - adc_read();   // invert X if you prefer

        adc_select_input(1);
        uint16_t raw_y = adc_read();

        bool sel = joystick_select();         // your joystick button helper

        printf("JOY rawX=%u rawY=%u SEL=%d\r\n", raw_x, raw_y, sel);

        // --- RFID / NFC tag (PN532) ---
        uint8_t uid[10];
        uint8_t uid_len;

        if (pn532_read_uid(uid, &uid_len)) {
            printf("TAG UID (%u bytes): ", uid_len);
            for (uint8_t i = 0; i < uid_len; i++) {
                printf("%02X", uid[i]);
                if (i < uid_len - 1) {
                    printf(":");
                }
            }
            printf("\r\n");
        }

        sleep_ms(200);
    }
}
