#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "rfid_reader_uart.h"
#include "pn532_uart.h"

static pn532_uart_t pn532;
static bool pn532_ready = false;

void pn532_uart_reader_init(void) {
    // RP2350 Proton Board connections for UART
    const uint TX_PIN = 0;  // GPIO 0 = UART0 TX -> PN532 RX
    const uint RX_PIN = 1;  // GPIO 1 = UART0 RX <- PN532 TX
    const uint BAUD_RATE = 115200;
    
    printf("pn532_uart_reader_init: begin\r\n");
    printf("UART Configuration:\r\n");
    printf(" - UART: uart0\r\n");
    printf(" - TX Pin: GPIO %d (to PN532 RX)\r\n", TX_PIN);
    printf(" - RX Pin: GPIO %d (from PN532 TX)\r\n", RX_PIN);
    printf(" - Baud Rate: %d\r\n", BAUD_RATE);
    printf(" - Switch Settings: Both OFF (HSU mode)\r\n");
    printf("\r\n");
    
    // Initialize UART interface
    pn532_uart_init(&pn532, uart0, TX_PIN, RX_PIN, BAUD_RATE);
    
    // Give PN532 time to boot
    printf("Waiting for PN532 to boot...\r\n");
    sleep_ms(500);
    
    // Try to get firmware version
    printf("Attempting to get firmware version...\r\n");
    uint32_t ver = pn532_uart_get_firmware_version(&pn532);
    
    if (ver == 0) {
        printf("PN532: not found via UART\r\n");
        printf("\r\nTroubleshooting checklist:\r\n");
        printf("[1] SWITCHES: For Elechouse PN532 V3:\r\n");
        printf("    - Channel 1 = OFF (for UART/HSU)\r\n");
        printf("    - Channel 2 = OFF (for UART/HSU)\r\n");
        printf("[2] POWER: Check 3.3V and GND connections\r\n");
        printf("[3] UART: Check TX (GP0) and RX (GP1) connections\r\n");
        printf("    - RP2350 TX (GP0) -> PN532 RX\r\n");
        printf("    - RP2350 RX (GP1) -> PN532 TX\r\n");
        printf("[4] WIRING: Don't cross TX/RX - TX goes to RX on other side\r\n");
        pn532_ready = false;
        return;
    }
    
    // Extract version info
    uint8_t ic = (ver >> 24) & 0xFF;
    uint8_t ver_major = (ver >> 16) & 0xFF;
    uint8_t ver_minor = (ver >> 8) & 0xFF;
    uint8_t support = ver & 0xFF;
    
    printf("PN532 Found via UART!\r\n");
    printf(" - IC: 0x%02X (should be 0x32 for PN532)\r\n", ic);
    printf(" - Version: %d.%d\r\n", ver_major, ver_minor);
    printf(" - Support: 0x%02X\r\n", support);
    
    if (ic != 0x32) {
        printf("WARNING: IC version mismatch (expected 0x32)\r\n");
    }
    
    // Configure SAM
    printf("Configuring SAM...\r\n");
    if (!pn532_uart_sam_config(&pn532)) {
        printf("SAM configuration failed\r\n");
        pn532_ready = false;
        return;
    }
    
    printf("SAM configured successfully\r\n");
    printf("PN532 UART initialization complete!\r\n");
    printf("Ready to read tags...\r\n");
    pn532_ready = true;
}

bool pn532_uart_read_uid(uint8_t *uid, uint8_t *uid_len) {
    if (!pn532_ready) {
        static bool warned = false;
        if (!warned) {
            printf("pn532_uart_read_uid: PN532 not ready (init failed)\r\n");
            warned = true;
        }
        return false;
    }
    
    // Use a timeout appropriate for tag reading
    return pn532_uart_read_passive_target(&pn532, uid, uid_len, 200);
}