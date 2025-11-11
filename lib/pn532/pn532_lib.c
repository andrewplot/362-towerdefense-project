// pn532_lib.c - Main PN532 library implementation
#include "pn532_lib.h"
#include "pn532_debug.h"
#include <string.h>
#include <stdio.h>

void pn532_init(pn532_t *pn532, pn532_interface_t *interface) {
    pn532->interface = interface;
    memset(pn532->uid, 0, sizeof(pn532->uid));
    pn532->uid_len = 0;
    pn532->inlisted_tag = 0;
}

void pn532_begin(pn532_t *pn532) {
    pn532->interface->begin(pn532->interface);
    pn532->interface->wakeup(pn532->interface);
}

uint32_t pn532_get_firmware_version(pn532_t *pn532) {
    pn532->packet_buffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;
    
    if (pn532->interface->write_command(pn532->interface, pn532->packet_buffer, 1, NULL, 0) != 0) {
        return 0;
    }
    
    int16_t status = pn532->interface->read_response(pn532->interface, pn532->packet_buffer, sizeof(pn532->packet_buffer), 1000);
    if (status < 0) {
        return 0;
    }
    
    uint32_t response = ((uint32_t)pn532->packet_buffer[0] << 24) |
                       ((uint32_t)pn532->packet_buffer[1] << 16) |
                       ((uint32_t)pn532->packet_buffer[2] << 8)  |
                       ((uint32_t)pn532->packet_buffer[3]);
    
    return response;
}

bool pn532_sam_config(pn532_t *pn532) {
    pn532->packet_buffer[0] = PN532_COMMAND_SAMCONFIGURATION;
    pn532->packet_buffer[1] = 0x01;  // Normal mode
    pn532->packet_buffer[2] = 0x14;  // Timeout 50ms * 20 = 1 second
    pn532->packet_buffer[3] = 0x01;  // Use IRQ pin
    
    DMSG_STR("SAMConfig");
    
    if (pn532->interface->write_command(pn532->interface, pn532->packet_buffer, 4, NULL, 0) != 0) {
        return false;
    }
    
    return (pn532->interface->read_response(pn532->interface, pn532->packet_buffer, sizeof(pn532->packet_buffer), 1000) > 0);
}

bool pn532_set_passive_activation_retries(pn532_t *pn532, uint8_t max_retries) {
    pn532->packet_buffer[0] = PN532_COMMAND_RFCONFIGURATION;
    pn532->packet_buffer[1] = 5;           // Config item 5 (MaxRetries)
    pn532->packet_buffer[2] = 0xFF;        // MxRtyATR (default)
    pn532->packet_buffer[3] = 0x01;        // MxRtyPSL (default)
    pn532->packet_buffer[4] = max_retries; // MxRtyPassiveActivation
    
    if (pn532->interface->write_command(pn532->interface, pn532->packet_buffer, 5, NULL, 0) != 0) {
        return false;
    }
    
    return (pn532->interface->read_response(pn532->interface, pn532->packet_buffer, sizeof(pn532->packet_buffer), 1000) > 0);
}

bool pn532_read_passive_target_id(pn532_t *pn532, uint8_t card_baud_rate, uint8_t *uid, uint8_t *uid_length, uint16_t timeout) {
    pn532->packet_buffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
    pn532->packet_buffer[1] = 1;              // Max 1 card at once
    pn532->packet_buffer[2] = card_baud_rate; // Usually 0x00 for 106 kbps Type A
    
    if (pn532->interface->write_command(pn532->interface, pn532->packet_buffer, 3, NULL, 0) != 0) {
        return false;
    }
    
    int16_t status = pn532->interface->read_response(pn532->interface, pn532->packet_buffer, sizeof(pn532->packet_buffer), timeout);
    if (status < 0) {
        return false;
    }
    
    // Check if any tags were found
    if (pn532->packet_buffer[0] != 1) {
        return false;
    }
    
    uint16_t sens_res = (pn532->packet_buffer[2] << 8) | pn532->packet_buffer[3];
    
    DMSG("ATQA: 0x"); DMSG_HEX(sens_res >> 8); DMSG_HEX(sens_res & 0xFF);
    DMSG("  SAK: 0x"); DMSG_HEX(pn532->packet_buffer[4]);
    DMSG("\r\n");
    
    // Get UID length and copy UID
    *uid_length = pn532->packet_buffer[5];
    
    for (uint8_t i = 0; i < pn532->packet_buffer[5]; i++) {
        uid[i] = pn532->packet_buffer[6 + i];
    }
    
    return true;
}

bool pn532_in_data_exchange(pn532_t *pn532, uint8_t *send, uint8_t send_length, uint8_t *response, uint8_t *response_length) {
    pn532->packet_buffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532->packet_buffer[1] = pn532->inlisted_tag;
    
    if (pn532->interface->write_command(pn532->interface, pn532->packet_buffer, 2, send, send_length) != 0) {
        return false;
    }
    
    int16_t status = pn532->interface->read_response(pn532->interface, response, *response_length, 1000);
    if (status < 0) {
        return false;
    }
    
    if ((response[0] & 0x3F) != 0) {
        DMSG_STR("Status code indicates error");
        return false;
    }
    
    uint8_t length = status - 1;
    if (length > *response_length) {
        length = *response_length;
    }
    
    // Shift response data
    for (uint8_t i = 0; i < length; i++) {
        response[i] = response[i + 1];
    }
    *response_length = length;
    
    return true;
}

// Mifare Classic helper functions
bool pn532_mifareclassic_is_first_block(uint32_t block) {
    if (block < 128) {
        return (block % 4 == 0);
    } else {
        return (block % 16 == 0);
    }
}

bool pn532_mifareclassic_is_trailer_block(uint32_t block) {
    if (block < 128) {
        return ((block + 1) % 4 == 0);
    } else {
        return ((block + 1) % 16 == 0);
    }
}

bool pn532_mifareclassic_authenticate_block(pn532_t *pn532, uint8_t *uid, uint8_t uid_len, uint32_t block_number, uint8_t key_number, uint8_t *key_data) {
    // Store key and UID
    memcpy(pn532->key, key_data, 6);
    memcpy(pn532->uid, uid, uid_len);
    pn532->uid_len = uid_len;
    
    // Prepare authentication command
    pn532->packet_buffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532->packet_buffer[1] = 1;  // Max card numbers
    pn532->packet_buffer[2] = (key_number) ? MIFARE_CMD_AUTH_B : MIFARE_CMD_AUTH_A;
    pn532->packet_buffer[3] = block_number;
    memcpy(pn532->packet_buffer + 4, pn532->key, 6);
    
    for (uint8_t i = 0; i < uid_len; i++) {
        pn532->packet_buffer[10 + i] = pn532->uid[i];
    }
    
    if (pn532->interface->write_command(pn532->interface, pn532->packet_buffer, 10 + uid_len, NULL, 0) != 0) {
        return false;
    }
    
    pn532->interface->read_response(pn532->interface, pn532->packet_buffer, sizeof(pn532->packet_buffer), 1000);
    
    if (pn532->packet_buffer[0] != 0x00) {
        DMSG_STR("Authentication failed");
        return false;
    }
    
    return true;
}

bool pn532_mifareclassic_read_data_block(pn532_t *pn532, uint8_t block_number, uint8_t *data) {
    DMSG("Reading block "); DMSG_INT(block_number); DMSG("\r\n");
    
    pn532->packet_buffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532->packet_buffer[1] = 1;
    pn532->packet_buffer[2] = MIFARE_CMD_READ;
    pn532->packet_buffer[3] = block_number;
    
    if (pn532->interface->write_command(pn532->interface, pn532->packet_buffer, 4, NULL, 0) != 0) {
        return false;
    }
    
    pn532->interface->read_response(pn532->interface, pn532->packet_buffer, sizeof(pn532->packet_buffer), 1000);
    
    if (pn532->packet_buffer[0] != 0x00) {
        return false;
    }
    
    memcpy(data, pn532->packet_buffer + 1, 16);
    return true;
}

bool pn532_mifareclassic_write_data_block(pn532_t *pn532, uint8_t block_number, uint8_t *data) {
    pn532->packet_buffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532->packet_buffer[1] = 1;
    pn532->packet_buffer[2] = MIFARE_CMD_WRITE;
    pn532->packet_buffer[3] = block_number;
    memcpy(pn532->packet_buffer + 4, data, 16);
    
    if (pn532->interface->write_command(pn532->interface, pn532->packet_buffer, 20, NULL, 0) != 0) {
        return false;
    }
    
    return (pn532->interface->read_response(pn532->interface, pn532->packet_buffer, sizeof(pn532->packet_buffer), 1000) > 0);
}

bool pn532_mifareultralight_read_page(pn532_t *pn532, uint8_t page, uint8_t *buffer) {
    if (page >= 64) {
        DMSG_STR("Page out of range");
        return false;
    }
    
    pn532->packet_buffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532->packet_buffer[1] = 1;
    pn532->packet_buffer[2] = MIFARE_CMD_READ;
    pn532->packet_buffer[3] = page;
    
    if (pn532->interface->write_command(pn532->interface, pn532->packet_buffer, 4, NULL, 0) != 0) {
        return false;
    }
    
    pn532->interface->read_response(pn532->interface, pn532->packet_buffer, sizeof(pn532->packet_buffer), 1000);
    
    if (pn532->packet_buffer[0] == 0x00) {
        memcpy(buffer, pn532->packet_buffer + 1, 4);
        return true;
    }
    
    return false;
}

bool pn532_mifareultralight_write_page(pn532_t *pn532, uint8_t page, uint8_t *buffer) {
    pn532->packet_buffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532->packet_buffer[1] = 1;
    pn532->packet_buffer[2] = MIFARE_CMD_WRITE_ULTRALIGHT;
    pn532->packet_buffer[3] = page;
    memcpy(pn532->packet_buffer + 4, buffer, 4);
    
    if (pn532->interface->write_command(pn532->interface, pn532->packet_buffer, 8, NULL, 0) != 0) {
        return false;
    }
    
    return (pn532->interface->read_response(pn532->interface, pn532->packet_buffer, sizeof(pn532->packet_buffer), 1000) > 0);
}

void pn532_print_hex(const uint8_t *data, uint32_t num_bytes) {
    for (uint32_t i = 0; i < num_bytes; i++) {
        printf("%02X", data[i]);
        if (i < num_bytes - 1) {
            printf(":");
        }
    }
    printf("\r\n");
}