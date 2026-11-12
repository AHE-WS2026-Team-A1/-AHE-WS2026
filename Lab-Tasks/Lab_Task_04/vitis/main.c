/*
 * Lab 04 - Partial reconfiguration with MicroBlaze (Nexys A7-100T)
 *
 * The static region contains the MicroBlaze, memory, UART and one AXI GPIO:
 *   ch 1 (offset 0x00) = 16 outputs -> din  of the reconfigurable module
 *   ch 2 (offset 0x08) = 16 inputs  <- dout of the reconfigurable module
 *
 * The program does not know which module is loaded. It sends the test value
 * 0x0001 and looks at the answer:  A -> 0x0001, B -> 0xFFFE, C -> 0x0100.
 * Then it sends a counter and prints input and output over the UART.
 */

#include "xil_io.h"      /* Xil_In32, Xil_Out32 */
#include "xil_printf.h"  /* xil_printf over AXI UART Lite (9600 baud) */
#include "sleep.h"       /* usleep */

#define GPIO_BASE 0x40000000u   /* AXI GPIO: CHECK against your Address Editor */
#define DIN       0x00          /* ch 1 data (outputs): to the reconfigurable module   */
#define DOUT      0x08          /* ch 2 data (inputs):  from the reconfigurable module */

/* Find out which module is loaded by sending the test value 0x0001 */
static const char *identify(void)
{
  Xil_Out32(GPIO_BASE + DIN, 0x0001);                /* test value            */
  usleep(100);                                       /* module is registered  */
  u32 r = Xil_In32(GPIO_BASE + DOUT) & 0xFFFF;       /* read module output    */
  if (r == 0x0001) return "RM A (pass-through)";
  if (r == 0xFFFE) return "RM B (invert)";
  if (r == 0x0100) return "RM C (swap bytes)";
  return "unknown";                                  /* e.g. during the swap  */
}

int main(void)
{
  u16 cnt = 0;

  xil_printf("\r\nLab 04: partial reconfiguration demo\r\n");

  while (1) {
    const char *name = identify();                   /* which module?         */
    Xil_Out32(GPIO_BASE + DIN, cnt);                 /* counter -> module     */
    usleep(1000);
    u32 out = Xil_In32(GPIO_BASE + DOUT) & 0xFFFF;   /* module -> CPU (+LEDs) */
    xil_printf("loaded: %s   in=%04x out=%04x\r\n", name, cnt, out);
    cnt++;                                           /* keeps counting across swaps */
    usleep(250000);                                  /* 4 lines per second    */
  }
}
