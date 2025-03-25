set(requires "")
idf_component_register(SRCS ./bluart_e-puck2.c ./button_e-puck2.c ./main_e-puck2.c ./rfcomm_e-puck2.c ./rgb_led_e-puck2.c ./spi_e-puck2.c ./uart_e-puck2.c
                       INCLUDE_DIRS "."
                       REQUIRES ${requires})

