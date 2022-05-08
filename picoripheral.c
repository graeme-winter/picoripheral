#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9
#define I2C_FREQ 400000


int main()
{
    stdio_init_all();

    i2c_init(I2C_PORT, I2C_FREQ);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    return 0;
}
