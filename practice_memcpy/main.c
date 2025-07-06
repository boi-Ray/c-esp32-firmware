
#include "esp_timer.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


void *my_memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;

    for (size_t i = 0; i < n; ++i) {
        d[i] = s[i];
    }

    return dest;
}

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
