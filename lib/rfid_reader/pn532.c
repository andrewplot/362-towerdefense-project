#include "pn532.h"

#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"

#define PN532_PREAMBLE      0x00
#define PN532_STARTCODE1    0x00
#define PN532_STARTCODE2    0xFF
#define PN532_POSTAMBLE     0x00

#define PN532_HOST_TO_PN532 0xD4
#define PN532_PN532_TO_HOST 0xD5

// I2C protocol (PN532 as slave)
#define PN532_I2C_CMD       0x01    // host -> PN532
#define PN532_I2C_READY     0x01    // status: ready
#define PN532_I2C_BUSY      0x00    // status: busy

// Commands
#define PN532_CMD_GETFIRMWAREVERSION  0x02
#define PN532_CMD_SAMCONFIGURATION    0x14
#define PN532_CMD_INLISTPASSIVETARGET 0x4A

static const uint8_t PN532_ACK_FRAME[6] = {0x00,0x00,0xFF,0x00,0xFF,0x00};

void pn532_init(pn532_t *dev, const i2c_inst_t *i2c, uint8_t addr7) {
    dev->i2c = i2c;
    dev->addr7 = addr7;
}

static uint8_t calc_len_checksum(uint8_t len) {
    return (uint8_t)(~len + 1);
}

static uint8_t calc_data_checksum(uint8_t tfi, const uint8_t *data, uint8_t len) {
    uint16_t sum = tfi;
    for (uint8_t i = 0; i < len; i++) sum += data[i];
    return (uint8_t)(~sum + 1);
}

static bool i2c_write_frame(pn532_t *dev, const uint8_t *data, size_t len) {
    // Prepend I2C command byte (0x01)
    uint8_t buf[1 + 6 + 255]; // enough for typical frames
    buf[0] = PN532_I2C_CMD;

    memcpy(buf + 1, data, len);

    int ret = i2c_write_blocking(dev->i2c, dev->addr7, buf, (int)(len + 1), false);
    return ret == (int)(len + 1);
}

// Wait until PN532 status is READY or timeout
static bool wait_ready(pn532_t *dev, uint32_t timeout_ms) {
    uint8_t status;
    uint32_t start = to_ms_since_boot(get_absolute_time());
    while ((to_ms_since_boot(get_absolute_time()) - start) < timeout_ms) {
        int r = i2c_read_blocking(dev->i2c, dev->addr7, &status, 1, false);
        if (r == 1 && status == PN532_I2C_READY) {
            return true;
        }
        sleep_ms(5);
    }
    return false;
}

// Read and verify ACK frame
static bool read_ack(pn532_t *dev, uint32_t timeout_ms) {
    if (!wait_ready(dev, timeout_ms)) {
        return false;
    }
    uint8_t buf[7];
    int r = i2c_read_blocking(dev->i2c, dev->addr7, buf, 7, false);
    if (r != 7) return false;
    // buf[0] is status (should be 0x01)
    if (buf[0] != PN532_I2C_READY) return false;
    return memcmp(buf + 1, PN532_ACK_FRAME, 6) == 0;
}

// Send a PN532 command (no response parsing)
static bool send_command(pn532_t *dev, uint8_t cmd, const uint8_t *params, uint8_t params_len) {
    uint8_t len = params_len + 1; // TFI + params
    uint8_t lcs = calc_len_checksum(len);
    uint8_t frame[8 + 255];
    size_t idx = 0;

    frame[idx++] = PN532_PREAMBLE;
    frame[idx++] = PN532_STARTCODE1;
    frame[idx++] = PN532_STARTCODE2;
    frame[idx++] = len;
    frame[idx++] = lcs;
    frame[idx++] = PN532_HOST_TO_PN532;
    frame[idx++] = cmd;
    if (params_len && params) {
        memcpy(frame + idx, params, params_len);
        idx += params_len;
    }
    uint8_t dcs = calc_data_checksum(PN532_HOST_TO_PN532, &frame[6], 1 + params_len);
    frame[idx++] = dcs;
    frame[idx++] = PN532_POSTAMBLE;

    // Wait until chip is ready to accept command
    if (!wait_ready(dev, 1000)) {
        return false;
    }
    return i2c_write_frame(dev, frame, idx);
}

// Read a response frame, strip TFI and CMD+1 and leave payload from response.
static bool read_response(pn532_t *dev,
                          uint8_t expected_cmd,
                          uint8_t *out, uint8_t out_len,
                          uint32_t timeout_ms) {
    if (!wait_ready(dev, timeout_ms)) return false;

    // Read header: status + preamble + startcode + len + lcs + TFI + rspcmd
    uint8_t hdr[8];
    int r = i2c_read_blocking(dev->i2c, dev->addr7, hdr, 8, false);
    if (r != 8) return false;

    uint8_t status = hdr[0];
    if (status != PN532_I2C_READY) return false;
    if (hdr[1] != PN532_PREAMBLE ||
        hdr[2] != PN532_STARTCODE1 ||
        hdr[3] != PN532_STARTCODE2) {
        return false;
    }

    uint8_t len = hdr[4];
    uint8_t lcs = hdr[5];
    if ((uint8_t)(len + lcs) != 0x00) return false;

    uint8_t tfi = hdr[6];
    uint8_t rsp_cmd = hdr[7];

    if (tfi != PN532_PN532_TO_HOST) return false;
    if (rsp_cmd != (expected_cmd + 1)) {
        // Not the response we expected
        return false;
    }

    // Now len-2 bytes left: (we already read TFI + rsp_cmd), so payload_len = len - 2
    uint8_t payload_len = (len >= 2) ? (len - 2) : 0;
    uint8_t tail[255]; // payload + DCS + postamble
    if (payload_len + 2 > sizeof(tail)) return false;

    r = i2c_read_blocking(dev->i2c, dev->addr7, tail, payload_len + 2, false);
    if (r != payload_len + 2) return false;

    // Verify checksum
    uint16_t sum = tfi + rsp_cmd;
    for (uint8_t i = 0; i < payload_len; i++) sum += tail[i];
    uint8_t dcs = tail[payload_len];
    if ((uint8_t)(sum + dcs) != 0x00) return false;

    if (tail[payload_len + 1] != PN532_POSTAMBLE) return false;

    if (payload_len > out_len) payload_len = out_len;
    if (payload_len > 0 && out) {
        memcpy(out, tail, payload_len);
    }

    return true;
}

// ---- Public helpers ----

uint32_t pn532_get_firmware_version(pn532_t *dev) {
    if (!send_command(dev, PN532_CMD_GETFIRMWAREVERSION, NULL, 0)) {
        return 0;
    }
    if (!read_ack(dev, 1000)) {
        return 0;
    }
    uint8_t buf[8];
    if (!read_response(dev, PN532_CMD_GETFIRMWAREVERSION, buf, sizeof(buf), 1000)) {
        return 0;
    }
    // buf[0..3] = IC, Ver, Rev, Support
    uint32_t v = ((uint32_t)buf[0] << 24) |
                 ((uint32_t)buf[1] << 16) |
                 ((uint32_t)buf[2] << 8)  |
                 ((uint32_t)buf[3]);
    return v;
}

bool pn532_sam_config(pn532_t *dev) {
    // Normal mode, timeout 0x14, use IRQ
    uint8_t params[3] = {0x01, 0x14, 0x01};
    if (!send_command(dev, PN532_CMD_SAMCONFIGURATION, params, 3)) return false;
    if (!read_ack(dev, 1000)) return false;
    uint8_t buf[4];
    // We don't need the payload details; just make sure we get a good response
    return read_response(dev, PN532_CMD_SAMCONFIGURATION, buf, sizeof(buf), 1000);
}

bool pn532_read_passive_target(pn532_t *dev,
                               uint8_t *uid_buf,
                               uint8_t *uid_len,
                               uint32_t timeout_ms) {
    // InListPassiveTarget: max 1 target, 106 kbps Type A (0x00)
    uint8_t params[2] = {0x01, 0x00};
    if (!send_command(dev, PN532_CMD_INLISTPASSIVETARGET, params, 2)) return false;
    if (!read_ack(dev, 1000)) return false;

    uint8_t buf[32];
    if (!read_response(dev, PN532_CMD_INLISTPASSIVETARGET, buf, sizeof(buf), timeout_ms)) {
        return false; // timeout or error
    }

    // Expected layout (for Type A):
    // buf[0] = NbTg (number of targets, should be 1)
    // buf[1] = Tg
    // buf[2..] = target data, including SENS_RES, SEL_RES, UID length, UID, etc.
    if (buf[0] < 1) return false;

    // Heuristic: for ISO14443A, Adafruit-style layout:
    // buf[5] = UID length, buf[6..] = UID
    uint8_t length = buf[5];
    if (length == 0 || length > 10) return false;

    if (uid_buf && uid_len) {
        *uid_len = length;
        for (uint8_t i = 0; i < length; i++) {
            uid_buf[i] = buf[6 + i];
        }
    }

    return true;
}
