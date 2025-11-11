// pn532_lib.h - Main PN532 library header
#ifndef __PN532_LIB_H__
#define __PN532_LIB_H__

#include <stdint.h>
#include <stdbool.h>
#include "pn532_interface.h"

// PN532 Commands
#define PN532_COMMAND_GETFIRMWAREVERSION    (0x02)
#define PN532_COMMAND_SAMCONFIGURATION      (0x14)
#define PN532_COMMAND_RFCONFIGURATION       (0x32)
#define PN532_COMMAND_INLISTPASSIVETARGET   (0x4A)
#define PN532_COMMAND_INDATAEXCHANGE        (0x40)
#define PN532_COMMAND_INRELEASE             (0x52)
#define PN532_COMMAND_READREGISTER          (0x06)
#define PN532_COMMAND_WRITEREGISTER         (0x08)
#define PN532_COMMAND_READGPIO              (0x0C)
#define PN532_COMMAND_WRITEGPIO             (0x0E)

// Card types
#define PN532_MIFARE_ISO14443A              (0x00)

// Mifare commands
#define MIFARE_CMD_AUTH_A                   (0x60)
#define MIFARE_CMD_AUTH_B                   (0x61)
#define MIFARE_CMD_READ                     (0x30)
#define MIFARE_CMD_WRITE                    (0xA0)
#define MIFARE_CMD_WRITE_ULTRALIGHT         (0xA2)

typedef struct {
    pn532_interface_t *interface;
    uint8_t packet_buffer[64];
    uint8_t uid[7];
    uint8_t uid_len;
    uint8_t key[6];
    uint8_t inlisted_tag;
} pn532_t;

// Initialization
void pn532_init(pn532_t *pn532, pn532_interface_t *interface);
void pn532_begin(pn532_t *pn532);

// Basic functions
uint32_t pn532_get_firmware_version(pn532_t *pn532);
bool pn532_sam_config(pn532_t *pn532);
bool pn532_set_passive_activation_retries(pn532_t *pn532, uint8_t max_retries);

// ISO14443A functions
bool pn532_read_passive_target_id(pn532_t *pn532, uint8_t card_baud_rate, uint8_t *uid, uint8_t *uid_length, uint16_t timeout);
bool pn532_in_data_exchange(pn532_t *pn532, uint8_t *send, uint8_t send_length, uint8_t *response, uint8_t *response_length);

// Mifare Classic functions
bool pn532_mifareclassic_is_first_block(uint32_t block);
bool pn532_mifareclassic_is_trailer_block(uint32_t block);
bool pn532_mifareclassic_authenticate_block(pn532_t *pn532, uint8_t *uid, uint8_t uid_len, uint32_t block_number, uint8_t key_number, uint8_t *key_data);
bool pn532_mifareclassic_read_data_block(pn532_t *pn532, uint8_t block_number, uint8_t *data);
bool pn532_mifareclassic_write_data_block(pn532_t *pn532, uint8_t block_number, uint8_t *data);

// Mifare Ultralight functions
bool pn532_mifareultralight_read_page(pn532_t *pn532, uint8_t page, uint8_t *buffer);
bool pn532_mifareultralight_write_page(pn532_t *pn532, uint8_t page, uint8_t *buffer);

// Utility functions
void pn532_print_hex(const uint8_t *data, uint32_t num_bytes);

#endif // __PN532_LIB_H__