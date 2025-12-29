#define ECG_Buffer_Size 500
#define RR_Buffer_Size    60
#define IGNORE           -10000 

typedef struct {
    int16_t buffer[RR_Buffer_Size];
    int16_t index;
} rr_buffer_t;

typedef struct {  
    int16_t ecg_buffer[ECG_Buffer_Size];
    int16_t rr_buffer[RR_Buffer_Size];
    uint8_t L0_MINUS;
    uint8_t L0_PLUS;
    uint64_t time;
} ecg_msg_t;

uint32_t last_r_peak_time = 0, current_time = 0;