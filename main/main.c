/*Code is written for ESP32-DevKitC-32U*/

//Inlcude
#include <stdio.h>
#include "main.h"

#include "esp_log.h"
#include "esp_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "semaphore.h"
#include "Queue.h"

#include "wifi_sta.h"
#include "tb_http.h"
//#include "MQTT.h"

#include "adc.h"
#include "timer.h"
#include "driver/gpio.h"

#include "HPF.h"
#include "LPF.h"
#include "SBF.h"

#include "PanTompkins.h"
#include <math.h>

//Global variables
const char                *ECG_Sensor = "AD8232"; //Tag for writing log
const char                *Wifi_STA   = "Wifi Station"; //Tag for writing log
SemaphoreHandle_t adc_timer_semaphore_handle = NULL; //semphore to synchronize adc and timer
adc_oneshot_unit_handle_t adc_handle = NULL;   // main handle ADC 
QueueHandle_t ecg_https_queue_handle = NULL;
gptimer_handle_t adc_timer_handle = NULL; //trigger timer for ADC
esp_http_client_handle_t client = NULL;
HPFType HPF;
SBFType SBF;
LPFType LPF;
//Define

//Typedef

//Functions Prototype
float filtering(int data);
void build_json_packet(char *buffer, size_t max_len, const ecg_msg_t *msg);
int16_t downsampling(int16_t data);
//Tasks
void ECG_Reading_Task(void *pvParameters){
    static int raw = 0;
    static ecg_msg_t ecg_msg;
    static int16_t ecg_batch_index = 0;
    static int16_t RR_history_buffer[RR_Batch_Size] = {0};
    static int8_t RR_history_buffer_index = 0;
    static int8_t is_r_peak = 0;
    while (1){
        if (xSemaphoreTake(adc_timer_semaphore_handle, portMAX_DELAY) == pdTRUE)
        {
            //Read ecg value
            ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_Channel, &raw));
            //filtering data
            int16_t filtered_data = (int16_t)filtering(raw);
            //add filtered data into batch
            ecg_msg.ecg_batch[ecg_batch_index++] = filtered_data;
            //detect r peak and calculate heart rate
            int16_t downsampling_data = downsampling(filtered_data);
            if (downsampling_data != IGNORE){
                current_time = esp_timer_get_time() / 1000;
                is_r_peak = (PT_StateMachine(downsampling_data) != 0) ? 1 : 0;
                if (is_r_peak){
                    uint32_t RR = (current_time - last_r_peak_time);
                    //HR between 30 and 220 BPM
                    if(RR >= 273 && RR < 2000){
                        RR_history_buffer[RR_history_buffer_index] = (int16_t)RR;
                        RR_history_buffer_index = (RR_history_buffer_index + 1) % RR_Batch_Size;
                    } 
                    last_r_peak_time = current_time;
                }
            }
            printf("%d,%d,%d\n", is_r_peak * 400, filtered_data, PT_get_LongTimeHR_output(200));
            //send to https task if batch is full
            if (ecg_batch_index >= STREAMING_WINDOW){
                ecg_msg.time = (esp_timer_get_time() / 1000);
                int8_t head = RR_history_buffer_index;
                int8_t part1_len = RR_Batch_Size- head;
                memcpy(&ecg_msg.RR_buffer[0], &RR_history_buffer[head], part1_len * sizeof(int16_t));
                if (head > 0)
                    memcpy(&ecg_msg.RR_buffer[part1_len], &RR_history_buffer[0], head * sizeof(int16_t));
                xQueueSend(ecg_https_queue_handle, &ecg_msg, pdMS_TO_TICKS(100));
                ecg_batch_index = 0;
            }
        }
    }
}

void HTTPS_Transmission_Task(void *pvParameters){
    static ecg_msg_t ecg_msg;
    static char json[4096];
    while (1){
        if (xQueueReceive(ecg_https_queue_handle, &ecg_msg, portMAX_DELAY) == pdTRUE){
            build_json_packet(json, sizeof(json), &ecg_msg);
            tb_http_send_json(json);
        }
    }
}

void app_main(void)
{
    //Initialize Wifi
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    if (CONFIG_LOG_MAXIMUM_LEVEL > CONFIG_LOG_DEFAULT_LEVEL) {
        /* If you only want to open more logs in the wifi module, you need to make the max level greater than the default level,
         * and call esp_log_level_set() before esp_wifi_init() to improve the log level of the wifi module. */
        esp_log_level_set("wifi", CONFIG_LOG_MAXIMUM_LEVEL);
    }

    ESP_LOGI(Wifi_STA, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
    tb_http_init();
    //MQTT ThingsBoard Initialization
    //mqtt_init();
    //Peripherals initialization
    //ADC initialization
    ADC_Init();

    HPF_init(&HPF);
    SBF_init(&SBF);
    LPF_init(&LPF);
    PT_init();
    //freeRTOS
    Queue_Init(&ecg_https_queue_handle, 20, sizeof(ecg_msg_t));
    Semaphore_Init(&adc_timer_semaphore_handle);
    //Timer initializtion
    Timer_Init();
    //Create Task
    //Core 1 Tasks
    xTaskCreatePinnedToCore(
        ECG_Reading_Task,
        "ECG_Reading_Task",
        4096,    // stack size
        NULL,    // parameters
        1,       // priority
        NULL,    // handle
        1);      // core ID
    //Core 0 Tasks
    xTaskCreatePinnedToCore(
    HTTPS_Transmission_Task,
    "HTTPS_Transmition_Task",
    8192   ,   // stack size
    NULL,    // parameters
    0,       // priority
    NULL,    // handle
    0);      // core ID
        
}

//Functions Definition
float filtering(int data){
    float nor_data = (float)data;
    //0.5Hz order 4 HPF
    HPF_writeInput(&HPF, nor_data);
    float HPF_Output = HPF_readOutput(&HPF); 
    //40-50Hz order 4 SBF
    SBF_writeInput(&SBF, HPF_Output);
    float SBF_Output = SBF_readOutput(&SBF); 
    //40HZ order 4 LPF 
    LPF_writeInput(&LPF, SBF_Output);
    float filtered_data = LPF_readOutput(&LPF);
    
    return filtered_data;
}

int16_t downsampling(int16_t data){
    static int8_t cycle_counter = 0;
    static int16_t index2_data = 0;
    int16_t downsampling_data = 0;
    switch(cycle_counter){
        case 0:
            downsampling_data = data;
            break;
        case 1:
            downsampling_data = IGNORE;
            break;
        case 2:
            index2_data = data;
            downsampling_data =  IGNORE;
            break;
        case 3:
            downsampling_data = (index2_data + data) / 2;
            break;
        case 4: 
            downsampling_data = IGNORE;
            break;
    }
    cycle_counter = (cycle_counter + 1) % 5;
    return downsampling_data;
}

void build_json_packet(char *buffer, size_t max_len, const ecg_msg_t *msg) 
{
    int offset = 0;
    int ret = 0;

    // 1. Mở JSON và ghi mảng ECG (Giữ nguyên phần ECG cũ)
    ret = snprintf(buffer + offset, max_len - offset, "{\"ecg_batch\":[");
    offset += ret;
    for (int i = 0; i < STREAMING_WINDOW; i++) {
        const char *sep = (i == 0) ? "" : ",";
        ret = snprintf(buffer + offset, max_len - offset, "%s%d", sep, msg->ecg_batch[i]);
        if (ret < 0 || ret >= max_len - offset) return; 
        offset += ret;
    }

    // 2. [THAY ĐỔI LỚN] Ghi mảng rr_list thay vì các chỉ số HR/HRV
    ret = snprintf(buffer + offset, max_len - offset, "],\"rr_buffer\":[");
    offset += ret;
    
    // RR_Batch_Size là 8 (như trong main.h)
    for (int i = 0; i < RR_Batch_Size; i++) {
        const char *sep = (i == 0) ? "" : ",";
        // Lấy dữ liệu từ buffer RR gốc
        ret = snprintf(buffer + offset, max_len - offset, "%s%d", sep, msg->RR_buffer[i]);
        if (ret < 0 || ret >= max_len - offset) return;
        offset += ret;
    }

    // 3. Đóng JSON (Chỉ gửi thêm time, bỏ hết hr, code, status cũ đi)
    ret = snprintf(buffer + offset, max_len - offset, "],\"time\":%llu}", msg->time);
    offset += ret;
}

