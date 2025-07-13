#include <stdio.h>
#include <string.h>
#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"



#define LED_GPIO GPIO_NUM_2          // 預設 LED 接腳
#define UART_PORT UART_NUM_0         // 使用 UART0（USB 連線）
#define BUF_SIZE 128

void led_control(const char *cmd) {
    if (strcmp(cmd, "LED ON") == 0) {
        gpio_set_level(LED_GPIO, 1);
        printf("LED 開啟\n");
    } else if (strcmp(cmd, "LED OFF") == 0) {
        gpio_set_level(LED_GPIO, 0);
        printf("LED 關閉\n");
    } else {
        printf("不明指令：%s\n", cmd);
    }
}

void uart_task(void *arg) {
    uint8_t ch;
    char line[BUF_SIZE];
    int idx = 0;

    while (1) {
        int len = uart_read_bytes(UART_PORT, &ch, 1, pdMS_TO_TICKS(100));
        if (len > 0) {
            if (ch == '\n' || ch == '\r') {
                if (idx > 0) {
                    line[idx] = '\0';     // 加入字串結尾
                    printf("收到指令：%s\n", line);
                    led_control(line);    // 執行指令
                    idx = 0;              // 重設 buffer
                }
            } else if (idx < BUF_SIZE - 1) {
                line[idx++] = ch;         // 加入 buffer
            }
        }
    }
}
// 初始化 UART（UART0, 波特率 115200）
void uart_init(void) {
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_PORT, &uart_config);
    uart_set_pin(UART_PORT, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE,
                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}
// 初始化 GPIO
void gpio_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << LED_GPIO,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
}

void app_main(void) {
    
    uart_init();
    gpio_init();

    // 建立 UART 任務
    xTaskCreate(uart_task, "uart_task", 2048, NULL, 10, NULL);
}