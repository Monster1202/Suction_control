//#pragma once
#ifndef __PARA_LIST_H__
#define __PARA_LIST_H__

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#ifdef __cplusplus
extern "C" {
#endif
// #define MQTT_PRIO 5
// #define PRINTF_LEVEL ESP_LOG_DEBUG

#define DEVICE_TYPE_SUCTION_PUMP 
//#define DEVICE_TYPE_BRUSH 
//#define DEVICE_TYPE_BLISTER
//#define DEVICE_TYPE_REMOTE 

//#define GPIOTEST
#define GPIOWORKING



// #define BACKUP_MQTT_BROKER_URL     "mqtt://172.16.171.97"//"mqtt://10.42.0.1"  //"mqtt://broker.emqx.io"  
// #define BACKUP_EXAMPLE_ESP_WIFI_SSID      "SHKJ2020"//  "CLEANING-SYSTEM"  //
// #define BACKUP_EXAMPLE_ESP_WIFI_PASS      "shkj1234."//  "12345678"    //
#define BACKUP_MQTT_BROKER_URL     "mqtt://10.42.0.1"    //"mqtt://172.16.170.48"//
#define BACKUP_EXAMPLE_ESP_WIFI_SSID      "CLEANING-SYSTEM"   //"SHKJ2020"//
#define BACKUP_EXAMPLE_ESP_WIFI_PASS      "12345678"    //"shkj1234."//
#define BACKUP_EXAMPLE_ESP_WIFI_AP_BSSID "xxxxx"
typedef struct
{
    char wifi_ssid[32];
    char wifi_pass[64];
    char broker_url[60];
    char update_url[60];
    uint8_t wifi_bssid_set;
    char wifi_bssid[6];
//    char tail[6];
}PARAMETER_CONNECTION;

#define GPIO_IO_DS18B20      4//9
#define I2C_MASTER_SCL_IO           4//1      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           5//2      /*!< GPIO number used for I2C master data  */
#define GPIO_SYS_LED         0
#define GPIO_BEEP            8

// #define CONFIG_ESP_SYSTEM_EVENT_QUEUE_SIZE 32  //32
// #define CONFIG_ESP_SYSTEM_EVENT_TASK_STACK_SIZE 2304  //2304
// #define CONFIG_ESP_MAIN_TASK_STACK_SIZE 3584//3584
/////////////////////////////////
#ifdef DEVICE_TYPE_BRUSH
    #define CONFIG_EXAMPLE_FIRMWARE_UPG_URL "http://10.42.0.245:8070/brush.bin" //"http://172.16.171.221:8070/brush.bin"

#define GPIO_OUTPUT_IO_STRETCH    14//39
#define GPIO_OUTPUT_IO_DRAW       13//40
#define GPIO_OUTPUT_IO_ROTATEX    12//41
#define GPIO_OUTPUT_IO_ROTATEY    11//42
#define GPIO_OUTPUT_IO_WATER      10//19
#define GPIO_OUTPUT_IO_BUBBLE     9//20
#define GPIO_OUTPUT_IO_7     46
#define GPIO_OUTPUT_IO_8     3

#define GPIO_OUTPUT_LED_1         18
#define GPIO_OUTPUT_LED_2         17
#define GPIO_OUTPUT_LED_3         16
#define GPIO_OUTPUT_LED_4         15
#define GPIO_OUTPUT_LED_5         7
#define GPIO_OUTPUT_LED_6         6

#define GPIO_INPUT_IO_1     35
#define GPIO_INPUT_IO_2     36
#define GPIO_INPUT_IO_3     37
#define GPIO_INPUT_IO_4     38
#define GPIO_INPUT_IO_5     39
#define GPIO_INPUT_IO_6     40
#define GPIO_INPUT_IO_7     41
#define GPIO_INPUT_IO_STOP     42
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_SYS_LED) |(1ULL<<GPIO_BEEP) |(1ULL<<GPIO_OUTPUT_IO_STRETCH) | (1ULL<<GPIO_OUTPUT_IO_DRAW)| (1ULL<<GPIO_OUTPUT_IO_ROTATEX)| (1ULL<<GPIO_OUTPUT_IO_ROTATEY)| (1ULL<<GPIO_OUTPUT_IO_WATER)| (1ULL<<GPIO_OUTPUT_IO_BUBBLE)| (1ULL<<GPIO_OUTPUT_IO_7)| (1ULL<<GPIO_OUTPUT_IO_8)| (1ULL<<GPIO_OUTPUT_LED_1)| (1ULL<<GPIO_OUTPUT_LED_2)| (1ULL<<GPIO_OUTPUT_LED_3)| (1ULL<<GPIO_OUTPUT_LED_4)| (1ULL<<GPIO_OUTPUT_LED_5)| (1ULL<<GPIO_OUTPUT_LED_6))
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_1)|(1ULL<<GPIO_INPUT_IO_2)|(1ULL<<GPIO_INPUT_IO_3)|(1ULL<<GPIO_INPUT_IO_4)|(1ULL<<GPIO_INPUT_IO_5)|(1ULL<<GPIO_INPUT_IO_6)|(1ULL<<GPIO_INPUT_IO_7)|(1ULL<<GPIO_INPUT_IO_STOP))
#else
    #ifdef DEVICE_TYPE_BLISTER
        #define CONFIG_EXAMPLE_FIRMWARE_UPG_URL "http://10.42.0.245:8070/blister.bin"

        #define GPIO_INPUT_IO_1     35
        #define GPIO_INPUT_IO_2     36
        #define GPIO_INPUT_IO_3     37
        #define GPIO_INPUT_IO_STOP     42//stop

        #define GPIO_INPUT_IO_4     38
        #define GPIO_INPUT_IO_5     39
        #define GPIO_INPUT_IO_6     40
        #define GPIO_INPUT_IO_7     41

        #define GPIO_OUTPUT_LED_1         18
        #define GPIO_OUTPUT_LED_2         17
        #define GPIO_OUTPUT_LED_3         16

        #define GPIO_OUTPUT_LED_4         15
        #define GPIO_OUTPUT_LED_5         7
        #define GPIO_OUTPUT_LED_6         6
        #define GPIO_SYS_LED         0    //stop
        #define GPIO_IO_FTC533       21//6   PWR_CTRL_2

        // #define GPIO_OUTPUT_IO_HEATER    14    //heater power
        // #define GPIO_OUTPUT_IO_WATER     13    //high pressure pump power
        // #define GPIO_OUTPUT_IO_BUBBLE    12    //diaphragm pump
        // #define GPIO_OUTPUT_IO_PUMP      11    // air pump
        // #define GPIO_OUTPUT_IO_5      10//19
        // #define GPIO_OUTPUT_IO_6     9//20
        // #define GPIO_OUTPUT_IO_7     46
        // #define GPIO_OUTPUT_IO_8     3
        // #define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_IO_FTC533) |(1ULL<<GPIO_OUTPUT_IO_PUMP) |(1ULL<<GPIO_OUTPUT_IO_HEATER) |(1ULL<<GPIO_SYS_LED) |(1ULL<<GPIO_BEEP) | (1ULL<<GPIO_OUTPUT_IO_WATER)| (1ULL<<GPIO_OUTPUT_IO_BUBBLE)| (1ULL<<GPIO_OUTPUT_LED_1)| (1ULL<<GPIO_OUTPUT_LED_2)| (1ULL<<GPIO_OUTPUT_LED_3)| (1ULL<<GPIO_OUTPUT_LED_4)| (1ULL<<GPIO_OUTPUT_LED_5)| (1ULL<<GPIO_OUTPUT_LED_6)|(1ULL<<GPIO_OUTPUT_IO_5)|(1ULL<<GPIO_OUTPUT_IO_6)|(1ULL<<GPIO_OUTPUT_IO_7)|(1ULL<<GPIO_OUTPUT_IO_8))  
        #define GPIO_OUTPUT_IO_W_SWITCH    14    //heater power
        #define GPIO_OUTPUT_IO_P_SWITCH     13    //high pressure pump power
        #define GPIO_OUTPUT_IO_BUBBLE    12    //diaphragm pump
        #define GPIO_OUTPUT_IO_4      11    // air pump
        #define GPIO_OUTPUT_IO_5      10//19
        #define GPIO_OUTPUT_IO_PUMP     9//20
        #define GPIO_OUTPUT_IO_WATER     46
        #define GPIO_OUTPUT_IO_HEATER     3
        #define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_IO_FTC533) |(1ULL<<GPIO_OUTPUT_IO_PUMP) |(1ULL<<GPIO_OUTPUT_IO_HEATER) |(1ULL<<GPIO_SYS_LED) |(1ULL<<GPIO_BEEP) | (1ULL<<GPIO_OUTPUT_IO_WATER)| (1ULL<<GPIO_OUTPUT_IO_BUBBLE)| (1ULL<<GPIO_OUTPUT_LED_1)| (1ULL<<GPIO_OUTPUT_LED_2)| (1ULL<<GPIO_OUTPUT_LED_3)| (1ULL<<GPIO_OUTPUT_LED_4)| (1ULL<<GPIO_OUTPUT_LED_5)| (1ULL<<GPIO_OUTPUT_LED_6)|(1ULL<<GPIO_OUTPUT_IO_5)|(1ULL<<GPIO_OUTPUT_IO_4)|(1ULL<<GPIO_OUTPUT_IO_W_SWITCH)|(1ULL<<GPIO_OUTPUT_IO_P_SWITCH))  
        #define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_1)|(1ULL<<GPIO_INPUT_IO_2)|(1ULL<<GPIO_INPUT_IO_3)|(1ULL<<GPIO_INPUT_IO_4)|(1ULL<<GPIO_INPUT_IO_5)|(1ULL<<GPIO_INPUT_IO_6)|(1ULL<<GPIO_INPUT_IO_7)|(1ULL<<GPIO_INPUT_IO_STOP))
    #else
        #ifdef DEVICE_TYPE_REMOTE
        #define CONFIG_EXAMPLE_FIRMWARE_UPG_URL "http://10.42.0.245:8070/remote.bin"

        #define GPIO_INPUT_IO_1     35
        #define GPIO_INPUT_IO_2     36
        #define GPIO_INPUT_IO_3     37
        #define GPIO_INPUT_IO_4     38
        #define GPIO_INPUT_IO_5     39
        #define GPIO_INPUT_IO_6     40
        #define GPIO_INPUT_IO_7     41
        #define GPIO_INPUT_IO_STOP     42//stop
        #define GPIO_OUTPUT_LED_1         18
        #define GPIO_OUTPUT_LED_2         17
        #define GPIO_OUTPUT_LED_3         16
        #define GPIO_OUTPUT_LED_4         15
        #define GPIO_OUTPUT_LED_5         7
        #define GPIO_OUTPUT_LED_6         6
        #define GPIO_SYS_LED         0    //stop
        #define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_SYS_LED) |(1ULL<<GPIO_BEEP) | (1ULL<<GPIO_OUTPUT_LED_1)| (1ULL<<GPIO_OUTPUT_LED_2)| (1ULL<<GPIO_OUTPUT_LED_3)| (1ULL<<GPIO_OUTPUT_LED_4)| (1ULL<<GPIO_OUTPUT_LED_5)| (1ULL<<GPIO_OUTPUT_LED_6))  
        #define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_1)|(1ULL<<GPIO_INPUT_IO_2)|(1ULL<<GPIO_INPUT_IO_3)|(1ULL<<GPIO_INPUT_IO_4)|(1ULL<<GPIO_INPUT_IO_5)|(1ULL<<GPIO_INPUT_IO_6)|(1ULL<<GPIO_INPUT_IO_7)|(1ULL<<GPIO_INPUT_IO_STOP))
        #define mqtt_test
        #else
            #ifdef DEVICE_TYPE_SUCTION_PUMP
            #define CONFIG_EXAMPLE_FIRMWARE_UPG_URL "http://10.42.0.245:8070/suction.bin"
            #define GPIO_OUTPUT_1         32
            #define GPIO_OUTPUT_2         33
            #define GPIO_OUTPUT_3         25
            #define GPIO_OUTPUT_4         26
            #define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_1)| (1ULL<<GPIO_OUTPUT_2)| (1ULL<<GPIO_OUTPUT_3)| (1ULL<<GPIO_OUTPUT_4))  
            #endif
        #endif
    #endif
#endif

#define MQTT_BROKER_URL     "mqtt://10.42.0.1"
#define EXAMPLE_ESP_WIFI_SSID      "CLEANING-SYSTEM"
#define EXAMPLE_ESP_WIFI_PASS      "12345678"
//#define HOTPOT_MODE
//#define TEST_MODE
// #ifdef HOTPOT_MODE
// #define MQTT_BROKER_URL     "mqtt://10.42.0.1"
// #define EXAMPLE_ESP_WIFI_SSID      "CLEANING-SYSTEM"
// #define EXAMPLE_ESP_WIFI_PASS      "12345678"
// #else
//     #ifdef TEST_MODE
//     #define MQTT_BROKER_URL     "mqtt://broker.emqx.io" //"mqtt://172.16.171.97"   //"mqtt://10.42.0.1"    "mqtt://172.16.161.171"
//     #define EXAMPLE_ESP_WIFI_SSID      "SHKJ2020"//CONFIG_ESP_WIFI_SSID   "CLEANING-SYSTEM"  "yyg"//
//     #define EXAMPLE_ESP_WIFI_PASS      "shkj1234."//CONFIG_ESP_WIFI_PASSWORD   "12345678"//
//     #else
//     #define MQTT_BROKER_URL     "mqtt://172.16.171.97"   //"mqtt://10.42.0.1"    "mqtt://172.16.161.171"
//     #define EXAMPLE_ESP_WIFI_SSID      "SHKJ2020"//CONFIG_ESP_WIFI_SSID   "CLEANING-SYSTEM"  "yyg"//
//     #define EXAMPLE_ESP_WIFI_PASS      "shkj1234."//CONFIG_ESP_WIFI_PASSWORD   "12345678"//
//     #endif
// #endif


/////////////////////////////////////////////
uint16_t parameter_read_pressure(void);
void parameter_write_pressure(uint16_t pressure);
void parameter_write_temperature(double temperature);
double parameter_read_temperature(void);
void para_init(void);
esp_err_t get_chip_id(uint32_t* chip_id);

void parameter_write_centralizer(uint8_t value);
uint8_t parameter_read_centralizer(void);
void parameter_write_rotation(uint8_t value);
uint8_t parameter_read_rotation(void);
void parameter_write_nozzle(uint8_t value);
uint8_t parameter_read_nozzle(void);
void parameter_write_emergency_stop(uint8_t value);
uint8_t parameter_read_emergency_stop(void);

void parameter_write_msg_id(char *str_msgid);
char *parameter_read_msg_id(void);
void parameter_write_timestamp(double timestamp);
double parameter_read_timestamp(void);
void parameter_write_water(uint8_t value);
uint8_t parameter_read_water(void);
void parameter_write_pressure_alarm(uint8_t value);
uint8_t parameter_read_pressure_alarm(void);

void parameter_write_heater(uint8_t value);
uint8_t parameter_read_heater(void);
void parameter_write_mode(uint8_t value);
uint8_t parameter_read_mode(void);
double blister_read_temperature(void);
void parameter_write_liquid_alarm(uint8_t value);

typedef struct
{
    char uuid[6];//uint32_t uuid;
    uint8_t nozzle;      //command down
    uint8_t centralizer;
    uint8_t rotation;
    uint8_t status;     //status upload
    uint8_t water;
    uint8_t pressure_alarm;
    uint8_t emergency_stop;
    double timestamp;
    char msg_id[30];
    double temperature;
    uint16_t pressure;
    char version[30];
    int8_t rssi;
    uint8_t wifi_connection;    //0\1  wifi_con  2\3 mqtt_con
    uint8_t air_pump;
    int time_int;
    char time_string[20];
}PARAMETER_BRUSH;

typedef struct
{
    char uuid[6];//uint32_t uuid;
    uint8_t mode;      //command down
    uint8_t heater;    
    uint8_t status;     //status upload
    uint8_t water;
    uint8_t pressure_alarm;
    uint8_t liquid_alarm;
    uint8_t emergency_stop;
    double timestamp;
    char msg_id[30];
    double temperature;
    uint16_t pressure;
    char version[30];
    int8_t rssi;
    uint8_t wifi_connection; 
    uint8_t air_pump;
    int time_int;
    char time_string[20];
}PARAMETER_BLISTER;

typedef struct
{
    char uuid[6];//uint32_t uuid;
    uint8_t status;     //status upload
    double timestamp;
    char msg_id[30];
    uint8_t nozzle;      //command down
    uint8_t centralizer;
    uint8_t rotation;
    uint8_t heater;
    uint8_t mode;
    uint8_t angle;
    uint8_t emergency_stop;
    char version[30];
    int8_t rssi;
    uint8_t wifi_connection; 
    uint8_t air_pump;
    int time_int;
    char time_string[20];
}PARAMETER_REMOTE;

typedef struct
{
    uint8_t status;     //status upload
    int8_t rssi;
    uint8_t wifi_connection; 
    uint8_t suction[4];
    int time_int;
    double timestamp;
    char uuid[6];//uint32_t uuid;
    char msg_id[30];
    char version[30];
    char time_string[20];
}PARAMETER_SUCTION_PUMP;

void parameter_write_version(char *str_version);
void get_parameter(PARAMETER_BRUSH *brush_t);
void get_blister_parameter(PARAMETER_BLISTER *blister_t);
void get_remote_parameter(PARAMETER_REMOTE *remote_t);
void parameter_write_rssi(int8_t value);

void parameter_write_wifi_connection(uint8_t value);
uint8_t parameter_read_wifi_connection(void);

void parameter_write_wifi_ssid(char *str_para);

char *parameter_read_wifi_ssid(void);

void parameter_write_wifi_pass(char *str_para);

char *parameter_read_wifi_pass(void);

void parameter_write_broker_url(char *str_para);
char *parameter_read_broker_url(void);

void parameter_write_update_url(char *str_para);
char *parameter_read_update_url(void);
int8_t flash_write_parameter(void);
int8_t flash_read_parameter(void);
void wifi_url_inital_set_para(void);
void wifi1_url_inital_set_para(void);
int8_t flash_erase_parameter(void);


void parameter_write_FTC533(uint8_t value);
uint8_t parameter_read_FTC533(void);
//uint8_t FTC533_KEY_press;
void parameter_write_debug(uint32_t value);
uint32_t parameter_read_debug(void);

void parameter_write_air_pump(uint8_t value);
uint8_t parameter_read_air_pump(void);

void parameter_write_twai_status(uint8_t value);
uint8_t parameter_read_twai_status(void);

//void parameter_write_remote_xyz(char *speed_x,char *speed_y,char *speed_z);
void parameter_write_remote_xyz(double speed_x,double speed_y,double speed_z,double speed_axes3);
//void parameter_read_remote_xyz(double speed_x,double speed_y,double speed_z);
void parameter_write_robot_h_v(uint8_t horizontal,uint8_t vertical,uint8_t servo,uint8_t video,uint8_t bakup);


void parameter_write_wifi_bssid(uint8_t str_para[]);
char *parameter_read_wifi_bssid(void);
void parameter_write_wifi_bssid_set(uint8_t b1);
uint8_t parameter_read_wifi_bssid_set(void);

void parameter_write_vehicle_battery(uint16_t bat);
uint16_t parameter_read_vehicle_battery(void);

void parameter_write_suction_para(uint8_t str_para[]);
char *parameter_read_suction_para(void);

typedef struct times
{
	int Year;
	int Mon;
	int Day;
	int Hour;
	int Min;
	int Second;
}Times;
//Times stamp_to_standard(int stampTime);
Times stamp_to_standard(int stampTime,char *time_s);
typedef struct
{
    uint16_t Year;
    uint16_t Mon;
    uint16_t Day;
    uint16_t Hour;
    uint16_t Min;
    uint16_t Sec;
}ST_DATE_TIME,*PST_DATE_TIME;

uint32_t DateTime2Stamp(PST_DATE_TIME pstDateTime);
ST_DATE_TIME TimeStamp2DateTime(uint32_t Stamp);


char *parameter_read_time_string(void);
void get_suction_pump_parameter(PARAMETER_SUCTION_PUMP *rsuction_pump_t);

#ifdef __cplusplus
}
#endif

#endif

