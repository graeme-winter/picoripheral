#include <stdio.h>

#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include "timer.pio.h"
#include "delay.pio.h"

#define TRIGGER 14
#define COUNTER 15
#define CLOCK0 16
#define CLOCK1 17

void timer_no_delay(PIO pio, uint sm, uint pin, uint32_t high, uint32_t low, bool enable);

void timer_delay(PIO pio, uint sm, uint pin, uint32_t delay, uint32_t high, uint32_t low, bool enable);

volatile uint32_t counter;
volatile uint64_t t0, t1;

void __not_in_flash_func(callback)(uint gpio, uint32_t event) {
  if (gpio == COUNTER) {
    if (event == GPIO_IRQ_EDGE_FALL) {
      counter++;
    }
  }
  if (gpio == TRIGGER) {
    if (event == GPIO_IRQ_EDGE_RISE) {
      pio_sm_set_enabled(pio1, 0, true);
      t0 = time_us_64();
      counter = 0;
    } else {
      pio_sm_set_enabled(pio1, 0, false);
      t1 = time_us_64();
      printf("%d %ld\n", counter, t1 - t0);
    }
  }
}

int main() {
  setup_default_uart();

  uint32_t freq = clock_get_hz(clk_sys);
  
  // set up the IRQ
  uint32_t irq_mask = GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL;
  gpio_set_irq_enabled_with_callback(TRIGGER, irq_mask, true, &callback);
  gpio_set_irq_enabled(COUNTER, irq_mask, true);

  // bulk clock pio
  timer_no_delay(pio0, 0, CLOCK1, freq / 2, freq / 2, true);
  // start LED 5 seconds late
  timer_delay(pio0, 1, 25, 5 * freq, freq / 2, freq / 2, true);

  // fast clock pio
  timer_no_delay(pio1, 0, CLOCK0, freq / 20000, 9 * freq / 20000, false);

  while (true) {
    tight_loop_contents();
  }
}

// no-delay timer program
void timer_no_delay(PIO pio, uint sm, uint pin, uint32_t high, uint32_t low, bool enable) {
  // load program, init, push to registers
  uint offset = pio_add_program(pio, &timer_program);
  timer_program_init(pio, sm, offset, pin);

  // intrinsic delays
  high -= 3;
  low -= 3;

  // load low into OSR then copy to ISR
  pio->txf[sm] = low;
  pio_sm_exec(pio, sm, pio_encode_pull(false, false));
  pio_sm_exec(pio, sm, pio_encode_out(pio_isr, 32));

  // load high into OSR
  pio->txf[sm] = high;

  // optionally enable
  pio_sm_set_enabled(pio, sm, enable);
}

// with-delay timer program
void timer_delay(PIO pio, uint sm, uint pin, uint32_t delay, uint32_t high, uint32_t low, bool enable) {
  // load program, init, push to registers
  uint offset = pio_add_program(pio, &delay_program);
  delay_program_init(pio, sm, offset, pin);

  // intrinsic delays - I _think_ the delay on 1st cycle is 2 not 3
  delay -= 2;
  high -= 3;
  low -= 3;

  // load delay into OSR then copy to Y
  pio->txf[sm] = delay;
  pio_sm_exec(pio, sm, pio_encode_pull(false, false));
  pio_sm_exec(pio, sm, pio_encode_out(pio_y, 32));

  // load low into OSR then copy to ISR
  pio->txf[sm] = low;
  pio_sm_exec(pio, sm, pio_encode_pull(false, false));
  pio_sm_exec(pio, sm, pio_encode_out(pio_isr, 32));

  // load high into OSR
  pio->txf[sm] = high;

  // optionally enable
  pio_sm_set_enabled(pio, sm, enable);
}
