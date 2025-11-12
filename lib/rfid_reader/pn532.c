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

// Debug flag - set to 1 for verbose output
#define DEBUG_PN532 1

void pn532_init(pn532_t *dev, i2c_inst_t *i2c, uint8_t addr7) {
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

#if DEBUG_PN532
    printf("I2C Write to 0x%02X: ", dev->addr7);
    for (size_t i = 0; i < len + 1 && i < 16; i++) {
        printf("%02X ", buf[i]);
    }
    if (len + 1 > 16) printf("...");
    printf("\r\n");
#endif

    int ret = i2c_write_blocking(dev->i2c, dev->addr7, buf, (int)(len + 1), false);
    
#if DEBUG_PN532
    printf("I2C write result: %d (expected %d)\r\n", ret, (int)(len + 1));
#endif
    
    return ret == (int)(len + 1);
}

// Try different methods to wake up the PN532
static void pn532_wakeup_extended(pn532_t *dev) {
    printf("Attempting extended wake-up sequence...\r\n");
    
    // Method 1: Send dummy byte
    uint8_t dummy = 0x55;
    i2c_write_blocking(dev->i2c, dev->addr7, &dummy, 1, false);
    sleep_ms(10);
    
    // Method 2: Send another dummy byte pattern
    dummy = 0x00;
    i2c_write_blocking(dev->i2c, dev->addr7, &dummy, 1, false);
    sleep_ms(10);
    
    // Method 3: Try to read to trigger wake-up
    uint8_t read_dummy;
    i2c_read_blocking(dev->i2c, dev->addr7, &read_dummy, 1, false);
    sleep_ms(10);
    
    // Method 4: Send SAMConfiguration wake sequence (some clones need this)
    uint8_t sam_wake[] = {0x01, 0x00, 0x00, 0xFF, 0x03, 0xFD, 0xD4, 0x14, 0x01, 0x17, 0x00};
    i2c_write_blocking(dev->i2c, dev->addr7, sam_wake, sizeof(sam_wake), false);
    sleep_ms(50);
}

// Wait until PN532 status is READY or timeout
static bool wait_ready(pn532_t *dev, uint32_t timeout_ms) {
    uint8_t status;
    uint32_t start = to_ms_since_boot(get_absolute_time());
    int attempts = 0;
    
    while ((to_ms_since_boot(get_absolute_time()) - start) < timeout_ms) {
        int r = i2c_read_blocking(dev->i2c, dev->addr7, &status, 1, false);
        attempts++;
        
#if DEBUG_PN532
        if (attempts == 1 || (attempts % 10 == 0)) {
            printf("wait_ready: attempt %d, read %d byte(s), status=0x%02X\r\n", 
                   attempts, r, (r == 1) ? status : 0xFF);
        }
#endif
        
        if (r == 1 && status == PN532_I2C_READY) {
#if DEBUG_PN532
            printf("PN532 ready after %d attempts\r\n", attempts);
#endif
            return true;
        }
        
        // If we've been waiting a while, try another wake-up
        if (attempts == 20) {
            printf("Still waiting, trying another wake-up...\r\n");
            uint8_t dummy = 0x55;
            i2c_write_blocking(dev->i2c, dev->addr7, &dummy, 1, false);
        }
        
        sleep_ms(10);  // Increased from 5ms
    }
    
#if DEBUG_PN532
    printf("wait_ready timeout after %lu ms (%d attempts)\r\n", (unsigned long)timeout_ms, attempts);
#endif
    
    return false;
}

// Read and verify ACK frame
static bool read_ack(pn532_t *dev, uint32_t timeout_ms) {
    if (!wait_ready(dev, timeout_ms)) {
#if DEBUG_PN532
        printf("read_ack: wait_ready failed\r\n");
#endif
        return false;
    }
    
    uint8_t buf[7];
    int r = i2c_read_blocking(dev->i2c, dev->addr7, buf, 7, false);
    
#if DEBUG_PN532
    printf("read_ack: read %d bytes: ", r);
    for (int i = 0; i < r; i++) {
        printf("%02X ", buf[i]);
    }
    printf("\r\n");
#endif
    
    if (r != 7) {
#if DEBUG_PN532
        printf("read_ack: expected 7 bytes, got %d\r\n", r);
#endif
        return false;
    }
    
    // buf[0] is status (should be 0x01)
    if (buf[0] != PN532_I2C_READY) {
#if DEBUG_PN532
        printf("read_ack: status not ready (0x%02X)\r\n", buf[0]);
#endif
        return false;
    }
    
    bool ack_ok = memcmp(buf + 1, PN532_ACK_FRAME, 6) == 0;
    
#if DEBUG_PN532
    if (!ack_ok) {
        printf("read_ack: ACK mismatch\r\n");
    } else {
        printf("read_ack: ACK OK\r\n");
    }
#endif
    
    return ack_ok;
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

#if DEBUG_PN532
    printf("send_command: cmd=0x%02X, len=%d\r\n", cmd, params_len);
#endif

    // Try extended wake-up first
    pn532_wakeup_extended(dev);
    
    // Wait until chip is ready to accept command
    if (!wait_ready(dev, 1000)) {
#if DEBUG_PN532
        printf("send_command: initial wait_ready failed\r\n");
#endif
        return false;
    }
    
    return i2c_write_frame(dev, frame, idx);
}

// Alternative send command that doesn't wait for ready first
static bool send_command_no_wait(pn532_t *dev, uint8_t cmd, const uint8_t *params, uint8_t params_len) {
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

#if DEBUG_PN532
    printf("send_command_no_wait: cmd=0x%02X, len=%d\r\n", cmd, params_len);
#endif
    
    return i2c_write_frame(dev, frame, idx);
}

// Read a response frame, strip TFI and CMD+1 and leave payload from response.
static bool read_response(pn532_t *dev,
                          uint8_t expected_cmd,
                          uint8_t *out, uint8_t out_len,
                          uint32_t timeout_ms) {
    if (!wait_ready(dev, timeout_ms)) {
#if DEBUG_PN532
        printf("read_response: wait_ready failed\r\n");
#endif
        return false;
    }

    // Read header: status + preamble + startcode + len + lcs + TFI + rspcmd
    uint8_t hdr[8];
    int r = i2c_read_blocking(dev->i2c, dev->addr7, hdr, 8, false);
    
#if DEBUG_PN532
    printf("read_response: read %d header bytes: ", r);
    for (int i = 0; i < r; i++) {
        printf("%02X ", hdr[i]);
    }
    printf("\r\n");
#endif
    
    if (r != 8) {
#if DEBUG_PN532
        printf("read_response: expected 8 header bytes, got %d\r\n", r);
#endif
        return false;
    }

    uint8_t status = hdr[0];
    if (status != PN532_I2C_READY) {
#if DEBUG_PN532
        printf("read_response: status not ready (0x%02X)\r\n", status);
#endif
        return false;
    }
    
    if (hdr[1] != PN532_PREAMBLE ||
        hdr[2] != PN532_STARTCODE1 ||
        hdr[3] != PN532_STARTCODE2) {
#if DEBUG_PN532
        printf("read_response: invalid preamble/startcode\r\n");
#endif
        return false;
    }

    uint8_t len = hdr[4];
    uint8_t lcs = hdr[5];
    if ((uint8_t)(len + lcs) != 0x00) {
#if DEBUG_PN532
        printf("read_response: LCS error (len=0x%02X, lcs=0x%02X)\r\n", len, lcs);
#endif
        return false;
    }

    uint8_t tfi = hdr[6];
    uint8_t rsp_cmd = hdr[7];

#if DEBUG_PN532
    printf("read_response: TFI=0x%02X, rsp_cmd=0x%02X (expected 0x%02X)\r\n", 
           tfi, rsp_cmd, expected_cmd + 1);
#endif

    if (tfi != PN532_PN532_TO_HOST) {
#if DEBUG_PN532
        printf("read_response: wrong TFI\r\n");
#endif
        return false;
    }
    
    if (rsp_cmd != (expected_cmd + 1)) {
#if DEBUG_PN532
        printf("read_response: wrong response command\r\n");
#endif
        return false;
    }

    // Now len-2 bytes left: (we already read TFI + rsp_cmd), so payload_len = len - 2
    uint8_t payload_len = (len >= 2) ? (len - 2) : 0;
    uint8_t tail[255]; // payload + DCS + postamble
    if (payload_len + 2 > sizeof(tail)) return false;

    r = i2c_read_blocking(dev->i2c, dev->addr7, tail, payload_len + 2, false);
    
#if DEBUG_PN532
    printf("read_response: read %d tail bytes\r\n", r);
#endif
    
    if (r != payload_len + 2) {
#if DEBUG_PN532
        printf("read_response: expected %d tail bytes, got %d\r\n", payload_len + 2, r);
#endif
        return false;
    }

    // Verify checksum
    uint16_t sum = tfi + rsp_cmd;
    for (uint8_t i = 0; i < payload_len; i++) sum += tail[i];
    uint8_t dcs = tail[payload_len];
    if ((uint8_t)(sum + dcs) != 0x00) {
#if DEBUG_PN532
        printf("read_response: DCS error\r\n");
#endif
        return false;
    }

    if (tail[payload_len + 1] != PN532_POSTAMBLE) {
#if DEBUG_PN532
        printf("read_response: missing postamble\r\n");
#endif
        return false;
    }

    if (payload_len > out_len) payload_len = out_len;
    if (payload_len > 0 && out) {
        memcpy(out, tail, payload_len);
    }

#if DEBUG_PN532
    printf("read_response: success, payload_len=%d\r\n", payload_len);
#endif

    return true;
}

// ---- Public helpers ----

uint32_t pn532_get_firmware_version(pn532_t *dev) {
#if DEBUG_PN532
    printf("pn532_get_firmware_version: starting\r\n");
#endif
    
    // Try Method 1: Standard approach with wake-up
    printf("Method 1: Standard with wake-up\r\n");
    if (send_command(dev, PN532_CMD_GETFIRMWAREVERSION, NULL, 0)) {
         sleep_ms(100);
        if (read_ack(dev, 1000)) {
            uint8_t buf[8];
            if (read_response(dev, PN532_CMD_GETFIRMWAREVERSION, buf, sizeof(buf), 1000)) {
                // buf[0..3] = IC, Ver, Rev, Support
                uint32_t v = ((uint32_t)buf[0] << 24) |
                             ((uint32_t)buf[1] << 16) |
                             ((uint32_t)buf[2] << 8)  |
                             ((uint32_t)buf[3]);
                
#if DEBUG_PN532
                printf("Method 1 SUCCESS: IC=0x%02X, Ver=%d, Rev=%d, Support=0x%02X\r\n",
                       buf[0], buf[1], buf[2], buf[3]);
#endif
                return v;
            }
        }
    }
    
    // Try Method 2: Send command without waiting for ready first
    printf("Method 2: Send without initial wait\r\n");
    pn532_wakeup_extended(dev);
    if (send_command_no_wait(dev, PN532_CMD_GETFIRMWAREVERSION, NULL, 0)) {
        sleep_ms(50);  // Give it time to process
        if (read_ack(dev, 1000)) {
            uint8_t buf[8];
            if (read_response(dev, PN532_CMD_GETFIRMWAREVERSION, buf, sizeof(buf), 1000)) {
                uint32_t v = ((uint32_t)buf[0] << 24) |
                             ((uint32_t)buf[1] << 16) |
                             ((uint32_t)buf[2] << 8)  |
                             ((uint32_t)buf[3]);
                
#if DEBUG_PN532
                printf("Method 2 SUCCESS: IC=0x%02X, Ver=%d, Rev=%d, Support=0x%02X\r\n",
                       buf[0], buf[1], buf[2], buf[3]);
#endif
                return v;
            }
        }
    }
    
    // Try Method 3: Direct frame write without command wrapper
    printf("Method 3: Direct frame write\r\n");
    uint8_t direct_frame[] = {0x00, 0x00, 0xFF, 0x02, 0xFE, 0xD4, 0x02, 0x2A, 0x00};
    if (i2c_write_frame(dev, direct_frame, sizeof(direct_frame))) {
        sleep_ms(100);
        if (read_ack(dev, 1000)) {
            uint8_t buf[8];
            if (read_response(dev, PN532_CMD_GETFIRMWAREVERSION, buf, sizeof(buf), 1000)) {
                uint32_t v = ((uint32_t)buf[0] << 24) |
                             ((uint32_t)buf[1] << 16) |
                             ((uint32_t)buf[2] << 8)  |
                             ((uint32_t)buf[3]);
                
#if DEBUG_PN532
                printf("Method 3 SUCCESS: IC=0x%02X, Ver=%d, Rev=%d, Support=0x%02X\r\n",
                       buf[0], buf[1], buf[2], buf[3]);
#endif
                return v;
            }
        }
    }

#if DEBUG_PN532
    printf("All methods failed to get firmware version\r\n");
#endif
    
    return 0;
}

bool pn532_sam_config(pn532_t *dev) {
#if DEBUG_PN532
    printf("pn532_sam_config: starting\r\n");
#endif

    // Normal mode, timeout 0x14, use IRQ
    uint8_t params[3] = {0x01, 0x14, 0x01};
    if (!send_command(dev, PN532_CMD_SAMCONFIGURATION, params, 3)) {
#if DEBUG_PN532
        printf("pn532_sam_config: send_command failed\r\n");
#endif
        return false;
    }
    
    if (!read_ack(dev, 1000)) {
#if DEBUG_PN532
        printf("pn532_sam_config: read_ack failed\r\n");
#endif
        return false;
    }
    
    uint8_t buf[4];
    // We don't need the payload details; just make sure we get a good response
    bool result = read_response(dev, PN532_CMD_SAMCONFIGURATION, buf, sizeof(buf), 1000);
    
#if DEBUG_PN532
    printf("pn532_sam_config: %s\r\n", result ? "success" : "failed");
#endif
    
    return result;
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