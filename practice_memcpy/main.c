// esp32/my_memcpy/main/main.c
#include "esp_timer.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "my_memcpy.h"  // 自定義的memcpy
//void *可以回傳任何型別的記憶體位址

void app_main(void)
{
    char source[] = "ESP32 MEMCPY TEST";
    char dest[32];
    int64_t start_time = esp_timer_get_time();
    my_memcpy(dest, source, sizeof(source));
    int64_t end_time = esp_timer_get_time();
    printf("Copied: %s\n", dest);
    printf("my_memcpy took %lld microseconds\n", end_time - start_time);
}