#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_timer.h"
#include "freertos/event_groups.h"

// 建立事件群組
EventGroupHandle_t morse_event_group;

// 事件旗標定義
#define EVENT_DOT     (1 << 0)
#define EVENT_DASH    (1 << 1)
#define EVENT_char_space    (1 << 2)
#define EVENT_WORD_space    (1 << 3)

// GPIO 定義
#define BUTTON_GPIO     GPIO_NUM_0
#define LED_GPIO        GPIO_NUM_2

// 定時器參數
#define DEBOUNCE_INTERVAL_MS 20  // 20ms 去抖動輪詢
#define DEBOUNCE_THRESHOLD   3   // 持續確認 3 次（30ms）

// PWM 參數
#define PWM_FREQ_HZ     5000
#define PWM_RES         LEDC_TIMER_10_BIT// // PWM 解析度設定為 10 位元（範圍 0~1023）
#define PWM_DUTY_MAX    ((1 << 10) - 1)  // 1023 for 10-bit PWM

volatile int button_state = 0;
//volatile int debounce_count = 0;
//volatile int pwm_active = 0;
volatile int last_button_state = 0;         //上次按鈕狀態
volatile int press_duration_ms = 0;         //按下秒數
volatile int unpressed_duration_ms = 0;     //沒按秒數
volatile char morse_word[20];               
volatile int word_index = 0;
static bool word_space_sent = true;        //旗標

// === Morse Code Tree ===
typedef struct MorseNode {
    char symbol;  // 節點對應的字元（如 'A'）
    struct MorseNode *dot;   // 左子樹（.）
    struct MorseNode *dash;  // 右子樹（-）
} MorseNode;

//創建二元數記憶體 賦值
MorseNode* create_node(char symbol) {
    MorseNode* node = calloc(1, sizeof(MorseNode));
    node->symbol = symbol;
    return node;
}
//插入摩斯碼二元數
void insert_morse(MorseNode* root, const char* code, char symbol) {
    MorseNode* current = root;
    for (int i = 0; code[i]; i++) {
        if (code[i] == '.') {
            if (!current->dot) current->dot = create_node(0);
            current = current->dot;
        } else if (code[i] == '-') {
            if (!current->dash) current->dash = create_node(0);
            current = current->dash;
        }
    }
    current->symbol = symbol;
}

typedef struct {
    const char* code;
    char symbol;
} MorseEntry;

MorseEntry morse_table[] = {
    {".-", 'A'}, {"-...", 'B'}, {"-.-.", 'C'}, {"-..", 'D'}, {".", 'E'},
    {"..-.", 'F'}, {"--.", 'G'}, {"....", 'H'}, {"..", 'I'}, {".---", 'J'},
    {"-.-", 'K'}, {".-..", 'L'}, {"--", 'M'}, {"-.", 'N'}, {"---", 'O'},
    {".--.", 'P'}, {"--.-", 'Q'}, {".-.", 'R'}, {"...", 'S'}, {"-", 'T'},
    {"..-", 'U'}, {"...-", 'V'}, {".--", 'W'}, {"-..-", 'X'}, {"-.--", 'Y'},
    {"--..", 'Z'},
    {"-----", '0'}, {".----", '1'}, {"..---", '2'}, {"...--", '3'},
    {"....-", '4'}, {".....", '5'}, {"-....", '6'}, {"--...", '7'},
    {"---..", '8'}, {"----.", '9'},
    {".-.-.-", '.'}, {"--..--", ','}, {"..--..", '?'}, {"-.-.--", '!'},
    {"---...", ':'}, {".----.", '\''}, {".-..-.", '"'}, {"-...-", '='},
    {"-..-.", '/'}, {".-.-.", '+'}, {".--.-.", '@'}
};

MorseNode* morse_root = NULL;

//創建摩斯二元樹
MorseNode* build_morse_tree() {
    MorseNode* root = create_node(0);
    for (int i = 0; i < sizeof(morse_table) / sizeof(MorseEntry); i++) {
        insert_morse(root, morse_table[i].code, morse_table[i].symbol);
    }
    return root;
}

//摩斯解碼
char decode_morse(MorseNode* root, const char* code) {
    MorseNode* current = root;
    for (int i = 0; code[i]; i++) {
        if (code[i] == '.') {
            if (!current->dot) return '?';  
            current = current->dot;
        } else if (code[i] == '-') {
            if (!current->dash) return '?';
            current = current->dash;
        } else {
            return '?';
        }
    }
    return current->symbol ? current->symbol : '?';
}

//二元樹解碼顯示
void decode_and_print(char* morse_code) {
    char decoded = decode_morse(morse_root, morse_code);
    morse_word [word_index] = decoded;
    word_index++;                       // 字元索引
    printf(" => %c , %d\n", decoded , word_index);
}

// Timer 回呼函式：每 10ms 檢查按鈕狀態
void IRAM_ATTR debounce_timer_callback(void* arg) {
    int level = gpio_get_level(BUTTON_GPIO);
    button_state = level;
    if (level == 0) {  // 假設按鍵低電平觸發
        //剛按下時
        if(last_button_state == 1 &&level == 0){
            //空格時間
            if(unpressed_duration_ms > 300 && !word_space_sent){
                BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                xEventGroupSetBitsFromISR(morse_event_group, EVENT_char_space, &xHigherPriorityTaskWoken);
                unpressed_duration_ms = 0;
            }
        
        }else{
            press_duration_ms += DEBOUNCE_INTERVAL_MS;
            unpressed_duration_ms = 0;
        }
    word_space_sent = false;
    //剛放開按鈕時
    } else if(last_button_state == 0 &&level == 1){
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        //判斷點或劃
        if (press_duration_ms > 50) { // 避免誤觸
            if (press_duration_ms < 300) {
                xEventGroupSetBitsFromISR(morse_event_group, EVENT_DOT, &xHigherPriorityTaskWoken);
            
            } else {
                xEventGroupSetBitsFromISR(morse_event_group, EVENT_DASH, &xHigherPriorityTaskWoken);
            }
            press_duration_ms = 0;
        }
    }else{
        unpressed_duration_ms += DEBOUNCE_INTERVAL_MS; //無輸入時間
        press_duration_ms = 0;
        //若無動作則執行字詞空格
        if(unpressed_duration_ms > 700 && !word_space_sent ){
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            xEventGroupSetBitsFromISR(morse_event_group, EVENT_WORD_space, &xHigherPriorityTaskWoken);
            unpressed_duration_ms = 0;
            word_space_sent = true;
        }
    }
    last_button_state = level;
}

// 設定按鍵 GPIO
void button_gpio_init() {
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE
    };
    gpio_config(&io_conf);  // 套用 GPIO 初始化設定
}

void morse_task(void *arg) {
    char morse_buffer[10];  // 最多接收 10 符號
    int morse_index = 0;
    while (1) {
        // 等待任一個事件發生（並清除）
        EventBits_t bits = xEventGroupWaitBits(
            morse_event_group,
            EVENT_DOT | EVENT_DASH | EVENT_char_space | EVENT_WORD_space,
            pdTRUE,    // 清除已觸發的 bits
            pdFALSE,   // 任一個即可（不是 all）
            portMAX_DELAY
        );
        //點
        if (bits & EVENT_DOT) {
            printf(".");
            morse_buffer[morse_index++] = '.';
        }
        //劃
        if (bits & EVENT_DASH) {
            printf("-");
            morse_buffer[morse_index++] = '-';
        }
        //字元空格
        if (bits & EVENT_char_space) {
            printf(" ");
            morse_buffer[morse_index] = '\0';  // 結尾加 '\0'
            decode_and_print(morse_buffer);    // 自訂函式：把 ".-" 轉為 A
            morse_index = 0;                   // 清空
        }
        //詞空格
        if(bits &EVENT_WORD_space){
            morse_buffer[morse_index] = '\0'; 
            decode_and_print(morse_buffer);
            morse_word[word_index] = '\0';
            printf("%s \n",morse_word);
            morse_index = 0;
            word_index = 0;

            
        }
    }
}

void app_main() {
    button_gpio_init();
    //pwm_init();
    morse_event_group = xEventGroupCreate();
    // 建立 Timer 參數
    if (!morse_root)
    morse_root = build_morse_tree();

    const esp_timer_create_args_t debounce_timer_args = {
        .callback = &debounce_timer_callback,
        .name = "debounce_timer"
    };
    esp_timer_handle_t debounce_timer;                                      
    esp_timer_create(&debounce_timer_args, &debounce_timer);                // 創建Timer   
    esp_timer_start_periodic(debounce_timer, DEBOUNCE_INTERVAL_MS * 1000);  // 啟動Timer讓它在10ms週期性呼叫一次 callback 函數

    xTaskCreate(morse_task, "morse_task", 2048, NULL, 5, NULL);

}
