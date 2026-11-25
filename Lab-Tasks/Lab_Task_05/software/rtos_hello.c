/* Lab 05, Part B: first FreeRTOS test. Two tasks and a queue; LED0 blinks, the terminal shows the data.
   Set the addresses from your Address Editor. Use as the ONLY content of main.c. Not compiled by the author. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "xil_io.h"
#include "xil_printf.h"
#include "xil_cache.h"

#define GPIO0_BASE 0x40000000u   /* LEDs and switches */
#define GPIO_LED   0x08u         /* ch 2 data: LEDs (check your design) */

static QueueHandle_t g_q;

static void producer_task(void *arg)
{
  u32 n = 0;
  for (;;) {
    xQueueSend(g_q, &n, portMAX_DELAY);
    n++;
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

static void consumer_task(void *arg)
{
  u32 v;
  for (;;) {
    if (xQueueReceive(g_q, &v, portMAX_DELAY) == pdPASS)
      xil_printf("received %d at tick %d\r\n", (int)v, (int)xTaskGetTickCount());
  }
}

static void blink_task(void *arg)
{
  u32 on = 0;
  for (;;) {
    on ^= 1;
    Xil_Out32(GPIO0_BASE + GPIO_LED, on);
    vTaskDelay(pdMS_TO_TICKS(250));
  }
}

void vApplicationMallocFailedHook(void)
{
  xil_printf("malloc failed\r\n");
  for (;;) { }
}

void vApplicationStackOverflowHook(TaskHandle_t t, char *name)
{
  xil_printf("stack overflow in %s\r\n", name);
  for (;;) { }
}

int main(void)
{
  Xil_ICacheEnable();
  Xil_DCacheEnable();
  xil_printf("\r\nFreeRTOS hello\r\n");
  g_q = xQueueCreate(4, sizeof(u32));
  xTaskCreate(producer_task, "prod",  256, NULL, 2, NULL);
  xTaskCreate(consumer_task, "cons",  512, NULL, 2, NULL);
  xTaskCreate(blink_task,    "blink", 256, NULL, 1, NULL);
  vTaskStartScheduler();
  for (;;) { }
}
