#include "pr.h"

// with-delay timer program
void timer(PIO pio, uint sm, uint pin, uint32_t delay, uint32_t high,
           uint32_t low, bool enable) {
  // if delay, load one program else other
  // set clock divider to give ~ 0.2µs / tick (i.e. /= 25)
  delay *= 5;
  high *= 5;
  low *= 5;
  if (delay == 0) {
    programs[sm] = &timer_program;
    offsets[sm] = pio_add_program(pio, &timer_program);
    timer_program_init(pio, sm, offsets[sm], pin, 25);
    // intrinsic delays - I _think_ the delay on 1st cycle is 2 not 3
    delay -= 2;
  } else {
    programs[sm] = &delay_program;
    offsets[sm] = pio_add_program(pio, &delay_program);
    delay_program_init(pio, sm, offsets[sm], pin, 25);
    delay -= 2;
  }

  // intrinsic delays - these are certainly 3 cycles
  high -= 3;
  low -= 3;

  if (delay != 0) {
    // load delay into OSR then copy to Y if we are running delay program
    // otherwise Y register unused
    pio->txf[sm] = delay;
    pio_sm_exec(pio, sm, pio_encode_pull(false, false));
    pio_sm_exec(pio, sm, pio_encode_out(pio_y, 32));
  }

  // load low into OSR then copy to ISR
  pio->txf[sm] = low;
  pio_sm_exec(pio, sm, pio_encode_pull(false, false));
  pio_sm_exec(pio, sm, pio_encode_out(pio_isr, 32));

  // load high into OSR
  pio->txf[sm] = high;

  // optionally enable
  pio_sm_set_enabled(pio, sm, enable);
}
