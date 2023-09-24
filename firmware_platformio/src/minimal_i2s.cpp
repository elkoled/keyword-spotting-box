// /*
//  * Display A-weighted sound level measured by I2S Microphone
//  * 
//  * (c)2019 Ivan Kostoski
//  *
//  * This program is free software: you can redistribute it and/or modify
//  * it under the terms of the GNU General Public License as published by
//  * the Free Software Foundation, either version 3 of the License, or
//  * (at your option) any later version.
//  *    
//  * This program is distributed in the hope that it will be useful,
//  * but WITHOUT ANY WARRANTY; without even the implied warranty of
//  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  * GNU General Public License for more details.
//  *
//  * You should have received a copy of the GNU General Public License
//  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
//  */

// /*
//  * Sketch samples audio data from I2S microphone, processes the data 
//  * with digital IIR filters and calculates A or C weighted Equivalent 
//  * Continuous Sound Level (Leq)
//  * 
//  * I2S is setup to sample data at Fs=48000KHz (fixed value due to 
//  * design of digital IIR filters). Data is read from I2S queue 
//  * in 'sample blocks' (default 125ms block, equal to 6000 samples) 
//  * by 'i2s_reader_task', filtered trough two IIR filters (equalizer 
//  * and weighting), summed up and pushed into 'samples_queue' as 
//  * sum of squares of filtered samples. The main task then pulls data 
//  * from the queue and calculates decibel value relative to microphone 
//  * reference amplitude, derived from datasheet sensitivity dBFS 
//  * value, number of bits in I2S data, and the reference value for 
//  * which the sensitivity is specified (typically 94dB, pure sine
//  * wave at 1KHz).
//  * 
//  * Displays line on the small OLED screen with 'short' LAeq(125ms)
//  * response and numeric LAeq(1sec) dB value from the signal RMS.
//  */
// #include <Arduino.h>
// #include <driver/i2s.h>

// //
// // Configuration
// //

// // Customize these values from microphone datasheet
// #define MIC_SENSITIVITY   -26         // dBFS value expected at MIC_REF_DB (Sensitivity value from datasheet)
// #define MIC_REF_DB        94.0        // Value at which point sensitivity is specified in datasheet (dB)
// #define MIC_OVERLOAD_DB   116.0       // dB - Acoustic overload point
// #define MIC_NOISE_DB      29          // dB - Noise floor
// #define MIC_BITS          24          // valid number of bits in I2S data
// #define MIC_CONVERT(s)    (s >> (SAMPLE_BITS - MIC_BITS))
// #define MIC_TIMING_SHIFT  0           // Set to one to fix MSB timing for some microphones, i.e. SPH0645LM4H-x

// // Calculate reference amplitude value at compile time
// constexpr double MIC_REF_AMPL = pow(10, double(MIC_SENSITIVITY)/20) * ((1<<(MIC_BITS-1))-1);

// //
// // I2S pins - Can be routed to almost any (unused) ESP32 pin.
// //            SD can be any pin, inlcuding input only pins (36-39).
// //            SCK (i.e. BCLK) and WS (i.e. L/R CLK) must be output capable pins
// //
// // Below ones are just example for my board layout, put here the pins you will use
// //
// #define I2S_WS            15 
// #define I2S_SCK           14 
// #define I2S_SD            32 

// // I2S peripheral to use (0 or 1)
// #define I2S_PORT          I2S_NUM_0

// //
// // Sampling
// //
// #define SAMPLE_RATE       16000 // Hz, fixed to 48000 Hz as design of IIR filters
// #define SAMPLE_BITS       32    // bits
// #define SAMPLE_T          int32_t 
// #define SAMPLES_SHORT     (SAMPLE_RATE / 8) // ~125ms
// #define SAMPLES_LEQ       (SAMPLE_RATE * LEQ_PERIOD)
// #define DMA_BANK_SIZE     (SAMPLES_SHORT / 16)
// #define DMA_BANKS         32

// QueueHandle_t samples_queue;
// // Static buffer for block of samples
// float samples[SAMPLES_SHORT] __attribute__((aligned(4)));

// //
// // I2S Microphone sampling setup 
// //
// void mic_i2s_init() {
//   // Setup I2S to sample mono channel for SAMPLE_RATE * SAMPLE_BITS
//   // NOTE: Recent update to Arduino_esp32 (1.0.2 -> 1.0.3)
//   //       seems to have swapped ONLY_LEFT and ONLY_RIGHT channels
//   const i2s_config_t i2s_config = {
//     mode: i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
//     sample_rate: SAMPLE_RATE,
//     bits_per_sample: i2s_bits_per_sample_t(SAMPLE_BITS),
//     channel_format: I2S_CHANNEL_FMT_ONLY_LEFT,
//     communication_format: i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
//     intr_alloc_flags: ESP_INTR_FLAG_LEVEL1,
//     dma_buf_count: DMA_BANKS,
//     dma_buf_len: DMA_BANK_SIZE,
//     use_apll: true,
//     tx_desc_auto_clear: false,
//     fixed_mclk: 0
//   };
//   // I2S pin mapping
//   const i2s_pin_config_t pin_config = {
//     bck_io_num:   I2S_SCK,  
//     ws_io_num:    I2S_WS,    
//     data_out_num: -1, // not used
//     data_in_num:  I2S_SD   
//   };

//   i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);

//   #if (MIC_TIMING_SHIFT > 0) 
//     // Undocumented (?!) manipulation of I2S peripheral registers
//     // to fix MSB timing issues with some I2S microphones
//     REG_SET_BIT(I2S_TIMING_REG(I2S_PORT), BIT(9));   
//     REG_SET_BIT(I2S_CONF_REG(I2S_PORT), I2S_RX_MSB_SHIFT);  
//   #endif
  
//   i2s_set_pin(I2S_PORT, &pin_config);
// }

// //
// // I2S Reader Task
// //
// // Rationale for separate task reading I2S is that IIR filter
// // processing cam be scheduled to different core on the ESP32
// // while main task can do something else, like update the 
// // display in the example
// //
// // As this is intended to run as separate hihg-priority task, 
// // we only do the minimum required work with the I2S data
// // until it is 'compressed' into sum of squares 
// //
// // FreeRTOS priority and stack size (in 32-bit words) 
// #define I2S_TASK_PRI   4
// #define I2S_TASK_STACK 2048
// //
// void mic_i2s_reader_task(void* parameter) {
//   mic_i2s_init();

//   // Discard first block, microphone may have startup time (i.e. INMP441 up to 83ms)
//   size_t bytes_read = 0;
//   i2s_read(I2S_PORT, &samples, SAMPLES_SHORT * sizeof(int32_t), &bytes_read, portMAX_DELAY);

//   while (true) {
//     // Block and wait for microphone values from I2S
//     //
//     // Data is moved from DMA buffers to our 'samples' buffer by the driver ISR
//     // and when there is requested ammount of data, task is unblocked
//     //
//     // Note: i2s_read does not care it is writing in float[] buffer, it will write
//     //       integer values to the given address, as received from the hardware peripheral. 
//     i2s_read(I2S_PORT, &samples, SAMPLES_SHORT * sizeof(SAMPLE_T), &bytes_read, portMAX_DELAY);

//     TickType_t start_tick = xTaskGetTickCount();
    
//     // Convert (including shifting) integer microphone values to floats, 
//     // using the same buffer (assumed sample size is same as size of float), 
//     // to save a bit of memory
//     SAMPLE_T* int_samples = (SAMPLE_T*)&samples;
//     for(int i=0; i<SAMPLES_SHORT; i++) samples[i] = MIC_CONVERT(int_samples[i]);

//     /* TODO PROCESS SAMPLES */

//     // Send the sums to FreeRTOS queue where main task will pick them up
//     // and further calcualte decibel values (division, logarithms, etc...)
//     // xQueueSend(samples_queue, &samples, portMAX_DELAY);
//   }
// }

// //
// // Setup and main loop 
// //
// // Note: Use doubles, not floats, here unless you want to pin
// //       the task to whichever core it happens to run on at the moment
// // 
// void setup() {

//   // If needed, now you can actually lower the CPU frquency,
//   // i.e. if you want to (slightly) reduce ESP32 power consumption

//   // setCpuFrequencyMhz(80); // It should run as low as 80MHz
  
//   Serial.begin(112500);
//   delay(1000); // Safety
  
//   // Create FreeRTOS queue
//   samples_queue = xQueueCreate(8, sizeof(uint32_t));
  
//   // Create the I2S reader FreeRTOS task
//   // NOTE: Current version of ESP-IDF will pin the task 
//   //       automatically to the first core it happens to run on
//   //       (due to using the hardware FPU instructions).
//   //       For manual control see: xTaskCreatePinnedToCore
//   xTaskCreate(mic_i2s_reader_task, "Mic I2S Reader", I2S_TASK_STACK, NULL, I2S_TASK_PRI, NULL);

// uint32_t *int_sample;
// //   sum_queue_t q;
// //   uint32_t Leq_samples = 0;
// //   double Leq_sum_sqr = 0;
// //   double Leq_dB = 0;

//   // Read sum of samaples, calculated by 'i2s_reader_task'
//   while (xQueueReceive(samples_queue, &int_sample, portMAX_DELAY)) {
//     /* OPTION 2: process samples here */
//   }
// }