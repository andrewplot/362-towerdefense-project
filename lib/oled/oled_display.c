#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "oled_display.h"
#include "../pin-definitions.h"

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


void send_spi_cmd(spi_inst_t* spi, int value) {
    while (spi_is_busy(spi)) {
        tight_loop_contents();
    }
    spi_get_hw(spi)->dr = value;
}

void send_spi_data(spi_inst_t* spi, int value) {
    int data_value = 0x200 | value;
    while (spi_is_busy(spi)) {
        tight_loop_contents();
    }
    spi_get_hw(spi)->dr = data_value;
}

void cd_write_line(int row, const char *s) {
    int addr_cmd = (row == 0) ? 0x80 : 0xC0;
    send_spi_cmd(spi1, addr_cmd);
    sleep_ms(50);

    for (int i = 0; i < 16; i++) {
        char c = (s && s[i]) ? s[i] : ' '; // checks if character is valid or a null (space)
        send_spi_data(spi1, (int)c);
    }
}

void print_message(const char *lines[2]) {
    cd_write_line(0, lines ? lines[0] : "");
    cd_write_line(1, lines ? lines[1] : "");
}
