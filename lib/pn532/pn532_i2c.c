// pn532_i2c.c - I2C interface implementation for PN532
#include "pn532_i2c.h"
#include "pn532_debug.h"
#include "pico/stdlib.h"
#include <string.h>

// Forward declarations of interface functions
static void pn532_i2c_begin(pn532_interface_t *self);
static void pn532_i2c_wakeup(pn532_interface_t *self);
static int8_t pn532_i2c_write_command(pn532_interface_t *self, const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen);
static int16_t pn532_i2c_read_response(pn532_interface_t *self, uint8_t *buf, uint8_t len, uint16_t timeout);

// Helper functions
static int8_t read_ack_frame(pn532_i2c_t *iface);
static int16_t get_response_length(pn532_i2c_t *iface, uint16_t timeout);

void pn532_i2c_init(pn532_i2c_t *iface, i2c_inst_t *i2c, uint8_t address) {
    iface->i2c = i2c;
    iface->address = address;
    iface->command = 0;
    
    // Set up function pointers
    iface->interface.begin = pn532_i2c_begin;
    iface->interface.wakeup = pn532_i2c_wakeup;
    iface->interface.write_command = pn532_i2c_write_command;
    iface->interface.read_response = pn532_i2c_read_response;
    iface->interface.context = iface;
}

pn532_interface_t* pn532_i2c_get_interface(pn532_i2c_t *iface) {
    return &iface->interface;
}

static void pn532_i2c_begin(pn532_interface_t *self) {
    // I2C should already be initialized by the main application
    // Nothing to do here
}

static void pn532_i2c_wakeup(pn532_interface_t *self) {
    sleep_ms(500);  // Wait for PN532 to be ready
}

static int8_t pn532_i2c_write_command(pn532_interface_t *self, const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen) {
    pn532_i2c_t *iface = (pn532_i2c_t*)self->context;
    iface->command = header[0];
    
    // Calculate total frame length
    uint8_t length = hlen + blen + 1;  // TFI + DATA
    
    // Build frame in buffer (max I2C packet is limited)
    uint8_t frame[64];  // Reasonable buffer size
    uint8_t idx = 0;
    
    frame[idx++] = PN532_PREAMBLE;
    frame[idx++] = PN532_STARTCODE1;
    frame[idx++] = PN532_STARTCODE2;
    frame[idx++] = length;
    frame[idx++] = (uint8_t)(~length + 1);  // Length checksum
    frame[idx++] = PN532_HOSTTOPN532;
    
    uint8_t sum = PN532_HOSTTOPN532;
    
    DMSG("write: ");
    
    // Copy header
    for (uint8_t i = 0; i < hlen; i++) {
        if (idx >= sizeof(frame) - 2) {
            DMSG("\r\nFrame too large for buffer\r\n");
            return PN532_INVALID_FRAME;
        }
        frame[idx++] = header[i];
        sum += header[i];
        DMSG_HEX(header[i]);
    }
    
    // Copy body
    for (uint8_t i = 0; i < blen; i++) {
        if (idx >= sizeof(frame) - 2) {
            DMSG("\r\nFrame too large for buffer\r\n");
            return PN532_INVALID_FRAME;
        }
        frame[idx++] = body[i];
        sum += body[i];
        DMSG_HEX(body[i]);
    }
    
    frame[idx++] = (uint8_t)(~sum + 1);  // Data checksum
    frame[idx++] = PN532_POSTAMBLE;
    
    DMSG("\r\n");
    
    // Write frame to I2C
    int ret = i2c_write_blocking(iface->i2c, iface->address, frame, idx, false);
    if (ret != idx) {
        return PN532_INVALID_FRAME;
    }
    
    return read_ack_frame(iface);
}

static int16_t get_response_length(pn532_i2c_t *iface, uint16_t timeout) {
    const uint8_t PN532_NACK[] = {0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00};
    uint8_t buf[6];
    uint32_t start = to_ms_since_boot(get_absolute_time());
    
    // Wait for ready status
    while (1) {
        int ret = i2c_read_blocking(iface->i2c, iface->address, buf, 6, false);
        if (ret == 6) {
            if (buf[0] & 0x01) {  // Ready bit
                break;
            }
        }
        
        sleep_ms(1);
        if (timeout && (to_ms_since_boot(get_absolute_time()) - start) > timeout) {
            return PN532_TIMEOUT;
        }
    }
    
    // Verify frame structure
    if (buf[1] != 0x00 || buf[2] != 0x00 || buf[3] != 0xFF) {
        return PN532_INVALID_FRAME;
    }
    
    uint8_t length = buf[4];
    
    // Send NACK to request full response
    i2c_write_blocking(iface->i2c, iface->address, PN532_NACK, sizeof(PN532_NACK), false);
    
    return length;
}

static int16_t pn532_i2c_read_response(pn532_interface_t *self, uint8_t *buf, uint8_t len, uint16_t timeout) {
    pn532_i2c_t *iface = (pn532_i2c_t*)self->context;
    
    int16_t length = get_response_length(iface, timeout);
    if (length < 0) {
        return length;  // Error code
    }
    
    // Read full response frame
    uint8_t frame[64];
    uint8_t frame_len = 6 + length + 2;  // Status + header + data + checksum + postamble
    
    if (frame_len > sizeof(frame)) {
        return PN532_NO_SPACE;
    }
    
    uint32_t start = to_ms_since_boot(get_absolute_time());
    
    // Wait for data ready
    while (1) {
        int ret = i2c_read_blocking(iface->i2c, iface->address, frame, frame_len, false);
        if (ret == frame_len) {
            if (frame[0] & 0x01) {  // Ready bit
                break;
            }
        }
        
        sleep_ms(1);
        if (timeout && (to_ms_since_boot(get_absolute_time()) - start) > timeout) {
            return PN532_TIMEOUT;
        }
    }
    
    // Verify frame header
    if (frame[1] != 0x00 || frame[2] != 0x00 || frame[3] != 0xFF) {
        return PN532_INVALID_FRAME;
    }
    
    uint8_t frame_length = frame[4];
    uint8_t frame_checksum = frame[5];
    
    if ((uint8_t)(frame_length + frame_checksum) != 0x00) {
        return PN532_INVALID_FRAME;
    }
    
    uint8_t cmd = iface->command + 1;
    if (frame[6] != PN532_PN532TOHOST || frame[7] != cmd) {
        return PN532_INVALID_FRAME;
    }
    
    length = frame_length - 2;  // Subtract TFI and CMD
    if (length > len) {
        return PN532_NO_SPACE;
    }
    
    DMSG("read:  ");
    DMSG_HEX(cmd);
    
    uint8_t sum = PN532_PN532TOHOST + cmd;
    for (uint8_t i = 0; i < length; i++) {
        buf[i] = frame[8 + i];
        sum += buf[i];
        DMSG_HEX(buf[i]);
    }
    DMSG("\r\n");
    
    uint8_t data_checksum = frame[8 + length];
    if ((uint8_t)(sum + data_checksum) != 0x00) {
        DMSG("Checksum error\r\n");
        return PN532_INVALID_FRAME;
    }
    
    return length;
}

static int8_t read_ack_frame(pn532_i2c_t *iface) {
    const uint8_t PN532_ACK[] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
    uint8_t ack_buf[7];  // Status + ACK frame
    
    DMSG("Waiting for ACK...\r\n");
    
    uint32_t start = to_ms_since_boot(get_absolute_time());
    
    while (1) {
        int ret = i2c_read_blocking(iface->i2c, iface->address, ack_buf, sizeof(ack_buf), false);
        if (ret == sizeof(ack_buf)) {
            if (ack_buf[0] & 0x01) {  // Ready bit
                break;
            }
        }
        
        sleep_ms(1);
        if ((to_ms_since_boot(get_absolute_time()) - start) > PN532_ACK_WAIT_TIME) {
            DMSG("ACK timeout\r\n");
            return PN532_TIMEOUT;
        }
    }
    
    if (memcmp(&ack_buf[1], PN532_ACK, sizeof(PN532_ACK)) != 0) {
        DMSG("Invalid ACK\r\n");
        return PN532_INVALID_ACK;
    }
    
    return 0;
}