#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "rfid_reader.h"
#include "pn532.h"

static pn532_t pn532;
static bool pn532_ready = false;


static void i2c_scan_debug(void) {
    printf("I2C scan:\r\n");
    for (uint8_t addr = 1; addr < 127; addr++) {
        // zero-length write: just see if someone ACKs this address
        int ret = i2c_write_blocking(i2c0, addr, NULL, 0, false);
        if (ret >= 0) {
            printf(" - Found device at 0x%02X\r\n", addr);
        }
    }
}


void pn532_i2c_init(void) {
    const uint SDA_PIN = 4;
    const uint SCL_PIN = 5;
    const uint SPEED_HZ = 100000;

    // Setup I2C0 on pins 4/5
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);
    i2c_init(i2c0, SPEED_HZ);

    // Init PN532 struct
    pn532_init(&pn532, i2c0, PN532_DEFAULT_I2C_ADDR);

    sleep_ms(50);

    // Check firmware
    uint32_t ver = pn532_get_firmware_version(&pn532);
    if (ver == 0) {
        printf("PN532: not found (check wiring, address, I2C mode switch)\r\n");
        pn532_ready = false;
        return;
    }

    printf("PN532: found. IC=0x%02X, FW=%u.%u\r\n",
           (unsigned)((ver >> 24) & 0xFF),
           (unsigned)((ver >> 16) & 0xFF),
           (unsigned)((ver >> 8) & 0xFF));

    if (!pn532_sam_config(&pn532)) {
        printf("PN532: SAMConfig failed\r\n");
        pn532_ready = false;
        return;
    }

    printf("PN532: ready (I2C)\r\n");
    pn532_ready = true;
}

bool pn532_read_uid(uint8_t *uid, uint8_t *uid_len) {
    if (!pn532_ready) return false;
    return pn532_read_passive_target(&pn532, uid, uid_len, 200); // 200 ms poll
}
