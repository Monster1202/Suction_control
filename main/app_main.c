/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "cJSON.h"
#include "para_list.h"
#include "ds18b20.h"
#include "pressure_i2c.h"
#include "wifi_sta.h"
#include "ota_app.h"
#include "mqtt_app.h"
#include "gpio_ctrl.h"
#include "led_strip.h"
#include "timer_app.h"
#include "uart485.h"
#include "esp_partition.h"
#include "esp_err.h"
#include "twai_app.h"

void flashwrite_reset(void);
int8_t test_custom_partition();
// void log_write_send(const char *format,...);
// void log_read_send(const char *format,...);

static const char *TAG = "main";
void airpump_process(void);

void print_heapsize(void) 
{
    printf("esp_get_free_heap_size = %d\n", esp_get_free_heap_size());
    printf("esp_get_free_internal_heap_size = %d\n", esp_get_free_internal_heap_size());
    printf("esp_get_minimum_free_heap_size = %d\n", esp_get_minimum_free_heap_size());
}
void app_main(void)
{
    print_heapsize();
    spiff_init();
//parameter_init
    para_init();
//gpio task in/out     PRIO 10 
    gpio_init();
//wifi connect STA    configMAX_PRIORITIES -5                  ( 5 )
    wifi_connect();     
//wifi_scan
    xTaskCreate(wifi_scan, "wifi_scan", 4096, NULL, 6, NULL);
//OTA enable   get version
    native_ota_app();
//MQTT enable     MQTT task priority, default is 5,
    mqtt_init();
    
//    xTaskCreate(airpump_process, "airpump_process", 1024, NULL, 8, NULL);
    xTaskCreate(log_process, "log_process", 4096, NULL, 3, NULL);
//uart read/write example without event queue;
    //xTaskCreate(ota_debug_process, "ota_debug_process", 4096, NULL, 12, NULL);
// #ifdef DEVICE_TYPE_BLISTER
// //pressure_read
//     timer_FTC533();
//     xTaskCreate(heater_init_process, "heater_init_process", 4096, NULL, 7, NULL);
//     //heater_init(1);
//     //timer_heater_init();
// #endif

// // #ifndef DEVICE_TYPE_REMOTE
// #ifdef GPIOTEST 
//     xTaskCreate(uart485_task, "uart485_task", 4096, NULL, 10, NULL);
//     xTaskCreate(uart232_task, "uart232_task", 4096, NULL, 11, NULL);
//     xTaskCreate(pressure_read, "pressure_read", 4096, NULL, 2, NULL);
// #endif
// #ifdef DEVICE_TYPE_BRUSH  
//     twai_init();
// #endif
// // //DS18B20 task
// //     //xTaskCreate(ds18b20_read, "ds18b20_read", 4096, NULL, 24, NULL);///////23 OK  22 2% ERROR
// // #endif
// #ifdef mqtt_test
//     xTaskCreate(mqtt_gpio_test, "mqtt_gpio_test", 4096, NULL, 13, NULL);
// #endif
    timer_periodic();  //init end beep 
    // printf("configMAX_PRIORITIES:%d",configMAX_PRIORITIES);
    // printf("CONFIG_ESP_SYSTEM_EVENT_QUEUE_SIZE:%d",CONFIG_ESP_SYSTEM_EVENT_QUEUE_SIZE);
    // printf("CONFIG_ESP_SYSTEM_EVENT_TASK_STACK_SIZE:%d",CONFIG_ESP_SYSTEM_EVENT_TASK_STACK_SIZE);
    // printf("CONFIG_ESP_MAIN_TASK_STACK_SIZE:%d",CONFIG_ESP_MAIN_TASK_STACK_SIZE);
    //gpio_set_level(23, 1);
    uint8_t s_led_state = 0;
    uint8_t wifi_sta = 0;
    uint32_t time_cnt = 0;
    uint8_t nozzle_mode = 0;
    uint8_t air_pressure = 0;
    uint8_t blister_led_state = 0;
    uint8_t blister_emergency_state = 0;
    while(1) {
        //time_cnt = parameter_read_debug();  
        vTaskDelay(200 / portTICK_RATE_MS);
        wifi_sta=parameter_read_wifi_connection();
        time_cnt++;
        // if(wifi_sta>=1){
        //     if(time_cnt % (5-wifi_sta) == 1){
        //         s_led_state = !s_led_state;
        //         gpio_set_level(GPIO_SYS_LED, s_led_state);
        //     }
        // }
        if(time_cnt%100==10){  
            print_heapsize();
        }

        //printf("wifi_sta: %d\n", wifi_sta);

        //test_custom_partition(); 
        //printf("parameter_read_FTC533:cnt: %d\n", cnt);
    }
}



void airpump_process(void)
{
    int cnt = 0;
    uint8_t nozzle_state = 0;
    uint8_t air_pump_state = 0;
    while(1){
        vTaskDelay(200 / portTICK_RATE_MS);
        #ifdef DEVICE_TYPE_BRUSH
        nozzle_state = parameter_read_nozzle();
        air_pump_state = parameter_read_air_pump();
        if(nozzle_state == 2){
            cnt++;
            if(cnt >= 25 && cnt <= 35){
                gpio_set_level(GPIO_OUTPUT_IO_WATER, 0);   //change I/O to pump
                //gpio_set_level(GPIO_OUTPUT_IO_7, 1);
                //gpio_set_level(GPIO_OUTPUT_LED_5, 1);               
                //cnt = 0;
            }           
        }
        else{
            //if(air_pump_state != 1){
                //gpio_set_level(GPIO_OUTPUT_LED_5, 0);
                //gpio_set_level(GPIO_OUTPUT_IO_7, 0);
                gpio_set_level(GPIO_OUTPUT_IO_WATER, 1);
            //}
            cnt = 0;
        }
        #endif

        #ifdef DEVICE_TYPE_BLISTER
        nozzle_state = parameter_read_mode();
        air_pump_state = parameter_read_air_pump();
        if(nozzle_state == 2){
            cnt++;
            if(cnt >= 25 && cnt <= 35){
                gpio_set_level(GPIO_OUTPUT_IO_PUMP, 1);
                //gpio_set_level(GPIO_OUTPUT_LED_4, 1);               
                //cnt = 0;
            }           
        } 
        else{
            if(air_pump_state != 1){
            gpio_set_level(GPIO_OUTPUT_IO_PUMP, 0);
            //gpio_set_level(GPIO_OUTPUT_LED_4, 0);
            }
            cnt = 0;
        }
        #endif
    }
}

int8_t test_custom_partition()
{
    const char* data = "Test read and write partition";
    uint8_t dest_data[1024] = {0};
    const esp_partition_t *find_partition = NULL;
    find_partition = esp_partition_find_first(0x40, 0x0, NULL);
    if(find_partition == NULL){
	    printf("No partition found!\r\n");
	    return -1;
    }

    printf("Erase custom partition\r\n");
    if (esp_partition_erase_range(find_partition, 0, 0x1000) != ESP_OK) {
	    printf("Erase partition error");
	    return -1;
    }

    printf("Write data to custom partition\r\n");
    if (esp_partition_write(find_partition, 0, data, strlen(data) + 1) != ESP_OK) {   // incude '\0'
	    printf("Write partition data error");
	    return -1;
    }

    printf("Read data from custom partition\r\n");
    if (esp_partition_read(find_partition, 0, dest_data, 1024) != ESP_OK) {
	    printf("Read partition data error");
	    return -1;
    }

    printf("Receive data: %s\r\n", (char*)dest_data);

    return 0;
}

// void led_state(void)
// {
// // #define BLINK_GPIO 48
// // #define CONFIG_BLINK_LED_RMT_CHANNEL 0
// //static led_strip_t *pStrip_a;
// // static void blink_led(uint8_t s_led_state);
// // static void configure_led(void);
// // static const char *TAG = "led_strip";

//     //int cnt = 0;
//     //configure_led();
//     //uint8_t s_led_state = 0;
//             // printf("cnt: %d\n", cnt++);
//         // ESP_LOGI(TAG, "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");
//         //blink_led(s_led_state);
//         // /* Toggle the LED state */
//         //s_led_state = !s_led_state;
//         //gpio_set_level(GPIO_SYS_LED, s_led_state);
//         //gpio_set_level(GPIO_BEEP, s_led_state);
// }

 
// static void blink_led(uint8_t s_led_state)
// {
//     /* If the addressable LED is enabled */
//     if (s_led_state) {
//         /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
//         pStrip_a->set_pixel(pStrip_a, 0, 16, 16, 16);
//         /* Refresh the strip to send data */
//         pStrip_a->refresh(pStrip_a, 100);
//     } else {
//         /* Set all LED off to clear all pixels */
//         pStrip_a->clear(pStrip_a, 50);
//     }
// }

// static void configure_led(void)
// {
//     ESP_LOGI(TAG, "Example configured to blink addressable LED!");
//     /* LED strip initialization with the GPIO and pixels number*/
//     pStrip_a = led_strip_init(CONFIG_BLINK_LED_RMT_CHANNEL, BLINK_GPIO, 1);
//     /* Set all LED off to clear all pixels */
//     pStrip_a->clear(pStrip_a, 50);
// }


// void wifi_mqtt_start(void)
// {
//     ESP_LOGI(TAG, "[APP] Startup..");
//     ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
//     ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

//     esp_log_level_set("*", ESP_LOG_INFO);//  CONFIG_LOG_COLORS
//     esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
//     esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_INFO);////ESP_LOG_DEBUG ESP_LOG_INFO ESP_LOG_WARN
//     esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
//     esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
//     esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
//     esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

//     ESP_ERROR_CHECK(nvs_flash_init());
//     ESP_ERROR_CHECK(esp_netif_init());
//     ESP_ERROR_CHECK(esp_event_loop_create_default());

//     /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
//      * Read "Establishing Wi-Fi or Ethernet Connection" section in
//      * examples/protocols/README.md for more information about this function.
//      */
//     ESP_ERROR_CHECK(example_connect());
//     // esp_err_t ret = nvs_flash_init();
//     // if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
//     //   ESP_ERROR_CHECK(nvs_flash_erase());
//     //   ret = nvs_flash_init();
//     // }
//     // ESP_ERROR_CHECK(ret);

//     // ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
//      //wifi_init_sta();

// //   WiFi.begin(ssid,psw);
// //   while(WiFi.status()!=WL_CONNECTED){      //未连接上
// //     delay(500);
// //     Serial.println("connect to wifi...");
// //   }
// //   Serial.println("successfully connected");  
//     mqtt_app_start();
// }