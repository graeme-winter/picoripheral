#include <stdio.h>

#include "hardware/adc.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include "picoripheral.pio.h"

#define TRIGGER 16
#define COUNTER 17
#define ADC0 26

void picoripheral_pin_forever(PIO pio, uint sm, uint offset, uint pin,
                              uint freq, bool enable);

volatile uint32_t counter;
uint16_t data[20000];

void __not_in_flash_func(callback)(uint gpio, uint32_t event) {
  if (gpio == COUNTER) {
    if (event == GPIO_IRQ_EDGE_FALL) {
      // N.B. this is a 12 bit read
      data[counter] = adc_read();
      counter++;
    }
  }
  if (gpio == TRIGGER) {
    if (event == GPIO_IRQ_EDGE_RISE) {
      pio_sm_set_enabled(pio1, 0, true);
      counter = 0;
    } else {
      pio_sm_set_enabled(pio1, 0, false);
      // compute mean: should not be in IRQ handler ... should update
      // pointer and reset buffer, or stream out over i2c or similar
      uint32_t total = 0;
      for (uint32_t j = 0; j < counter; j++) total += data[j];
      printf("%d %d\n", counter, total / counter);
    }
  }
}

// FIXME give an API to return this amongst other things
// clock_get_hz(clk_sys)

int main() {
  setup_default_uart();
  adc_init();

  // set up ADC
  adc_gpio_init(ADC0);
  adc_select_input(0);

  // set up the IRQ
  uint32_t irq_mask = GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL;
  gpio_set_irq_enabled_with_callback(TRIGGER, irq_mask, true, &callback);
  gpio_set_irq_enabled(COUNTER, irq_mask, true);

  // bulk clock pio
  uint off0 = pio_add_program(pio0, &picoripheral_program);
  picoripheral_pin_forever(pio0, 0, off0, 15, 1, true);
  picoripheral_pin_forever(pio0, 1, off0, 25, 1, true);

  // fast clock pio
  uint off1 = pio_add_program(pio1, &picoripheral_program);
  picoripheral_pin_forever(pio1, 0, off1, 14, 20000, false);



  while (true)
    ;
}

void picoripheral_pin_forever(PIO pio, uint sm, uint offset, uint pin,
                              uint freq, bool enable) {
  picoripheral_program_init(pio, sm, offset, pin);
  pio_sm_set_enabled(pio, sm, enable);

  pio->txf[sm] = (clock_get_hz(clk_sys) / (2 * freq)) - 3;
}
