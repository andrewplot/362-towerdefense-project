/*#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
// #include "hub75.pio.h"


static inline void hub75_rows_init( PIO pio, 
                                    uint state_machine, 
                                    uint offset, 
                                    uint first_row_pin,
                                    uint num_pins, 
                                    uint latch_pin
                                ) {
    printf("Hello");


}

static inline void hub75_rgb_init(  PIO pio, 
                                    uint sm, 
                                    uint offset,
                                    uint first_rgb_pin,
                                    uint clock_pin
                                ) {
    pio_sm_set_consectutive_pindirs(pio, sm, first_rgb_pin, 6, true);
    pio_sm_set_consectutive_pindirs(pio, sm, clock_pin, 1, true);
    for (uint i = first_rgb_pin; i < first_rgb_pin + 6; i++) {
        pio_gpio_init(pio, i);
    }
    pio_gpio_init(pio, clock_pin);

    pio_sm_config c = hub75_rgb_init_program_get_default_config(offset);
    sm_config_set_out_pins(&c, first_rgb_pin, 6);
    sm_config_set_sideset_pins(&c, clock_pin);

    sm_config_set_out_shift(&c, true, true, 24); 
    sm_config_set_in_shift(&c, false, false, 24);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);

    pio_sm_init(pio, sm, offset, &c);
    pio_sm_exec(pio, sm, offset + hub75_rgb_init_offset_entry_point);
    pio_sm_set_enabled(pio, sm, true);
}   
*/