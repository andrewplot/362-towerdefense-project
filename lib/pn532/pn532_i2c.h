// pn532_i2c.h - I2C interface implementation for PN532
#ifndef __PN532_I2C_H__
#define __PN532_I2C_H__

#include "pn532_interface.h"
#include "hardware/i2c.h"

// Default I2C address (0x48 >> 1 = 0x24)
#define PN532_I2C_ADDRESS       (0x24)

typedef struct {
    pn532_interface_t interface;  // Must be first member
    i2c_inst_t *i2c;
    uint8_t address;
    uint8_t command;  // Last command sent
} pn532_i2c_t;

// Initialize I2C interface for PN532
void pn532_i2c_init(pn532_i2c_t *iface, i2c_inst_t *i2c, uint8_t address);

// Get the base interface pointer
pn532_interface_t* pn532_i2c_get_interface(pn532_i2c_t *iface);

#endif // __PN532_I2C_H__