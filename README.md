# ESP32 韌體練習專案（C 語言 & FreeRTOS）

嗨，我是 Ray，這個專案是我在學習 ESP32 韌體開發過程中，針對幾個重要主題進行的實作練習。  
由於目前尚無相關工作經歷，因此我希望透過這份作品展示我在 **C 語言基礎、硬體控制、RTOS 任務管理** 方面的能力與學習成果。

所有程式均使用 C 語言撰寫，搭配 [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/) 框架開發，目標為模擬實際韌體開發中會遇到的問題並提出解決方案。

---

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

---

## 🔧 技術總結

這份專案涵蓋以下主題：

- C 語言函式實作與優化
- UART 基本輸入解析
- GPIO 控制與中斷處理
- FreeRTOS 任務與事件旗標應用
- Timer 中斷與非同步任務協作
- Morse Code Tree 演算法與資料結構整合

---

## 👋 想說的話

感謝您花時間閱讀這份作品。我相信韌體開發是一個很需要「解決問題能力」與「實作細節掌握」的領域，而我正是透過這樣的練習，不斷提升自己的技術。

如果您對這些內容感興趣，或願意給我一個學習與貢獻的機會，非常期待有機會與您聊聊 🙌
