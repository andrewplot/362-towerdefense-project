#ifndef PN532_H
#define PN532_H

#include <stdint.h>
#include <stdbool.h>
#include "hardware/i2c.h"

#define PN532_DEFAULT_I2C_ADDR 0x24  // change if your scan shows different

typedef struct {
    const i2c_inst_t *i2c;
    uint8_t addr7;
} pn532_t;

void pn532_init(pn532_t *dev, const i2c_inst_t *i2c, uint8_t addr7);

uint32_t pn532_get_firmware_version(pn532_t *dev);

bool pn532_sam_config(pn532_t *dev);

// Reads one passive ISO14443A target (MIFARE-type tags).
// timeout_ms: how long to wait for a response after issuing the command.
// On success: returns true, fills uid_buf and uid_len.
bool pn532_read_passive_target(pn532_t *dev,
                               uint8_t *uid_buf,
                               uint8_t *uid_len,
                               uint32_t timeout_ms);

#endif // PN532_H
