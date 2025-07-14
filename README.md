# c-esp32-firmware

這個專案收錄了一些 C 語言與韌體開發的練習題，使用esp32練習涵蓋基礎的函式實作、硬體中斷處理、以及 RTOS 任務管理，適合用來練習與測試韌體開發技巧。

## 題目簡介

### 1. 自行實作 `memcpy` 並了解原理
**目標：**  
熟悉標準 C 函式的行為與記憶體操作原理。  
**內容：**  
在 `components/` 資料夾中自建一個元件（例如 `my_memcpy`），自行實作 `memcpy` 。  
**實作檔案：**  
c-esp32-firmware/practice_memcpy/my_memcpy.c

---

### 2. UART 指令控制 LED（使用 UART 接收緩衝）
**目標：**  
學習如何透過 UART 接收文字指令，解析後控制硬體（此處為 LED）。  
**內容：**  
- 持續接收使用者輸入的字串（例如 LED ON / LED OFF） 
- 當接收到換行符號（'\n' 或 '\r'）時觸發指令解析  
- 根據字串內容控制 GPIO2（開關 LED）

**實作檔案：**
  UART-test/main.c

---

### 3. FreeRTOS 任務排程（多任務 + 事件同步） 
**目標：**  
練習 FreeRTOS 任務建立、事件同步，以及任務間狀態傳遞，整合硬體輸出與使用者輸入互動。

**功能說明：**  
此範例建立三個任務，實作按鈕控制 LED 閃爍頻率，並在 UART 輸出當前狀態。

| 任務名稱      | 說明 |
|---------------|------|
| `led_task`    | 根據當前狀態 `flash_state` 控制 LED 閃爍頻率 |
| `uart_task`   | 每次狀態改變時，在 UART 上輸出目前 LED 閃爍頻率 |
| `button_scan` | 偵測按鈕事件（GPIO0），每按一次切換一種閃爍模式，並通知 UART 任務 |

**使用技術：**
- FreeRTOS 多任務排程 (`xTaskCreate`)  
- 事件同步 (`xEventGroupSetBits`, `xEventGroupWaitBits`)  
- GPIO 控制與按鈕去彈跳  
- UART 資訊輸出

**實作檔案：**
  FreeRTOS_Exercises/main.c
