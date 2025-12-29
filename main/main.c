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

#include "adc.h"
#include "timer.h"
#include "gpio.h"

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
QueueHandle_t ecg_http_queue_handle = NULL;
gptimer_handle_t adc_timer_handle = NULL; //trigger timer for ADC
esp_http_client_handle_t client = NULL;
HPFType HPF;
SBFType SBF;
LPFType LPF;
//Define

//Typedef

//Functions Prototype
int16_t filtering(int data);
void build_json_packet(char *buffer, size_t max_len, const ecg_msg_t *msg);
int16_t downsampling(int16_t data);
//Tasks
void ECG_Reading_Task(void *pvParameters){
    static int raw = 0;
    static ecg_msg_t ecg_msg;
    static rr_buffer_t rr;
    for (int i = 0; i < RR_Buffer_Size; i++)
        rr.buffer[i] = 0;
    rr.index = 0;
    static int8_t is_r_peak = 0;
    static int16_t ecg_index = 0;
    static uint16_t L0_MINUS_COUNT = 0;
    static uint16_t L0_PLUS_COUNT = 0;
    while (1){
        if (xSemaphoreTake(adc_timer_semaphore_handle, portMAX_DELAY) == pdTRUE)
        {
            //Read ecg value
            ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_Channel, &raw));
            int16_t filtered_data = filtering(raw);
            //add filtered data into buffer
            ecg_msg.ecg_buffer[ecg_index++] = filtered_data;
            L0_MINUS_COUNT += gpio_get_level(GPIO_L0_MINUS_PIN);
            L0_PLUS_COUNT += gpio_get_level(GPIO_L0_PLUS_PIN);
            //detect r peak and calculate heart rate
            int16_t downsampling_data = downsampling(filtered_data);
            if (downsampling_data != IGNORE){
                current_time = esp_timer_get_time() / 1000;
                is_r_peak = (PT_StateMachine(downsampling_data) != 0) ? 1 : 0;
                if (is_r_peak){
                    uint32_t RR = (current_time - last_r_peak_time);
                    //HR between 30 and 220 BPM
                    if(RR >= 273 && RR < 2000){
                        rr.buffer[rr.index] = (int16_t)RR;
                        rr.index = (rr.index + 1) % RR_Buffer_Size;
                    } 
                    last_r_peak_time = current_time;
                }
            }
            if (ecg_index >= ECG_Buffer_Size){
                ecg_msg.time = (esp_timer_get_time() / 1000);
                ecg_msg.L0_MINUS = (L0_MINUS_COUNT > (ECG_Buffer_Size / 2)) ? 1 : 0;
                ecg_msg.L0_PLUS = (L0_PLUS_COUNT > (ECG_Buffer_Size / 2)) ? 1 : 0;
                int8_t head = rr.index;
                int8_t part1_len = RR_Buffer_Size - head;
                memcpy(&ecg_msg.rr_buffer[0], &rr.buffer[head], part1_len * sizeof(int16_t));
                if (head > 0)
                memcpy(&ecg_msg.rr_buffer[part1_len], &rr.buffer[0], head * sizeof(int16_t));
                xQueueSend(ecg_http_queue_handle, &ecg_msg, 0);
                ecg_index = 0;
                L0_MINUS_COUNT = 0;
                L0_PLUS_COUNT = 0;
            }
        }
    }
}

void HTTP_Transmission_Task(void *pvParameters){
    static ecg_msg_t ecg_msg;
    static char json[4096];
    while (1){
        if (xQueueReceive(ecg_http_queue_handle, &ecg_msg, portMAX_DELAY) == pdTRUE){
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
    //Peripherals initialization
    GPIO_Init();
    ADC_Init();
    //Digital signal processing filter
    HPF_init(&HPF);
    SBF_init(&SBF);
    LPF_init(&LPF);
    //Pan-tompkins algorithm initialization
    PT_init();
    //freeRTOS
    Queue_Init(&ecg_http_queue_handle, 10, sizeof(ecg_msg_t));
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
        2,       // priority
        NULL,    // handle
        1);      // core ID
    //Core 0 Tasks
    xTaskCreatePinnedToCore(
    HTTP_Transmission_Task,
    "HTTP_Transmition_Task",
    8192   ,   // stack size
    NULL,    // parameters
    0,       // priority
    NULL,    // handle
    0);      // core ID
        
}

//Functions Definition
int16_t filtering(int data){
    float nor_data = (float)data;
    //0.5Hz order 4 HPF
    HPF_writeInput(&HPF, nor_data);
    float HPF_Output = HPF_readOutput(&HPF); 
    //40-50Hz order 4 SBF
    SBF_writeInput(&SBF, HPF_Output);
    float SBF_Output = SBF_readOutput(&SBF); 
    //40HZ order 4 LPF 
    LPF_writeInput(&LPF, SBF_Output);
    int16_t filtered_data = (int16_t)LPF_readOutput(&LPF);
    
    return filtered_data;
}

//downsampling 500Hz signal into 200Hz signal
int16_t downsampling(int16_t data){
    static uint8_t cycle_counter = 0;
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
//build JSON function
void build_json_packet(char *buffer, size_t max_len, const ecg_msg_t *msg) 
{
    int offset = 0;
    int ret = 0;

    ret = snprintf(buffer + offset, max_len - offset, "{\"ecg_buffer\":[");
    offset += ret;
    for (int i = 0; i < ECG_Buffer_Size; i++) {
        const char *sep = (i == 0) ? "" : ",";
        ret = snprintf(buffer + offset, max_len - offset, "%s%d", sep, msg->ecg_buffer[i]);
        if (ret < 0 || ret >= max_len - offset) return; 
        offset += ret;
    }

    ret = snprintf(buffer + offset, max_len - offset, "],\"rr_buffer\":[");
    offset += ret;
    
    for (int i = 0; i < RR_Buffer_Size; i++) {
        const char *sep = (i == 0) ? "" : ",";
        ret = snprintf(buffer + offset, max_len - offset, "%s%d", sep, msg->rr_buffer[i]);
        if (ret < 0 || ret >= max_len - offset) return;
        offset += ret;
    }

    ret = snprintf(buffer + offset, max_len - offset, "],\"L0_MINUS\":%d,\"L0_PLUS\":%d,\"time\":%llu}", msg->L0_MINUS, msg->L0_PLUS, msg->time);
    offset += ret;
}
