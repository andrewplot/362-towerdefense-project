#include <stdbool.h>
#include <stdint.h>
#include "hardware/spi.h"

void init_chardisp_pins(void);
void cd_init(void);

void send_spi_cmd(spi_inst_t* spi, uint16_t value);
void send_spi_data(spi_inst_t* spi, uint16_t value);

void cd_write_line(uint8_t row, const char *s);
void print_message(const char *lines[2]);