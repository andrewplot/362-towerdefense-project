#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "oled_display.h"
#include "../pin-definitions.h"

static const uint8_t heart[8] = {
    0b00000,
    0b01010,
    0b11111,
    0b11111,
    0b01110,
    0b00100,
    0b00000,
    0b00000
};


void oled_write_char(uint8_t row, uint8_t col, uint8_t ch) {
    uint8_t addr = (row == 0 ? 0x80 : 0xC0) + col;
    send_spi_cmd(spi1, addr);
    send_spi_data(spi1, ch);
}

void demo_heart() {
    oled_create_char(0, heart); // store heart at slot 0
    oled_write_char(0, 0, 'I');
    oled_write_char(0, 1, ' ');
    oled_write_char(0, 2, 0);   // custom heart char
    oled_write_char(0, 3, ' ');
    oled_write_char(0, 4, 'U');
}

void oled_create_char(uint8_t location, const uint8_t *pattern) {
    location &= 0x07; // valid: 0–7
    send_spi_cmd(spi1, 0x40 | (location << 3)); // set CGRAM address
    for (int i = 0; i < 8; i++) {
        send_spi_data(spi1, pattern[i]);
    }
}

void init_oled_pins() {
    spi_init(spi1, 10000);
    spi_set_format(spi1, 10, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    
    gpio_set_function(OLED_SPI_SCK, GPIO_FUNC_SPI);
    gpio_set_function(OLED_SPI_TX, GPIO_FUNC_SPI);
    gpio_set_function(OLED_SPI_CSn, GPIO_FUNC_SPI);
}

void init_oled() {
    init_oled_pins();
    
    sleep_ms(1);
    send_spi_cmd(spi1, 0x38);
    send_spi_cmd(spi1, 0x0C);
    send_spi_cmd(spi1, 0x01);
    sleep_ms(2);
    send_spi_cmd(spi1, 0x06);
}


void send_spi_cmd(spi_inst_t* spi, uint16_t value) {
    while (spi_is_busy(spi)) {
        tight_loop_contents();
    }
    spi_get_hw(spi)->dr = value;
}

void send_spi_data(spi_inst_t* spi, uint16_t value) {
    uint16_t data_value = 0x200 | value;
    while (spi_is_busy(spi)) {
        tight_loop_contents();
    }
    spi_get_hw(spi)->dr = data_value;
}

void cd_write_line(uint8_t row, const char *s) {
    uint16_t addr_cmd = (row == 0) ? 0x80 : 0xC0;
    send_spi_cmd(spi1, addr_cmd);
    sleep_ms(50);

    for (int i = 0; i < 16; ++i) {
        char c = (s && s[i]) ? s[i] : ' ';
        send_spi_data(spi1, (uint8_t)c);
    }
}

void print_message(const char *lines[2]) {
    cd_write_line(0, lines ? lines[0] : "");
    cd_write_line(1, lines ? lines[1] : "");
}
