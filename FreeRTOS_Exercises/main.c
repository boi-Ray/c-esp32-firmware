#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/event_groups.h"

// 設定 LED 腳位（大部分 ESP32 板子的內建 LED 是 GPIO2）
#define LED_GPIO GPIO_NUM_2
#define UART_PORT UART_NUM_0
#define BUTTON_GPIO GPIO_NUM_0
//事件組
EventGroupHandle_t event_group;
#define EVENT_STATE_CHANGED BIT0

// 建立閃爍狀態
int flash_state  = 0;

// 任務 A：3種 LED 閃爍狀態
void led_task(void *pvParameter) {
    int led_state = 0;
    int current_state = 0;
    while (1) {

        current_state = flash_state;
        //printf("[LED] Got new state: %d\n", current_state);
        led_state = !led_state;                         // 切換 0 ↔ 1
        gpio_set_level(LED_GPIO, led_state);            // 寫入 LED 腳位

        switch ( current_state )
        {
            //led狀態
            case 0:
                vTaskDelay(500 / portTICK_PERIOD_MS);          // vTaskDelay() 單位為tick = 毫秒 / 每個 tick 持續的毫秒數
                break;
            case 1:        
                vTaskDelay(200 / portTICK_PERIOD_MS);          
                break;
            
            case 2:          
                vTaskDelay(100 / portTICK_PERIOD_MS);          
                break;        
        }

    }
}

// 任務 B：每 1 秒輸出 LED狀態
void uart_task(void *pvParameter) {
    EventBits_t bits;
    
    while (1) {
        // 等待事件（自動清除）
        bits = xEventGroupWaitBits(event_group, EVENT_STATE_CHANGED, pdTRUE, pdFALSE, portMAX_DELAY);

        if (bits & EVENT_STATE_CHANGED) {
            printf("[UART] flash_state 為 %d\n", flash_state);

            switch (flash_state) {
                case 0: printf("LED Flashing frequency is 2 Hz\n"); break;
                case 1: printf("LED Flashing frequency is 5 Hz\n"); break;
                case 2: printf("LED Flashing frequency is 10 Hz\n"); break;
            }
        }
    }
}

// 按鈕掃描
void button_scan(void *pvParameter){
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY);  // 啟用內建上拉
    
    int last_state =0;  // 前一個狀態
    
    while (1) {
        int btn_state = gpio_get_level(BUTTON_GPIO);
        if (last_state-btn_state == 1){
            printf("按鈕狀態：%s\n", btn_state == 0 ? "按下" : "未按");
            flash_state++;
            flash_state = flash_state % 3;
            xEventGroupSetBits(event_group, EVENT_STATE_CHANGED);

        }
        
        last_state = btn_state;        
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void app_main(void) {
     
    // 初始化 GPIO
    esp_rom_gpio_pad_select_gpio(LED_GPIO);         //LED GPIO 設定成GPIO模式
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT); //LED GPIO 設成輸出

    // 建立事件組
    event_group = xEventGroupCreate();
    // 建立兩個任務
    xTaskCreate(led_task, "led_task", 2048, NULL, 5, NULL);
    xTaskCreate(uart_task, "uart_task", 2048, NULL, 5, NULL);
    xTaskCreate(button_scan, "button_scan", 2048, NULL, 5, NULL);
}
