#define STREAMING_WINDOW 500
#define RR_Batch_Size    60
#define IGNORE           -10000 

typedef struct {  
    int16_t ecg_batch[STREAMING_WINDOW];      
    int16_t RR_buffer[RR_Batch_Size];   
    uint64_t time; 
} ecg_msg_t;

uint32_t last_r_peak_time = 0, current_time = 0;