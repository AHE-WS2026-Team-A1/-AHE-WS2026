/*
 * Lab 03 - GPIO, 7-segment display and PWM IP on MicroBlaze (Nexys A7-100T)
 *
 * axi_gpio_0 @ 0x40000000 : ch1 = 16 switches (in),  ch2 = 16 LEDs (out)
 * axi_gpio_1 @ 0x40010000 : ch1 = segments CA..CG,DP, ch2 = digit enables AN0..AN7
 *                           (both active low: 0 = on)
 * pwm_axi_0  @ 0x44A00000 : own IP from Lab 01, drives RGB LED LD16
 *
 * Behaviour:
 *   - LEDs mirror the 16 switches
 *   - SW3..0 = red, SW7..4 = green, SW11..8 = blue level (0..15 -> 0..255)
 *   - 7-segment display shows the switch value in hexadecimal
 */

#include "xil_io.h"        /* Xil_In32, Xil_Out32 */
#include "xparameters.h"   /* base addresses from the XSA */
#include "sleep.h"         /* usleep */
#include "xil_printf.h"    /* xil_printf over UART Lite (9600 baud) */

/* ---------------- Base addresses (Address Editor) ---------------------- */
#define GPIO0_BASE  0x40000000u   /* switches + LEDs        */
#define GPIO1_BASE  0x40010000u   /* 7-segment display      */
#define PWM_BASE    0x44A00000u   /* pwm_axi (Lab 01 IP)    */

/* ---------------- AXI GPIO register offsets (PG144) -------------------- */
#define GPIO_SW     0x00   /* GPIO0 ch1 data : switches (read)       */
#define GPIO_LED    0x08   /* GPIO0 ch2 data : LEDs (write)          */
#define SEG_REG     0x00   /* GPIO1 ch1 data : segments CA..CG, DP   */
#define SEG_TRI     0x04   /* GPIO1 ch1 direction (0 = output)       */
#define AN_REG      0x08   /* GPIO1 ch2 data : digit enables AN0..AN7*/
#define AN_TRI      0x0C   /* GPIO1 ch2 direction (0 = output)       */

/* ---------------- pwm_axi register map (Lab 01) ------------------------ */
#define PWM_CTRL    0x00   /* bit 0 = enable for all three channels  */
#define PWM_R       0x04   /* bits 7..0 = red duty   (0..255)        */
#define PWM_G       0x08   /* bits 7..0 = green duty                 */
#define PWM_B       0x0C   /* bits 7..0 = blue duty                  */
/* Duty n -> n/256 high time; 255 = 99.6 %. PWM freq = 100 MHz/256 = 390.6 kHz */

/* ---------------- 7-segment patterns (active low) -----------------------
 * bit 0 = CA ... bit 6 = CG, bit 7 = DP; 0 switches a segment ON          */
static const u8 seg[16] = {
    0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8,   /* 0..7 */
    0x80, 0x90, 0x88, 0x83, 0xC6, 0xA1, 0x86, 0x8E    /* 8..F */
};

/* Multiplex a 16-bit value onto the 4 rightmost digits (~1 ms per digit) */
static void show_hex(u16 v)
{
    for (int d = 0; d < 4; d++) {
        u32 nib = (v >> (4 * d)) & 0xF;                    /* hex digit d    */
        Xil_Out32(GPIO1_BASE + AN_REG, 0xFF);              /* all digits off */
        Xil_Out32(GPIO1_BASE + SEG_REG, seg[nib]);         /* pattern        */
        Xil_Out32(GPIO1_BASE + AN_REG, ~(1u << d) & 0xFF); /* digit d on     */
        usleep(1000);                                      /* hold 1 ms      */
    }
}

int main(void)
{
    /* GPIO1 both channels = outputs; display off at start (active low) */
    Xil_Out32(GPIO1_BASE + SEG_TRI, 0x00);
    Xil_Out32(GPIO1_BASE + AN_TRI,  0x00);
    Xil_Out32(GPIO1_BASE + AN_REG,  0xFF);
    Xil_Out32(GPIO1_BASE + SEG_REG, 0xFF);

    /* PWM: all colours off, then enable */
    Xil_Out32(PWM_BASE + PWM_R, 0);
    Xil_Out32(PWM_BASE + PWM_G, 0);
    Xil_Out32(PWM_BASE + PWM_B, 0);
    Xil_Out32(PWM_BASE + PWM_CTRL, 1);

    xil_printf("Lab 03: switches -> LEDs, 7-segment, RGB LED\r\n");

    while (1) {
        u32 sw = Xil_In32(GPIO0_BASE + GPIO_SW) & 0xFFFF;   /* read switches */

        Xil_Out32(GPIO0_BASE + GPIO_LED, sw);                /* LEDs = SW     */

        Xil_Out32(PWM_BASE + PWM_R, ((sw >> 0) & 0xF) * 17); /* SW3..0  red   */
        Xil_Out32(PWM_BASE + PWM_G, ((sw >> 4) & 0xF) * 17); /* SW7..4  green */
        Xil_Out32(PWM_BASE + PWM_B, ((sw >> 8) & 0xF) * 17); /* SW11..8 blue  */

        show_hex((u16)sw);                                   /* hex on display*/
    }
}