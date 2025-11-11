#ifndef RFID_READER_H
#define RFID_READER_H

#include <stdint.h>
#include <stdbool.h>

// Call once at startup: sets up I2C and PN532 (SAMConfig etc.)
void pn532_i2c_init(void);

// Try to read a tag UID.
// Returns true if a tag was detected and uid/uid_len are filled.
bool pn532_read_uid(uint8_t *uid, uint8_t *uid_len);

#endif // RFID_READER_H
