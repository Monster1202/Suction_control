#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "mqtt_app.h"
#include "driver/gpio.h"
#include "cJSON.h"
#include "para_list.h"
#include "gpio_ctrl.h"
#include "wifi_sta.h"
#include "esp_spiffs.h"

#define TOPIC_TIMESTAMP "/timestamp"
#define TOPIC_EMERGENCY_CONTROL "/emergency-control"
#define TOPIC_DEVICE_REGISTER "/device-register"

#define TOPIC_BRUSH_CONTROL "/pneumatic-brush-device/switch-control"
#define TOPIC_BRUSH_STATES "/pneumatic-brush-device/states"
#define TOPIC_BLISTER_CONTROL "/blister-device/switch-control"
#define TOPIC_BLISTER_STATES "/blister-device/states"
#define TOPIC_REMOTE_CONTROL "/remote-control-device/switch-control"
#define TOPIC_REMOTE_STATES "/remote-control-device/states"
#define TOPIC_LOG_ESP32 "/log_esp32"

#define LOG_FILE_PATH "/spiffs/log3.txt"
//PARAMETER_BRUSH brush_para;

//char data_pub_1[300] = {0};
static const char *TAG = "MQTT_EXAMPLE";
esp_mqtt_client_handle_t mqtt_client;
static uint16_t buf_disconnect = 0;

uint8_t flag_write_para = 0;
uint8_t flag_log = 0;
size_t total = 0, used = 0;
static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

void flashwrite_reset(void)
{
    if(flash_write_parameter() == -1)
        ESP_LOGI(TAG, "flash_write_parameter_error!");
    if(flag_write_para == 0x01 || flag_write_para == 0x03){
        wifi_reset();
    }
    if(flag_write_para == 0x02 || flag_write_para == 0x03){
        vTaskDelay(1000 / portTICK_RATE_MS);
        mqtt_reset();
    }
    flag_write_para = 0;
}


static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
     
    char data_pub_1[300] = "init";
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        parameter_write_wifi_connection(3);
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    #ifndef DEVICE_TYPE_REMOTE   
        msg_id = esp_mqtt_client_subscribe(client, TOPIC_EMERGENCY_CONTROL, 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
    #endif
        msg_id = esp_mqtt_client_subscribe(client, TOPIC_TIMESTAMP, 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        
    #ifdef DEVICE_TYPE_BRUSH
        data_publish(data_pub_1,0);   //device_register
        msg_id = esp_mqtt_client_publish(client, TOPIC_DEVICE_REGISTER, data_pub_1, 0, 1, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, TOPIC_BRUSH_CONTROL, 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        data_publish(data_pub_1,1); 
        msg_id = esp_mqtt_client_publish(client, TOPIC_BRUSH_STATES, data_pub_1, 0, 1, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
    #else
        #ifdef DEVICE_TYPE_BLISTER
            data_publish(data_pub_1,2);   //device_register
            msg_id = esp_mqtt_client_publish(client, TOPIC_DEVICE_REGISTER, data_pub_1, 0, 1, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, TOPIC_BLISTER_CONTROL, 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            data_publish(data_pub_1,3); 
            msg_id = esp_mqtt_client_publish(client, TOPIC_BLISTER_STATES, data_pub_1, 0, 1, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        #else
            data_publish(data_pub_1,4);   //device_register
            msg_id = esp_mqtt_client_publish(client, TOPIC_DEVICE_REGISTER, data_pub_1, 0, 1, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, TOPIC_BRUSH_STATES, 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
            // msg_id = esp_mqtt_client_subscribe(client, TOPIC_BLISTER_STATES, 0);
            // ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        #endif
    #endif
        // msg_id = esp_mqtt_client_subscribe(client, topic_pub_1, 1);
        // ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        // msg_id = esp_mqtt_client_unsubscribe(client, topic_pub_1);
        // ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        parameter_write_wifi_connection(2);
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED buf_disconnect=%d",buf_disconnect);
        #ifndef DEVICE_TYPE_BLISTER  
        buf_disconnect++;            
        if(buf_disconnect == 15)   //10=1minute
            esp_restart();
        #endif   
        // uint8_t wifi_sta = 0;
        // wifi_sta=parameter_read_wifi_connection();
        // ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED wifi_sta=%d",wifi_sta);
        // if(wifi_sta)//wifi connected 
        //     esp_mqtt_client_reconnect(mqtt_client);
        // if(buf_disconnect == 10)
        // {
        //     wifi_reset();
        //     mqtt_reset();
        //     buf_disconnect = 0;
        // }    
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        // msg_id = esp_mqtt_client_publish(client, topic_sub_1, "SUBSCRIBED", 0, 0, 0);
        // ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        //ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        ESP_LOGI(TAG, "TOPIC=%.*s\r", event->topic_len, event->topic);
        ESP_LOGI(TAG, "DATA=%.*s\r", event->data_len, event->data);
        data_process(event->data);
#ifdef DEVICE_TYPE_BRUSH
        data_publish(data_pub_1,1); 
        msg_id = esp_mqtt_client_publish(client, TOPIC_BRUSH_STATES, data_pub_1, 0, 1, 0);
        //ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
#endif  
#ifdef DEVICE_TYPE_BLISTER      
        data_publish(data_pub_1,3); 
        msg_id = esp_mqtt_client_publish(client, TOPIC_BLISTER_STATES, data_pub_1, 0, 1, 0);
        //ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
#endif 
#ifdef DEVICE_TYPE_REMOTE      
        data_publish(data_pub_1,9); 
        msg_id = esp_mqtt_client_publish(client, TOPIC_REMOTE_STATES, data_pub_1, 0, 1, 0);
        //ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
#endif 
        buf_disconnect = 0;
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
    if(flag_write_para)
        flashwrite_reset();
}

void mqtt_app_start(void)
{
    char *broker_url = {0};
    broker_url = parameter_read_broker_url();
    printf("parameter_read_broker_url:%s",broker_url); 

    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = broker_url,//MQTT_BROKER_URL,//CONFIG_BROKER_URL,
        //.task_prio = MQTT_PRIO,
    };
    //strcpy(mqtt_cfg.uri,broker_url);
    //esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    // esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    // esp_mqtt_client_start(client);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
}


void data_process(char *data)
{
    cJSON *json_str_xy = cJSON_Parse(data);
    if(json_str_xy == NULL) {
        cJSON_Delete(json_str_xy);
        return 0;
    }
#ifdef DEVICE_TYPE_BRUSH
    cJSON *json_emergency_stop = cJSON_GetObjectItem(json_str_xy, "emergency_stop");
    if(json_emergency_stop != NULL && json_emergency_stop->type == cJSON_Number) {
        //brush_para.emergency_stop = json_emergency_stop->valueint;
        ESP_LOGI(TAG, "emergency_stop = %d", json_emergency_stop->valueint);
        brush_stop_io_out(json_emergency_stop->valueint,0);
    }
    cJSON *json_switch_name = cJSON_GetObjectItem(json_str_xy, "switch_name");
    if(json_switch_name != NULL && json_switch_name->type == cJSON_String) {
        ESP_LOGI(TAG, "switch_name = %s", json_switch_name->valuestring);
        cJSON *json_value = cJSON_GetObjectItem(json_str_xy, "value");
        uint8_t register_emergency_stop = 0;
        register_emergency_stop = parameter_read_emergency_stop();
        if(json_value != NULL && json_value->type == cJSON_Number) {
            ESP_LOGI(TAG, "value = %d", json_value->valueint);
            if(strcmp(json_switch_name->valuestring,"nozzle")==0){
                if(register_emergency_stop==0)
                    nozzle_io_out(json_value->valueint);
            }
            else if(strcmp(json_switch_name->valuestring,"centralizer")==0){
                if(register_emergency_stop==0)
                    centralizer_io_out(json_value->valueint);
            }
            else if(strcmp(json_switch_name->valuestring,"rotation")==0){
                if(register_emergency_stop==0)
                    rotation_io_out(json_value->valueint);
            }
        }
    }
    // cJSON *json_timestamp = cJSON_GetObjectItem(json_str_xy, "timestamp");
    // if(json_timestamp != NULL && json_timestamp->type == cJSON_Number) {
    //     //brush_para.timestamp = json_timestamp->valuedouble;
    //     parameter_write_timestamp(json_timestamp->valuedouble);
    //     ESP_LOGI(TAG, "timestamp = %f", json_timestamp->valuedouble);
    // }
    // cJSON *json_msg_id = cJSON_GetObjectItem(json_str_xy, "msg_id");
    // if(json_msg_id != NULL && json_msg_id->type == cJSON_String) {
    //     //strcpy(brush_para.msg_id,json_msg_id->valuestring);
    //     parameter_write_msg_id(json_msg_id->valuestring);
    //     ESP_LOGI(TAG, "msg_id = %s", json_msg_id->valuestring);
    // }
#else
    #ifdef DEVICE_TYPE_BLISTER   
    cJSON *json_emergency_stop = cJSON_GetObjectItem(json_str_xy, "emergency_stop");
    if(json_emergency_stop != NULL && json_emergency_stop->type == cJSON_Number) {
        ESP_LOGI(TAG, "emergency_stop = %d", json_emergency_stop->valueint);
        //blister_stop_io_out(json_emergency_stop->valueint,0);
    }
    cJSON *json_switch_name = cJSON_GetObjectItem(json_str_xy, "switch_name");
    if(json_switch_name != NULL && json_switch_name->type == cJSON_String) {
        ESP_LOGI(TAG, "switch_name = %s", json_switch_name->valuestring);
        cJSON *json_value = cJSON_GetObjectItem(json_str_xy, "value");
        uint8_t register_emergency_stop = 0;
        register_emergency_stop = parameter_read_emergency_stop();
        if(json_value != NULL && json_value->type == cJSON_Number) {
            ESP_LOGI(TAG, "value = %d", json_value->valueint);
            if(strcmp(json_switch_name->valuestring,"heater")==0){
                if(register_emergency_stop==0)
                    heater_io_out(json_value->valueint);
            }
            else if(strcmp(json_switch_name->valuestring,"mode")==0){
                if(register_emergency_stop==0)
                    blister_mode_io_out(json_value->valueint);
            }
        }
    }
    #else
    cJSON *json_emergency_stop = cJSON_GetObjectItem(json_str_xy, "emergency_stop");
    if(json_emergency_stop != NULL && json_emergency_stop->type == cJSON_Number) {
        ESP_LOGI(TAG, "emergency_stop = %d", json_emergency_stop->valueint);
        remote_stop_io_out(json_emergency_stop->valueint,1);
    }
    uint8_t register_emergency_stop = 0;
    register_emergency_stop = parameter_read_emergency_stop();
    cJSON *json_centralizer = cJSON_GetObjectItem(json_str_xy, "centralizer");
    if(json_centralizer != NULL && json_centralizer->type == cJSON_Number) {
        ESP_LOGI(TAG, "centralizer = %d", json_centralizer->valueint);
        if(register_emergency_stop==0)
            centralizer_io_out(json_centralizer->valueint);
    }
    cJSON *json_rotation = cJSON_GetObjectItem(json_str_xy, "rotation");
    if(json_rotation != NULL && json_rotation->type == cJSON_Number) {
        ESP_LOGI(TAG, "rotation = %d", json_rotation->valueint);
        if(register_emergency_stop==0)
            rotation_io_out(json_rotation->valueint);
    }
    cJSON *json_nozzle = cJSON_GetObjectItem(json_str_xy, "nozzle");
    if(json_nozzle != NULL && json_nozzle->type == cJSON_Number) {
        ESP_LOGI(TAG, "nozzle = %d", json_nozzle->valueint);
        if(register_emergency_stop==0)
            nozzle_io_out(json_nozzle->valueint);
    }
    #endif
#endif 
    cJSON *json_timestamp = cJSON_GetObjectItem(json_str_xy, "timestamp");
    if(json_timestamp != NULL && json_timestamp->type == cJSON_Number) {
        //brush_para.timestamp = json_timestamp->valuedouble;
        parameter_write_timestamp(json_timestamp->valuedouble);
        //ESP_LOGI(TAG, "timestamp = %f", json_timestamp->valuedouble);
    }
    cJSON *json_msg_id = cJSON_GetObjectItem(json_str_xy, "msg_id");
    if(json_msg_id != NULL && json_msg_id->type == cJSON_String) {
        //strcpy(brush_para.msg_id,json_msg_id->valuestring);
        parameter_write_msg_id(json_msg_id->valuestring);
        ESP_LOGI(TAG, "msg_id = %s", json_msg_id->valuestring);
    }

    cJSON *json_wifi_ssid = cJSON_GetObjectItem(json_str_xy, "wifi_ssid");
    if(json_wifi_ssid != NULL && json_wifi_ssid->type == cJSON_String) {
        parameter_write_wifi_ssid(json_wifi_ssid->valuestring);
        ESP_LOGI(TAG, "wifi_ssid = %s", json_wifi_ssid->valuestring);
        flag_write_para = 1;
    }
    cJSON *json_wifi_pass = cJSON_GetObjectItem(json_str_xy, "wifi_pass");
    if(json_wifi_pass != NULL && json_wifi_pass->type == cJSON_String) {
        parameter_write_wifi_pass(json_wifi_pass->valuestring);
        ESP_LOGI(TAG, "wifi_pass = %s", json_wifi_pass->valuestring);
        flag_write_para = 1;
    }
    cJSON *json_broker_url = cJSON_GetObjectItem(json_str_xy, "broker_url");
    if(json_broker_url != NULL && json_broker_url->type == cJSON_String) {
        parameter_write_broker_url(json_broker_url->valuestring);
        ESP_LOGI(TAG, "broker_url = %s", json_broker_url->valuestring);
        flag_write_para += 2;
    }
    cJSON *json_update_url = cJSON_GetObjectItem(json_str_xy, "update_url");
    if(json_update_url != NULL && json_update_url->type == cJSON_String) {
        parameter_write_update_url(json_update_url->valuestring);
        ESP_LOGI(TAG, "update_url = %s", json_update_url->valuestring);
    }
    cJSON *json_debug_parameter = cJSON_GetObjectItem(json_str_xy, "debug_parameter");
    if(json_debug_parameter != NULL && json_debug_parameter->type == cJSON_Number) {
        ESP_LOGI(TAG, "debug_parameter = %d", json_debug_parameter->valueint);
        parameter_write_debug(json_debug_parameter->valueint);
    }
    cJSON *json_buffer = cJSON_GetObjectItem(json_str_xy, "erase");
    if(json_buffer != NULL && json_buffer->type == cJSON_String) {
        ESP_LOGI(TAG, "erase : %s", json_buffer->valuestring);
        if(flash_erase_parameter() == -1)
            printf("flash_erase_parameter error");
    }
    json_buffer = cJSON_GetObjectItem(json_str_xy, "log_clear");
    if(json_buffer != NULL && json_buffer->type == cJSON_Number) {
        ESP_LOGI(TAG, "log_clear : %d", json_buffer->valueint);
        log_file_clear("0");
    }
    json_buffer = cJSON_GetObjectItem(json_str_xy, "log_read");
    if(json_buffer != NULL && json_buffer->type == cJSON_Number) {
        ESP_LOGI(TAG, "log_read : %d", json_buffer->valueint);
        //log_read_send("0");
        flag_log = 1;
    }
    // if(flag_write_para)
    //     flashwrite_reset();
    cJSON_Delete(json_str_xy);
}

void hex2str(uint8_t *input, uint16_t input_len, char *output)
{
    char *hexEncode = "0123456789ABCDEF";
    int i = 0, j = 0;

    for (i = 0; i < input_len; i++) {
        output[j++] = hexEncode[(input[i] >> 4) & 0xf];
        output[j++] = hexEncode[(input[i]) & 0xf];
    }
}

void device_states_publish(uint8_t button)
{
    int msg_id = 0;
    char data_pub_1[300] = "init";
#ifdef DEVICE_TYPE_BRUSH
    data_publish(data_pub_1,1);   
    msg_id = esp_mqtt_client_publish(mqtt_client, TOPIC_BRUSH_STATES, data_pub_1, 0, 1, 0);
#endif
#ifdef DEVICE_TYPE_BLISTER
    data_publish(data_pub_1,3);   
    msg_id = esp_mqtt_client_publish(mqtt_client, TOPIC_BLISTER_STATES, data_pub_1, 0, 1, 0);
#endif
#ifdef DEVICE_TYPE_REMOTE
    switch(button)
    {
        case 1:data_publish(data_pub_1,5);
        msg_id = esp_mqtt_client_publish(mqtt_client, TOPIC_BRUSH_CONTROL, data_pub_1, 0, 1, 0);break;
        case 2:data_publish(data_pub_1,6);
        msg_id = esp_mqtt_client_publish(mqtt_client, TOPIC_BRUSH_CONTROL, data_pub_1, 0, 1, 0);break;
        case 3:data_publish(data_pub_1,7);
        msg_id = esp_mqtt_client_publish(mqtt_client, TOPIC_BRUSH_CONTROL, data_pub_1, 0, 1, 0);break;
        case 4:data_publish(data_pub_1,8);
        msg_id = esp_mqtt_client_publish(mqtt_client, TOPIC_EMERGENCY_CONTROL, data_pub_1, 0, 1, 0);break;
        default:break;
    }   
#endif    
    // if(button == 20){
    //     data_publish(data_pub_1,20);   //case 20 log
    //     msg_id = esp_mqtt_client_publish(mqtt_client, TOPIC_LOG_ESP32, data_pub_1, 0, 1, 0);
    // }
    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
}



void data_publish(char *data,uint8_t case_pub)
{
    PARAMETER_BRUSH brush_buf = {0};
    get_parameter(&brush_buf);

    PARAMETER_BLISTER blister_buf = {0};
    get_blister_parameter(&blister_buf);

    PARAMETER_REMOTE remote_buf = {0};
    get_remote_parameter(&remote_buf);
    
    cJSON*root = cJSON_CreateObject();
    if(case_pub == 0){
        char string_uuid[20] = "0";//7cdfa1e592e0
        hex2str((uint8_t *)brush_buf.uuid,6,string_uuid);
        cJSON_AddItemToObject(root, "device_sn",cJSON_CreateString(string_uuid));
        //cJSON_AddNumberToObject(root, "device_sn",brush_buf.uuid);
        cJSON_AddNumberToObject(root, "timestamp",brush_buf.timestamp);
        cJSON_AddItemToObject(root, "device_type",cJSON_CreateString("PNEUMATIC_BRUSH"));
        cJSON_AddItemToObject(root, "device_version",cJSON_CreateString(brush_buf.version));
        }
    else if(case_pub == 1){
        cJSON_AddNumberToObject(root, "status",brush_buf.status);
        cJSON_AddNumberToObject(root, "water",brush_buf.water);
        cJSON_AddNumberToObject(root, "pressure_alarm",brush_buf.pressure_alarm);
        cJSON_AddNumberToObject(root, "nozzle",brush_buf.nozzle);
        cJSON_AddNumberToObject(root, "centralizer",brush_buf.centralizer);
        cJSON_AddNumberToObject(root, "rotation",brush_buf.rotation);
        cJSON_AddNumberToObject(root, "emergency_stop",brush_buf.emergency_stop);  
        cJSON_AddNumberToObject(root, "temperature",brush_buf.temperature); 
        cJSON_AddNumberToObject(root, "rssi",brush_buf.rssi);           
        cJSON_AddNumberToObject(root, "timestamp",brush_buf.timestamp);
        cJSON_AddItemToObject(root, "msg_id",cJSON_CreateString(brush_buf.msg_id)); 
        }
    else if(case_pub == 2){
        char string_uuid[20] = "0";//7cdfa1e592e0
        hex2str((uint8_t *)blister_buf.uuid,6,string_uuid);
        cJSON_AddItemToObject(root, "device_sn",cJSON_CreateString(string_uuid));
        //cJSON_AddNumberToObject(root, "device_sn",blister_buf.uuid);
        cJSON_AddNumberToObject(root, "timestamp",blister_buf.timestamp);
        cJSON_AddItemToObject(root, "device_type",cJSON_CreateString("PARAMETER_BLISTER"));
        cJSON_AddItemToObject(root, "device_version",cJSON_CreateString(blister_buf.version));
        }
    else if(case_pub == 3){
        cJSON_AddNumberToObject(root, "status",blister_buf.status);
        cJSON_AddNumberToObject(root, "water",blister_buf.water);
        cJSON_AddNumberToObject(root, "pressure_alarm",blister_buf.pressure_alarm);
        cJSON_AddNumberToObject(root, "liquid_alarm",blister_buf.liquid_alarm);
        cJSON_AddNumberToObject(root, "mode",blister_buf.mode);
        cJSON_AddNumberToObject(root, "heater",blister_buf.heater);
        cJSON_AddNumberToObject(root, "emergency_stop",blister_buf.emergency_stop);  
        cJSON_AddNumberToObject(root, "temperature",blister_buf.temperature);   
        cJSON_AddNumberToObject(root, "rssi",blister_buf.rssi);        
        cJSON_AddNumberToObject(root, "timestamp",blister_buf.timestamp);
        cJSON_AddItemToObject(root, "msg_id",cJSON_CreateString(blister_buf.msg_id)); //
        }
    else if(case_pub == 4){
        char string_uuid[20] = "0";//7cdfa1e592e0
        hex2str((uint8_t *)remote_buf.uuid,6,string_uuid);
        cJSON_AddItemToObject(root, "device_sn",cJSON_CreateString(string_uuid));
        //cJSON_AddNumberToObject(root, "device_sn",remote_buf.uuid);
        cJSON_AddNumberToObject(root, "timestamp",remote_buf.timestamp);
        cJSON_AddItemToObject(root, "device_type",cJSON_CreateString("PARAMETER_REMOTE"));
        cJSON_AddItemToObject(root, "device_version",cJSON_CreateString(remote_buf.version));
        }
    else if(case_pub == 5){
        cJSON_AddItemToObject(root, "switch_name",cJSON_CreateString("centralizer"));
        cJSON_AddNumberToObject(root, "value",remote_buf.centralizer);
        }
    else if(case_pub == 6){
        cJSON_AddItemToObject(root, "switch_name",cJSON_CreateString("rotation"));
        cJSON_AddNumberToObject(root, "value",remote_buf.rotation);
        }
    else if(case_pub == 7){
        cJSON_AddItemToObject(root, "switch_name",cJSON_CreateString("nozzle"));
        cJSON_AddNumberToObject(root, "value",remote_buf.nozzle);
        }
    else if(case_pub == 8){
        cJSON_AddNumberToObject(root, "emergency_stop",1);//remote_buf.emergency_stop
        }
    else if(case_pub == 9){
        cJSON_AddNumberToObject(root, "status",remote_buf.status);
        cJSON_AddNumberToObject(root, "rssi",remote_buf.rssi);
        }
    // else if(case_pub == 20){
    //     cJSON_AddItemToObject(root, "log_esp32",cJSON_CreateString(log_string));  //log_string get
    //     }
    char *msg = cJSON_Print(root);
    //ESP_LOGI(TAG, "%s",msg); 
    log_write_send("log:%s time:%s ",msg,parameter_read_time_string()); 
    strcpy(data,msg);
    cJSON_Delete(root);
}

void mqtt_reset(void)
{
    char *broker_url = {0};
    broker_url = parameter_read_broker_url();
    printf("parameter_read_broker_url:%s",broker_url); 

    esp_mqtt_client_stop(mqtt_client);
    esp_mqtt_client_disconnect(mqtt_client);
    esp_mqtt_client_set_uri(mqtt_client,broker_url);
    //esp_mqtt_set_config(mqtt_client,mqtt_cfg);
    //esp_mqtt_abort_connection
    //esp_mqtt_client_destroy(mqtt_client);
    // ets_delay_us(1000);
    // esp_mqtt_client_config_t mqtt_cfg = {
    //     .uri = broker_url,//BACKUP_MQTT_BROKER_URL,//CONFIG_BROKER_URL,
    // };

    //strcpy(mqtt_cfg.uri,broker_url);
    // mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    // esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
}

void ESP32_LOG_publish(char *log_buffer)
{
    int msg_id = 0;
    //char log_buf[2000] = "init";
    cJSON*root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "log_esp32",cJSON_CreateString(log_buffer));  //log_string get
    //data_publish(log_buffer,20);   //case 20 log
    msg_id = esp_mqtt_client_publish(mqtt_client, TOPIC_LOG_ESP32, log_buffer, 0, 1, 0);
    cJSON_Delete(root);
}
void mqtt_init(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");   
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);//  CONFIG_LOG_COLORS  
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_INFO);////ESP_LOG_DEBUG ESP_LOG_INFO ESP_LOG_WARN
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    //vTaskDelay(6000 / portTICK_RATE_MS); 
    mqtt_app_start();
}

void log_file_clear(char *filename)  
{
    ESP_LOGI(TAG, "clear_Opening file");
    FILE* f = fopen(LOG_FILE_PATH, "w");  //w a 
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "ESP32-LOG: ");
    fclose(f);
    ESP_LOGI(TAG, "File clear");

        // All done, unmount partition and disable SPIFFS
    esp_vfs_spiffs_unregister(NULL);
    ESP_LOGI(TAG, "SPIFFS unmounted");

    spiff_init();
}
void log_write_send(const char *format,...)  
{
    va_list arg;
    va_start(arg, format);
    vprintf(format, arg);
    
    ESP_LOGI(TAG, "Opening file");
    FILE* f = fopen(LOG_FILE_PATH, "a");  //w a 
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    vfprintf(f, format, arg);
    //device_states_publish(20); //topic log
    // fprintf(f, "Hello World!,format:%s;time:%d\n",format,esp_log_timestamp());
    //log_write(f, "log:time:%d\n",esp_log_timestamp());
    //ESP_LOGI(TAG, "File written");
    fclose(f);
    //ESP_LOGI(TAG, "File written");

    va_end(arg);
}
long long  getFileSize(char * files)
{
    long long size;
    FILE *fp =  fopen(files,"rb+");
    if( fp == NULL){
        printf("Open File Error");
        exit(0);
    }
    //定义pos
    fpos_t pos;
    //获取文件指针，写入pos
    fgetpos(fp,&pos);
    //文件指针指向末尾
    fseek(fp,0,SEEK_END);
    //获取文件指针到文件头部的字节大小
    size = ftell(fp);
    //文件指针还原
    fsetpos(fp,&pos);
    //释放文件
    fclose(fp);
    return size;
}
void log_process(void)
{
    char line[1000];
    int cnt = 0;
    ESP_LOGI(TAG, "log_process_enable");
    while(1)
    {
        if(cnt%60==30){
            char *cpSourceFile = LOG_FILE_PATH;
            long long res = getFileSize(cpSourceFile);
            printf("sizeof(/spiffs/log3.txt):%lld\n",res);
            if(res >= total-1000){
                log_file_clear("0");
            }
        }
        if(flag_log){
            int part_num = 0;
            ESP_LOGI(TAG, "Reading file");
            FILE* f = fopen(LOG_FILE_PATH, "r");//f = fopen("/spiffs/foo.txt", "r");
            if (f == NULL) {
                ESP_LOGE(TAG, "Failed to open file for reading");
                return;
            }
            ESP_LOGI(TAG, "f_start_addr:%p",&f);
            while(feof(f)==0){
                fread(line,1, sizeof(line), f);
                //ESP_LOGI(TAG, "part%d:%s ", part_num++,line);  //part_num++,
                ESP32_LOG_publish(line);
                memset(line,0,sizeof(line));
                vTaskDelay(30 / portTICK_RATE_MS); 
            }
            fclose(f);
            flag_log = 0;
        }
        vTaskDelay(1000 / portTICK_RATE_MS); 
        cnt++;
    }
}


void log_read_send(const char *format,...)  //  FILE* f,
{
    // va_list arg;
    // va_start(arg, format);
    // vprintf(format, arg);
    //fprintf(f, format);
    int part_num = 0;
    // Open renamed file for reading
    ESP_LOGI(TAG, "Reading file");
    FILE* f = fopen(LOG_FILE_PATH, "r");//f = fopen("/spiffs/foo.txt", "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    ESP_LOGI(TAG, "f_start_addr:%p",&f);
    char line[1000];
    //char *line = (char *)malloc(2000);
    //fgets(line, sizeof(line), f);
    ESP_LOGI(TAG, "Read from file:");
    while(feof(f)==0){
        fread(line,1, sizeof(line), f);
        //ESP_LOGI(TAG, "part%d:%s ", part_num++,line);  //part_num++,
        //device_states_publish(20); 
        ESP32_LOG_publish(line);
        memset(line,0,sizeof(line));
    }
    fclose(f);
    // for(int i = 0; i <100;i++)
    //     printf("%c",line[i]);
    // strip newline
    // char* pos = strchr(line, '\n');
    // if (pos) {
    //     *pos = '\0';
    // }
    //ESP_LOGI(TAG, "Read from file: '%s'", line);
 //   va_end(arg);
    //free(line);
}

void spiff_init(void)
{
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };

    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    // size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
        //ESP_LOGI(TAG, "Partition table: %s", conf.partition_label);
    }

    // log_write_send("testttt");
    // log_read_send('0');  
        // All done, unmount partition and disable SPIFFS
    // esp_vfs_spiffs_unregister(conf.partition_label);
    // ESP_LOGI(TAG, "SPIFFS unmounted");
}
// void cJSON_init(void)
// {
//     cJSON*root = cJSON_CreateObject();
//     cJSON *item = cJSON_CreateObject();
//     //CJSON_PUBLIC(cJSON*) cJSON_AddStringToObject(cJSON * const object, const char * const name, const char * const string);
//     cJSON_AddNumberToObject(root, "nozzle",1);
//     cJSON_AddNumberToObject(root, "status",1);
//     char *msg = cJSON_Print(root);
//     ESP_LOGI(TAG, "%s",msg);

//     cJSON_Delete(root);

//     return 0;
// }

// #if CONFIG_BROKER_URL_FROM_STDIN
//     char line[128];

//     if (strcmp(mqtt_cfg.uri, "FROM_STDIN") == 0) {
//         int count = 0;
//         ESP_LOGI(TAG, "Please enter url of mqtt broker");
//         while (count < 128) {
//             int c = fgetc(stdin);
//             if (c == '') {
//                 line[count] = '\0';
//                 break;
//             } else if (c > 0 && c < 127) {
//                 line[count] = c;
//                 ++count;
//             }
//             vTaskDelay(10 / portTICK_PERIOD_MS);
//         }
//         mqtt_cfg.uri = line;
//         ESP_LOGI(TAG, "Broker url: %s", line);
//     } else {
//         ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
//         abort();
//     }
// #endif /* CONFIG_BROKER_URL_FROM_STDIN */