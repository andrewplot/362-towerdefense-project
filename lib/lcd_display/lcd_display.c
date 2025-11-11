#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "lcd_display.h"
#include "../pin-definitions.h"

void init_chardisp_pins() {
    spi_init(spi1, 10000);
    spi_set_format(spi1, 10, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    
    gpio_set_function(SPI_DISP_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI_DISP_TX, GPIO_FUNC_SPI);
    gpio_set_function(SPI_DISP_CSn, GPIO_FUNC_SPI);
}

void send_spi_cmd(spi_inst_t* spi, uint16_t value) {
    while (spi_is_busy(spi)) {
        tight_loop_contents();
    }
    spi_get_hw(spi)->dr = value;
}

void cd_init() {
    sleep_ms(1);
    send_spi_cmd(spi1, 0x38);
    send_spi_cmd(spi1, 0x0C);
    send_spi_cmd(spi1, 0x01);
    sleep_ms(2);
    send_spi_cmd(spi1, 0x06);
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

void send_spi_data(spi_inst_t* spi, uint16_t value) {
    uint16_t data_value = 0x200 | value;
    while (spi_is_busy(spi)) {
        tight_loop_contents();
    }
    spi_get_hw(spi)->dr = data_value;
}

void print_message(const char *lines[2]) {
    cd_write_line(0, lines ? lines[0] : "");
    cd_write_line(1, lines ? lines[1] : "");
}