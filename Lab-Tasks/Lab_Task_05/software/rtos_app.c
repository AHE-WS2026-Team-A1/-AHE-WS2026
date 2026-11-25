/* Lab 05, Part C: FreeRTOS I/O controller with UART shell on MicroBlaze with DDR2.
   Tasks: input (switches), display (7-segment), logger (ring buffer in DDR2), shell (UART), monitor.
   A software timer blinks LED15 as heartbeat.
   Set all addresses from your Address Editor. Use as the ONLY content of main.c (one main per file).
   Not compiled by the author: check it in your release.

   CHANGES (Lab 05, Nexys A7-100T, Vitis 2025.2):
   - Addresses checked against the Address Editor: no change needed.
   - print_stats: stack marks via uxTaskGetSystemState (see CHANGED below). */
#include <string.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "xil_io.h"
#include "xil_printf.h"
#include "xil_cache.h"

/* ---- addresses and register offsets: take them from your design ---- */
#define UART_BASE   0x40600000u   /* AXI UART Lite (registers: RX 0x00, STAT 0x08) */
#define GPIO0_BASE  0x40000000u   /* AXI GPIO: LEDs and switches (board interfaces) */
#define GPIO_SW     0x00u         /* ch 1 data: switches (check your design) */
#define GPIO_LED    0x08u         /* ch 2 data: LEDs     (check your design) */
#define GPIO1_BASE  0x40010000u   /* AXI GPIO: 7-segment display (custom) */
#define SEG_REG     0x00u         /* ch 1: segments */
#define AN_REG      0x08u         /* ch 2: digit enables */

#if (configTICK_RATE_HZ < 1000)
#error "Set tick_rate to 1000 in the FreeRTOS settings (the display task needs a 2 ms delay)"
#endif

#define LOG_ENTRIES 65536u        /* 65536 x 8 bytes = 512 KiB, allocated from the heap (DDR2) */

typedef struct { u32 tick; u32 sw; } event_t;

static event_t           *g_log;
static volatile u32       g_log_head, g_log_count;
static QueueHandle_t      g_q;
static SemaphoreHandle_t  g_print_mx;
static volatile u32       g_seg_value = 0;
static volatile u32       g_mirror    = 1;
static volatile u32       g_monitor   = 0;
static u32                g_led       = 0;
static TaskHandle_t       h_input, h_display, h_logger, h_shell, h_monitor;

static const u8 seg_tab[16] = {
  0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,
  0x80,0x90,0x88,0x83,0xC6,0xA1,0x86,0x8E };

/* xil_printf is not thread safe: every task prints through this macro */
#define PRINT(...) do { xSemaphoreTake(g_print_mx, portMAX_DELAY); \
                        xil_printf(__VA_ARGS__); \
                        xSemaphoreGive(g_print_mx); } while (0)

static void led_update(u32 mask, u32 val)
{
  taskENTER_CRITICAL();
  g_led = (g_led & ~mask) | (val & mask);
  Xil_Out32(GPIO0_BASE + GPIO_LED, g_led);
  taskEXIT_CRITICAL();
}

static int uart_getc(void)             /* non-blocking: -1 if no character */
{
  if (Xil_In32(UART_BASE + 0x08) & 0x01) return (int)(Xil_In32(UART_BASE + 0x00) & 0xFF);
  return -1;
}

/* ---- tasks ---- */
static void input_task(void *arg)      /* samples the switches every 20 ms */
{
  u32 last = Xil_In32(GPIO0_BASE + GPIO_SW) & 0xFFFF;
  TickType_t t = xTaskGetTickCount();
  for (;;) {
    vTaskDelayUntil(&t, pdMS_TO_TICKS(20));
    u32 sw = Xil_In32(GPIO0_BASE + GPIO_SW) & 0xFFFF;
    if (sw != last) {
      event_t e = { (u32)xTaskGetTickCount(), sw };
      xQueueSend(g_q, &e, 0);          /* drop the event if the queue is full */
      last = sw;
      if (g_mirror) led_update(0x7FFF, sw);
    }
  }
}

static void display_task(void *arg)    /* multiplexes the four right digits */
{
  Xil_Out32(GPIO1_BASE + 0x4, 0x00);   /* both channels: outputs */
  Xil_Out32(GPIO1_BASE + 0xC, 0x00);
  for (;;) {
    u32 v = g_seg_value;
    for (int d = 0; d < 4; d++) {
      Xil_Out32(GPIO1_BASE + AN_REG, 0xFF);
      Xil_Out32(GPIO1_BASE + SEG_REG, seg_tab[(v >> (4 * d)) & 0xF]);
      Xil_Out32(GPIO1_BASE + AN_REG, ~(1u << d) & 0xFF);
      vTaskDelay(pdMS_TO_TICKS(2));
    }
  }
}

static void logger_task(void *arg)     /* stores events in the ring buffer in DDR2 */
{
  event_t e;
  for (;;) {
    if (xQueueReceive(g_q, &e, portMAX_DELAY) == pdPASS) {
      g_log[g_log_head] = e;
      g_log_head = (g_log_head + 1) % LOG_ENTRIES;
      if (g_log_count < LOG_ENTRIES) g_log_count++;
    }
  }
}

static void print_stats(void)
{
  PRINT("uptime %d ticks, heap free %d bytes (minimum %d), log %d entries\r\n",
        (int)xTaskGetTickCount(), (int)xPortGetFreeHeapSize(),
        (int)xPortGetMinimumEverFreeHeapSize(), (int)g_log_count);
#if (INCLUDE_uxTaskGetStackHighWaterMark == 1)
  PRINT("stack free (words): input %d, display %d, logger %d, shell %d, monitor %d\r\n",
        (int)uxTaskGetStackHighWaterMark(h_input),   (int)uxTaskGetStackHighWaterMark(h_display),
        (int)uxTaskGetStackHighWaterMark(h_logger),  (int)uxTaskGetStackHighWaterMark(h_shell),
        (int)uxTaskGetStackHighWaterMark(h_monitor));
#elif (configUSE_TRACE_FACILITY == 1)
  /* CHANGED (Lab 05): the Vitis 2025.2 FreeRTOS platform offers no option for
     INCLUDE_uxTaskGetStackHighWaterMark, so stats printed "n/a". The stack marks are
     read with uxTaskGetSystemState instead (needs freertos_use_trace_facility = true).
     The array is static so that it does not use the stack that is being measured. */
  {
    static TaskStatus_t st[12];
    UBaseType_t n = uxTaskGetSystemState(st, 12, NULL);
    PRINT("stack free (words):");
    for (UBaseType_t i = 0; i < n; i++)
      PRINT(" %s %d", st[i].pcTaskName, (int)st[i].usStackHighWaterMark);
    PRINT("\r\n");
  }
#else
  PRINT("stack free: n/a (enable INCLUDE_uxTaskGetStackHighWaterMark in the platform)\r\n");
#endif
}

static void monitor_task(void *arg)
{
  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(5000));
    if (g_monitor) print_stats();
  }
}

static void run_cmd(char *line)
{
  char *cmd = strtok(line, " ");
  char *arg = strtok(NULL, " ");
  if (!cmd) return;
  if (!strcmp(cmd, "help")) {
    PRINT("help | led <hex> | sw | seg <hex> | mirror on/off | log [clear] | stats | monitor on/off\r\n");
  } else if (!strcmp(cmd, "led") && arg) {
    g_mirror = 0;  led_update(0x7FFF, strtoul(arg, 0, 16));
  } else if (!strcmp(cmd, "sw")) {
    PRINT("switches: %04x\r\n", (unsigned)(Xil_In32(GPIO0_BASE + GPIO_SW) & 0xFFFF));
  } else if (!strcmp(cmd, "seg") && arg) {
    g_seg_value = strtoul(arg, 0, 16) & 0xFFFF;
  } else if (!strcmp(cmd, "mirror") && arg) {
    g_mirror = !strcmp(arg, "on");
  } else if (!strcmp(cmd, "monitor") && arg) {
    g_monitor = !strcmp(arg, "on");
  } else if (!strcmp(cmd, "stats")) {
    print_stats();
  } else if (!strcmp(cmd, "log")) {
    if (arg && !strcmp(arg, "clear")) {
      taskENTER_CRITICAL(); g_log_head = 0; g_log_count = 0; taskEXIT_CRITICAL();
    } else {
      u32 n = g_log_count < 10 ? g_log_count : 10;
      for (u32 i = 0; i < n; i++) {
        u32 idx = (g_log_head + LOG_ENTRIES - 1 - i) % LOG_ENTRIES;
        PRINT("tick %d: switches %04x\r\n", (int)g_log[idx].tick, (unsigned)g_log[idx].sw);
      }
      PRINT("%d entries in total\r\n", (int)g_log_count);
    }
  } else {
    PRINT("unknown command, try help\r\n");
  }
}

static void shell_task(void *arg)
{
  char line[64]; unsigned n = 0;
  PRINT("\r\n> ");
  for (;;) {
    int c = uart_getc();
    if (c < 0) { vTaskDelay(pdMS_TO_TICKS(10)); continue; }
    if (c == '\r' || c == '\n') {
      line[n] = 0; PRINT("\r\n");
      if (n) run_cmd(line);
      n = 0; PRINT("> ");
    } else if ((c == 8 || c == 127) && n) {
      n--; PRINT("\b \b");
    } else if (c >= 32 && c < 127 && n < sizeof(line) - 1) {
      line[n++] = (char)c; PRINT("%c", c);
    }
  }
}

static void heartbeat_cb(TimerHandle_t t)       /* software timer: LED15 blinks every 500 ms */
{
  static u32 s;
  s ^= 1;
  led_update(0x8000, s ? 0x8000 : 0);
}

/* ---- hooks required by some FreeRTOS configurations (delete them if the linker reports duplicates) ---- */
void vApplicationMallocFailedHook(void) { xil_printf("malloc failed\r\n"); for (;;) { } }
void vApplicationStackOverflowHook(TaskHandle_t t, char *name) { xil_printf("stack overflow in %s\r\n", name); for (;;) { } }

int main(void)
{
  Xil_ICacheEnable();
  Xil_DCacheEnable();
  xil_printf("\r\nFreeRTOS I/O controller\r\n");

  g_print_mx = xSemaphoreCreateMutex();
  g_q        = xQueueCreate(16, sizeof(event_t));
  g_log      = (event_t *)pvPortMalloc(LOG_ENTRIES * sizeof(event_t));
  if (!g_print_mx || !g_q || !g_log) { xil_printf("allocation failed: check the heap size\r\n"); for (;;) { } }

  xTaskCreate(input_task,   "input",   256, NULL, 3, &h_input);
  xTaskCreate(display_task, "display", 256, NULL, 3, &h_display);
  xTaskCreate(logger_task,  "logger",  256, NULL, 2, &h_logger);
  xTaskCreate(shell_task,   "shell",   768, NULL, 2, &h_shell);
  xTaskCreate(monitor_task, "monitor", 512, NULL, 1, &h_monitor);

  TimerHandle_t hb = xTimerCreate("hb", pdMS_TO_TICKS(500), pdTRUE, NULL, heartbeat_cb);
  xTimerStart(hb, 0);

  vTaskStartScheduler();
  for (;;) { }
}