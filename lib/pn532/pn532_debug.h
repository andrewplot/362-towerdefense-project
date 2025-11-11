// pn532_interface.h - Abstract interface for PN532 communication
#ifndef __PN532_INTERFACE_H__
#define __PN532_INTERFACE_H__

#include <stdint.h>
#include <stdbool.h>

// PN532 Frame structure constants
#define PN532_PREAMBLE                (0x00)
#define PN532_STARTCODE1              (0x00)
#define PN532_STARTCODE2              (0xFF)
#define PN532_POSTAMBLE               (0x00)

#define PN532_HOSTTOPN532             (0xD4)
#define PN532_PN532TOHOST             (0xD5)

#define PN532_ACK_WAIT_TIME           (10)  // ms, timeout of waiting for ACK

// Error codes
#define PN532_INVALID_ACK             (-1)
#define PN532_TIMEOUT                 (-2)
#define PN532_INVALID_FRAME           (-3)
#define PN532_NO_SPACE                (-4)

typedef struct pn532_interface pn532_interface_t;

struct pn532_interface {
    void (*begin)(pn532_interface_t *self);
    void (*wakeup)(pn532_interface_t *self);
    int8_t (*write_command)(pn532_interface_t *self, const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen);
    int16_t (*read_response)(pn532_interface_t *self, uint8_t *buf, uint8_t len, uint16_t timeout);
    void *context; // For storing implementation-specific data
};

#endif // __PN532_INTERFACE_H__