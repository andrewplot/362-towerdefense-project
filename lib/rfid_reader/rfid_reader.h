// pn532_i2c.h
#ifndef PN532_I2C_H
#define PN532_I2C_H

#include <stdint.h>
#include <stdbool.h>

// PN532 I2C Configuration
#define PN532_I2C_ADDRESS       0x24
#define PN532_SDA_PIN           4
#define PN532_SCL_PIN           5
// No RESET or IRQ pins on 4-pin I2C-only modules

// PN532 Frame Structure
#define PN532_PREAMBLE          0x00
#define PN532_STARTCODE1        0x00
#define PN532_STARTCODE2        0xFF
#define PN532_POSTAMBLE         0x00
#define PN532_HOSTTOPN532       0xD4
#define PN532_PN532TOHOST       0xD5

// PN532 Commands
#define PN532_COMMAND_GETFIRMWAREVERSION    0x02
#define PN532_COMMAND_SAMCONFIGURATION      0x14
#define PN532_COMMAND_INLISTPASSIVETARGET   0x4A
#define PN532_COMMAND_INDATAEXCHANGE        0x40
#define PN532_COMMAND_INRELEASE             0x52

// Timeouts
#define PN532_ACK_WAIT_TIME     1000
#define PN532_DEFAULT_TIMEOUT   1000

// Return codes
#define PN532_TIMEOUT           -1
#define PN532_INVALID_ACK       -2
#define PN532_INVALID_FRAME     -3
#define PN532_NO_SPACE          -4

// Function prototypes
void pn532_i2c_init(void);
void pn532_reset(void);
int8_t pn532_write_command(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen);
int16_t pn532_read_response(uint8_t *buf, uint8_t len, uint16_t timeout);
int8_t pn532_get_firmware_version(uint32_t *version);
int8_t pn532_sam_configuration(void);
int8_t pn532_read_passive_target(uint8_t *uid, uint8_t *uid_len, uint16_t timeout);

#endif // PN532_I2C_H