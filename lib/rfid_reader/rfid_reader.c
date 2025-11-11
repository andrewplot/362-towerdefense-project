#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "rfid_reader.h"
#include "pn532.h"

static pn532_t pn532;
static bool pn532_ready = false;

// Fixed i2c scan with timeout
static void i2c_scan_debug(void) {
    printf("I2C scan on i2c0:\r\n");
    bool found_any = false;
    for (uint8_t addr = 1; addr < 127; addr++) {
        // Use write with timeout instead of blocking read
        uint8_t dummy = 0;
        int ret = i2c_write_timeout_us(i2c0, addr, &dummy, 1, false, 1000); // 1ms timeout
        if (ret >= 0) {
            printf(" - Found device at 0x%02X\r\n", addr);
            found_any = true;
        }
    }
    if (!found_any) {
        printf(" - No devices found\r\n");
    }
}

void pn532_i2c_init(void) {
    const uint SDA_PIN = 4;
    const uint SCL_PIN = 5;
    const uint SPEED_HZ = 100000;  // 100kHz is standard for PN532

    printf("pn532_i2c_init: begin\r\n");

    // Initialize I2C
    i2c_init(i2c0, SPEED_HZ);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    // Give PN532 time to boot up
    printf("Waiting for PN532 to boot...\r\n");
    sleep_ms(1000);  // Increased boot time

    // Scan for devices
    i2c_scan_debug();

    // Initialize PN532 structure - note: no const qualifier
    pn532_init(&pn532, i2c0, PN532_DEFAULT_I2C_ADDR);

    // Try multiple times to get firmware version
    printf("Attempting to get firmware version...\r\n");
    
    uint32_t ver = 0;
    int attempts = 0;
    const int max_attempts = 3;
    
    while (attempts < max_attempts && ver == 0) {
        attempts++;
        printf("Attempt %d of %d...\r\n", attempts, max_attempts);
        
        // Add a small delay between attempts
        if (attempts > 1) {
            sleep_ms(500);
        }
        
        ver = pn532_get_firmware_version(&pn532);
        
        if (ver != 0) {
            printf("Success on attempt %d!\r\n", attempts);
        }
    }
    
    // Fixed format specifier for uint32_t
    printf("Firmware version response: 0x%08lX\r\n", (unsigned long)ver);
    
    if (ver == 0) {
        printf("PN532: not found after %d attempts (check wiring, I2C mode, address 0x%02X)\r\n",
               max_attempts, PN532_DEFAULT_I2C_ADDR);
        printf("\r\nTroubleshooting checklist:\r\n");
        printf("[1] SWITCHES: For Elechouse PN532 V3:\r\n");
        printf("    - Channel 1 = ON (for I2C)\r\n");
        printf("    - Channel 2 = OFF\r\n");
        printf("[2] POWER: Check 3.3V and GND connections\r\n");
        printf("[3] I2C: Check SDA (GP4) and SCL (GP5) connections\r\n");
        printf("[4] Try both switches OFF to see if it appears at 0x24\r\n");
        pn532_ready = false;
        return;
    }
    
    // Extract version info
    uint8_t ic = (ver >> 24) & 0xFF;
    uint8_t ver_major = (ver >> 16) & 0xFF;
    uint8_t ver_minor = (ver >> 8) & 0xFF;
    uint8_t support = ver & 0xFF;
    
    printf("PN532 Found!\r\n");
    printf(" - IC: 0x%02X (should be 0x32 for PN532)\r\n", ic);
    printf(" - Version: %d.%d\r\n", ver_major, ver_minor);
    printf(" - Support: 0x%02X\r\n", support);
    
    if (ic != 0x32) {
        printf("WARNING: IC version mismatch (expected 0x32)\r\n");
    }
    
    // Configure SAM (Secure Access Module)
    printf("Configuring SAM...\r\n");
    if (!pn532_sam_config(&pn532)) {
        printf("SAM configuration failed\r\n");
        pn532_ready = false;
        return;
    }
    
    printf("SAM configured successfully\r\n");
    printf("PN532 initialization complete!\r\n");
    printf("Ready to read tags...\r\n");
    pn532_ready = true;
}

bool pn532_read_uid(uint8_t *uid, uint8_t *uid_len) {
    if (!pn532_ready) {
        static bool warned = false;
        if (!warned) {
            printf("pn532_read_uid: PN532 not ready (init failed)\r\n");
            warned = true;
        }
        return false;
    }
    
    // Use a shorter timeout for tag reading to make it more responsive
    return pn532_read_passive_target(&pn532, uid, uid_len, 200);
}