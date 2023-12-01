#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <sys/param.h>
#include "esp_check.h"
#include "esp_log.h"
#include "para_list.h"
#include "esp_system.h"
#include "esp_partition.h"
#include <time.h>
//
PARAMETER_BRUSH brush_para;
PARAMETER_BLISTER blister_para;
PARAMETER_REMOTE remote_para;
PARAMETER_SUCTION_PUMP suction_pump_para;
PARAMETER_CONNECTION connection_para;

static const char *TAG = "para_list";
uint8_t FTC533_KEY_press = 0;
uint32_t time_delay = 25;
uint8_t twai_status = 0;
double remote_speed[4]= {0};
uint8_t robot_h_v[5]={0};
//char ap_mac_addr[6] = {0};
uint16_t vehicle_battery = 20000;


esp_err_t get_chip_id(uint32_t* chip_id){
    esp_err_t status = ESP_OK;
    *chip_id = 0x3FF00050; //(REG_READ(0x3FF00050) & 0xFF000000) |
                         //(REG_READ(0x3ff0005C) & 0xFFFFFF);
    return status;
}

void wifi_url_inital_set_para(void)
{
    strcpy(connection_para.wifi_ssid,BACKUP_EXAMPLE_ESP_WIFI_SSID);
    strcpy(connection_para.wifi_pass,BACKUP_EXAMPLE_ESP_WIFI_PASS);
    strcpy(connection_para.broker_url,BACKUP_MQTT_BROKER_URL);
    strcpy(connection_para.update_url,CONFIG_EXAMPLE_FIRMWARE_UPG_URL);   //remote   double press
    connection_para.wifi_bssid_set = 0;
    strcpy(connection_para.wifi_bssid,BACKUP_EXAMPLE_ESP_WIFI_AP_BSSID);
    if(flash_write_parameter() == -1)
        ESP_LOGI(TAG, "flash_write_parameter_error!");
}


void para_init(void)
{
    // uint32_t id;
    // get_chip_id(&id);
    // printf("SDK version:%s,chip id:%u\n", esp_get_idf_version(),id);
    static char mac_addr[6] = {0};
    esp_err_t ret =  esp_efuse_mac_get_default((uint8_t *)mac_addr);  //esp_read_mac  esp_efuse_mac_get_default
    if (ret != ESP_OK) 
        printf("esp_efuse_mac_get_default failed\n");
    else{
        printf("mac_addr:");
        for(uint8_t i = 0; i<6; i++)
            printf("%x",mac_addr[i]);
        printf("\r\n");
    }
    if(flash_read_parameter() == -1)
        printf("flash_read_parameter error");

    #ifdef DEVICE_TYPE_BRUSH
        //brush_para.uuid = id;
        strcpy(brush_para.uuid,mac_addr);
        brush_para.nozzle = 0;
        brush_para.centralizer = 0;
        brush_para.rotation = 0;
        brush_para.status = 1;
        brush_para.water = 0;
        brush_para.pressure_alarm = 1;
        brush_para.emergency_stop = 0;
        brush_para.timestamp = 1654585625000;
        strcpy(brush_para.msg_id,"msg_id");
        brush_para.temperature = 0;
        brush_para.pressure = 0;
        strcpy(brush_para.version,"1.0.0.0");
        brush_para.rssi = 0;
        brush_para.wifi_connection = 0;
    #else
        #ifdef DEVICE_TYPE_BLISTER
            //blister_para.uuid = id;
            strcpy(blister_para.uuid,mac_addr);
            blister_para.mode = 0;
            blister_para.heater = 0;
            blister_para.status = 1;
            blister_para.water = 0;
            blister_para.pressure_alarm = 1;
            blister_para.liquid_alarm = 1;
            blister_para.emergency_stop = 0;
            blister_para.timestamp = 1654585625000;
            strcpy(blister_para.msg_id,"msg_id");
            blister_para.temperature = 0;
            blister_para.pressure = 0;
            strcpy(blister_para.version,"1.0.0.0");
            blister_para.rssi = 0;
            blister_para.wifi_connection = 0;
        #else
            #ifdef DEVICE_TYPE_REMOTE
            //remote_para.uuid = id;
            strcpy(remote_para.uuid,mac_addr);
            remote_para.nozzle = 0;
            remote_para.centralizer = 0;
            remote_para.rotation = 0;
            remote_para.status = 1;
            remote_para.mode = 0;
            remote_para.angle = 0;
            remote_para.timestamp = 1654585625000;
            strcpy(remote_para.msg_id,"msg_id");
            strcpy(remote_para.version,"1.0.0.0");
            remote_para.rssi = 0;
            remote_para.wifi_connection = 0;
            #else
                #ifdef DEVICE_TYPE_SUCTION_PUMP
                strcpy(suction_pump_para.uuid,mac_addr);
                suction_pump_para.status = 1;
                suction_pump_para.suction[0] = 0;
                suction_pump_para.suction[1] = 0;
                suction_pump_para.suction[2] = 0;
                suction_pump_para.suction[3] = 0;
                suction_pump_para.timestamp = 1654585625000;
                strcpy(suction_pump_para.msg_id,"msg_id");
                strcpy(suction_pump_para.version,"1.0.0.0");
                suction_pump_para.rssi = 0;
                suction_pump_para.wifi_connection = 0;
                #endif   
            #endif
        #endif
    #endif
    
}

int8_t flash_write_parameter(void)
{
    const char* data = &connection_para;
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
    // printf("before write connection_para.wifi_ssid:%s\r\n",connection_para.wifi_ssid);
    // printf("connection_para.wifi_pass:%s\r\n",connection_para.wifi_pass);
    // printf("connection_para.broker_url:%s\r\n",connection_para.broker_url);
    // printf("connection_para.update_url:%s\r\n",connection_para.update_url);

    printf("Write data to custom partition\r\n");    //strlen(data) + 1  &connection_para.wifi_ssid
    if (esp_partition_write(find_partition, 0, data, sizeof(PARAMETER_CONNECTION)) != ESP_OK) {     
	    printf("Write partition data error");
	    return -1;
    }

    printf("Read data from custom partition\r\n");
    if (esp_partition_read(find_partition, 0, data, sizeof(PARAMETER_CONNECTION)) != ESP_OK) {
	    printf("Read partition data error");
	    return -1;
    } 
    printf("connection_para.wifi_ssid:%s\r\n",connection_para.wifi_ssid);
    printf("connection_para.wifi_pass:%s\r\n",connection_para.wifi_pass);
    printf("connection_para.broker_url:%s\r\n",connection_para.broker_url);
    printf("connection_para.update_url:%s\r\n",connection_para.update_url);
    printf("connection_para.wifi_bssid_set:%d\r\n",connection_para.wifi_bssid_set);
    printf("connection_para.wifi_bssid:");
    ESP_LOG_BUFFER_HEX(TAG, connection_para.wifi_bssid, 6);
    //printf("connection_para.wifi_bssid:%s\r\n",connection_para.wifi_bssid);
    // printf("Read data from custom partition\r\n");
    // if (esp_partition_read(find_partition, 0, dest_data, 1024) != ESP_OK) {
	//     printf("Read partition data error");
	//     return -1;
    // }
    // printf("Receive data: %s\r\n", (char*)dest_data);

    // strcpy(connection_para.wifi_ssid,"0");
    // strcpy(connection_para.wifi_pass,"0");
    // strcpy(connection_para.broker_url,"0");
    // strcpy(connection_para.update_url,"0");

    // printf("Write data to custom partition\r\n");    //strlen(data) + 1
    // if (esp_partition_write(find_partition, 0, data, sizeof(PARAMETER_CONNECTION)) != ESP_OK) {     
	//     printf("Write partition data error");
	//     return -1;
    // }

    // printf("Read data from custom partition\r\n");
    // if (esp_partition_read(find_partition, 0, dest_data, 1024) != ESP_OK) {
	//     printf("Read partition data error");
	//     return -1;
    // }
    return 0;
}

int8_t flash_erase_parameter(void)
{
    const char* data = &connection_para;
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
    printf("Read data from custom partition\r\n");
    if (esp_partition_read(find_partition, 0, data, sizeof(PARAMETER_CONNECTION)) != ESP_OK) {
	    printf("Read partition data error");
	    return -1;
    }
    printf("connection_para.wifi_ssid:%s\r\n",connection_para.wifi_ssid);
    printf("connection_para.wifi_pass:%s\r\n",connection_para.wifi_pass);
    printf("connection_para.broker_url:%s\r\n",connection_para.broker_url);
    printf("connection_para.update_url:%s\r\n",connection_para.update_url);
    printf("connection_para.wifi_bssid_set:%d\r\n",connection_para.wifi_bssid_set);
    printf("connection_para.wifi_bssid:");
    ESP_LOG_BUFFER_HEX(TAG, connection_para.wifi_bssid, 6);
    return 0;
}

int8_t flash_read_parameter(void)
{
    //const char* data = "Test read and write partition";
    const char* data = &connection_para;
    uint8_t dest_data[1024] = {0};
    const esp_partition_t *find_partition = NULL;
    find_partition = esp_partition_find_first(0x40, 0x0, NULL);
    if(find_partition == NULL){
	    printf("No partition found!\r\n");
	    return -1;
    }
    printf("Read data from custom partition\r\n");
    if (esp_partition_read(find_partition, 0, data, sizeof(PARAMETER_CONNECTION)) != ESP_OK) {
	    printf("Read partition data error");
	    return -1;
    }
    printf("connection_para.wifi_ssid:%s\r\n",connection_para.wifi_ssid);
    printf("connection_para.wifi_pass:%s\r\n",connection_para.wifi_pass);
    printf("connection_para.broker_url:%s\r\n",connection_para.broker_url);
    printf("connection_para.update_url:%s\r\n",connection_para.update_url);
    printf("connection_para.wifi_bssid_set:%d\r\n",connection_para.wifi_bssid_set);
    printf("connection_para.wifi_bssid:");
    ESP_LOG_BUFFER_HEX(TAG, connection_para.wifi_bssid, 6);

    if(connection_para.wifi_ssid[0] == 0xff && connection_para.wifi_pass[0] == 0xff){
        printf("connection_para == ff then write inital parameter");
        wifi_url_inital_set_para();
    }
    
    if(connection_para.wifi_bssid_set ==0xff  && connection_para.wifi_bssid[0] == 0xff){
        printf("connection_para == ff then write inital parameter");
        wifi_url_inital_set_para();
    }
    //wifi_url_inital_set_para();
    // if(connection_para.wifi_ssid[0] == 0x00 && connection_para.wifi_pass[0] == 0x00){
    //     printf("connection_para == 00 then write inital parameter");
    //     wifi_url_inital_set_para();
    // }    
    //printf("Receive data: %s\r\n", (char*)dest_data);
    return 0;
}
void parameter_write_wifi_bssid(uint8_t str_para[])
{   
    for(uint8_t i=0;i<sizeof(str_para)+2;i++)    //sizeof(str_para) = 4
        connection_para.wifi_bssid[i] = str_para[i];
    // printf("connection_para.wifi_bssid:");   
    // ESP_LOG_BUFFER_HEX(TAG, connection_para.wifi_bssid, 6);
}

char *parameter_read_wifi_bssid(void)
{
    return connection_para.wifi_bssid;
}

void parameter_write_wifi_bssid_set(uint8_t b1)
{
    connection_para.wifi_bssid_set = b1;
}
uint8_t parameter_read_wifi_bssid_set(void)
{
    return connection_para.wifi_bssid_set;
}


void parameter_write_wifi_ssid(char *str_para)
{   
    strcpy(connection_para.wifi_ssid,str_para);
}

char *parameter_read_wifi_ssid(void)
{
    return connection_para.wifi_ssid;
}

void parameter_write_wifi_pass(char *str_para)
{   
    strcpy(connection_para.wifi_pass,str_para);
}

char *parameter_read_wifi_pass(void)
{
    return connection_para.wifi_pass;
}

void parameter_write_broker_url(char *str_para)
{   
    strcpy(connection_para.broker_url,str_para);
}

char *parameter_read_broker_url(void)
{
    return connection_para.broker_url;
}

void parameter_write_update_url(char *str_para)
{   
    strcpy(connection_para.update_url,str_para);
}

char *parameter_read_update_url(void)
{
    return connection_para.update_url;
}

void parameter_write_water(uint8_t value)
{   
    brush_para.water = value;
    blister_para.water = value;
}

uint8_t parameter_read_water(void)
{
#ifdef DEVICE_TYPE_BRUSH
return brush_para.water;
#endif
#ifdef DEVICE_TYPE_BLISTER
return blister_para.water;
#endif
#ifdef DEVICE_TYPE_REMOTE
return remote_para.water;
#endif
#ifdef DEVICE_TYPE_SUCTION_PUMP
return 0;
#endif
}

void parameter_write_pressure_alarm(uint8_t value)
{   
    brush_para.pressure_alarm = value;
    blister_para.pressure_alarm = value;
}

uint8_t parameter_read_pressure_alarm(void)
{
#ifdef DEVICE_TYPE_BRUSH
return brush_para.pressure_alarm;
#endif
#ifdef DEVICE_TYPE_BLISTER
return blister_para.pressure_alarm;
#endif
#ifdef DEVICE_TYPE_REMOTE
return remote_para.pressure_alarm;
#endif
#ifdef DEVICE_TYPE_SUCTION_PUMP
return 0;
#endif
}

void parameter_write_liquid_alarm(uint8_t value)
{
    blister_para.liquid_alarm = value;
}

uint8_t parameter_read_liquid_alarm(void)
{
    return blister_para.liquid_alarm;
}


void get_parameter(PARAMETER_BRUSH *brush_t)
{
    memcpy(brush_t,&brush_para,sizeof(PARAMETER_BRUSH));
}

void get_blister_parameter(PARAMETER_BLISTER *blister_t)
{
    memcpy(blister_t,&blister_para,sizeof(PARAMETER_BLISTER));
}

void get_remote_parameter(PARAMETER_REMOTE *remote_t)
{
    memcpy(remote_t,&remote_para,sizeof(PARAMETER_REMOTE));
}

void get_suction_pump_parameter(PARAMETER_SUCTION_PUMP *rsuction_pump_t)
{
    memcpy(rsuction_pump_t,&suction_pump_para,sizeof(PARAMETER_SUCTION_PUMP));
}

void parameter_write_version(char *str_version)
{   
    strcpy(brush_para.version,str_version);
    strcpy(blister_para.version,str_version);
    strcpy(remote_para.version,str_version);
    strcpy(suction_pump_para.version,str_version);
}


// char *parameter_read_version(void)
// {
//     return brush_para.version;
// }

void parameter_write_msg_id(char *str_msgid)
{   
    strcpy(brush_para.msg_id,str_msgid);
    strcpy(blister_para.msg_id,str_msgid);
    strcpy(remote_para.msg_id,str_msgid);
    strcpy(suction_pump_para.msg_id,str_msgid);
}


char *parameter_read_msg_id(void)
{
    return brush_para.msg_id;
}

void parameter_write_timestamp(double timestamp)
{   
    int time_int = 0;
    char time_string[20] = {0};
    time_int = (int)(parameter_read_timestamp()/1000) + 8*60*60; //BEIJING timestamp += 8*60*60;
    stamp_to_standard(time_int,time_string);
    
    #ifdef DEVICE_TYPE_BRUSH
    brush_para.timestamp = timestamp;
    brush_para.time_int = time_int;
    strcpy(brush_para.time_string , time_string);
    #endif
    #ifdef DEVICE_TYPE_BLISTER
    blister_para.timestamp = timestamp;
    blister_para.time_int = time_int;
    strcpy(blister_para.time_string , time_string);
    #endif
    #ifdef DEVICE_TYPE_REMOTE
    remote_para.timestamp = timestamp;
    remote_para.time_int = time_int;
    strcpy(remote_para.time_string , time_string);
    #endif
    #ifdef DEVICE_TYPE_SUCTION_PUMP
    suction_pump_para.timestamp = timestamp;
    suction_pump_para.time_int = time_int;
    strcpy(suction_pump_para.time_string , time_string);
    #endif
}

double parameter_read_timestamp(void)
{
#ifdef DEVICE_TYPE_BRUSH
return brush_para.timestamp;
#endif
#ifdef DEVICE_TYPE_BLISTER
return blister_para.timestamp;
#endif
#ifdef DEVICE_TYPE_REMOTE
return remote_para.timestamp;
#endif
#ifdef DEVICE_TYPE_SUCTION_PUMP
return suction_pump_para.timestamp;
#endif
}

void parameter_write_emergency_stop(uint8_t value)
{   
    
#ifdef DEVICE_TYPE_BRUSH
brush_para.emergency_stop = value;
#endif
#ifdef DEVICE_TYPE_BLISTER
blister_para.emergency_stop = value;
#endif
#ifdef DEVICE_TYPE_REMOTE
remote_para.emergency_stop = value;
#endif
}

uint8_t parameter_read_emergency_stop(void)
{
#ifdef DEVICE_TYPE_BRUSH
return brush_para.emergency_stop;
#endif
#ifdef DEVICE_TYPE_BLISTER
return blister_para.emergency_stop;
#endif
#ifdef DEVICE_TYPE_REMOTE
return remote_para.emergency_stop;
#endif
#ifdef DEVICE_TYPE_SUCTION_PUMP
return 0;
#endif
}

void parameter_write_centralizer(uint8_t value)
{   
    brush_para.centralizer = value;
    remote_para.centralizer = value;
}

uint8_t parameter_read_centralizer(void)
{
#ifdef DEVICE_TYPE_BRUSH
return brush_para.centralizer;
#endif
#ifdef DEVICE_TYPE_BLISTER
return 0;
#endif
#ifdef DEVICE_TYPE_REMOTE
return remote_para.centralizer;
#endif
#ifdef DEVICE_TYPE_SUCTION_PUMP
return 0;
#endif
}

void parameter_write_rotation(uint8_t value)
{   
    brush_para.rotation = value;
    remote_para.rotation = value;
}

uint8_t parameter_read_rotation(void)
{
#ifdef DEVICE_TYPE_BRUSH
return brush_para.rotation;
#endif
#ifdef DEVICE_TYPE_BLISTER
return 0;
#endif
#ifdef DEVICE_TYPE_REMOTE
return remote_para.rotation;
#endif
#ifdef DEVICE_TYPE_SUCTION_PUMP
return 0;
#endif
}

void parameter_write_nozzle(uint8_t value)
{   
    brush_para.nozzle = value;
    remote_para.nozzle = value;
}

uint8_t parameter_read_nozzle(void)
{
#ifdef DEVICE_TYPE_BRUSH
return brush_para.nozzle;
#endif
#ifdef DEVICE_TYPE_BLISTER
return 0;
#endif
#ifdef DEVICE_TYPE_REMOTE
return remote_para.nozzle;
#endif
#ifdef DEVICE_TYPE_SUCTION_PUMP
return 0;
#endif
    
}

void parameter_write_pressure(uint16_t pressure)
{   
    brush_para.pressure = pressure;
}

uint16_t parameter_read_pressure(void)
{
    return brush_para.pressure;
}

void parameter_write_temperature(double temperature)
{   
    brush_para.temperature = temperature;
    blister_para.temperature = temperature;
}

double parameter_read_temperature(void)
{
    return brush_para.temperature;
}
double blister_read_temperature(void)
{
    return blister_para.temperature;
}

void parameter_write_heater(uint8_t value)
{   
    blister_para.heater = value;
    remote_para.heater = value;
}

uint8_t parameter_read_heater(void)
{
    return blister_para.heater;
}

void parameter_write_mode(uint8_t value)
{   
    blister_para.mode = value;
    remote_para.mode = value;
}

uint8_t parameter_read_mode(void)
{
    return blister_para.mode;
}

void parameter_write_rssi(int8_t value)
{  
    brush_para.rssi = value; 
    blister_para.rssi = value;
    remote_para.rssi = value;
    suction_pump_para.rssi = value;
}
void parameter_write_wifi_connection(uint8_t value)
{  
    brush_para.wifi_connection = value; 
    blister_para.wifi_connection = value;
    remote_para.wifi_connection = value;
    suction_pump_para.wifi_connection = value;
}

uint8_t parameter_read_wifi_connection(void)
{
#ifdef DEVICE_TYPE_BRUSH
return brush_para.wifi_connection;
#endif
#ifdef DEVICE_TYPE_BLISTER
return blister_para.wifi_connection;
#endif
#ifdef DEVICE_TYPE_REMOTE
return remote_para.wifi_connection;
#endif
#ifdef DEVICE_TYPE_SUCTION_PUMP
return suction_pump_para.wifi_connection;
#endif
}

void parameter_write_FTC533(uint8_t value)
{   
    FTC533_KEY_press = value;
}

uint8_t parameter_read_FTC533(void)
{
    return FTC533_KEY_press;
}

void parameter_write_debug(uint32_t value)
{   
    time_delay = value;
}

uint32_t parameter_read_debug(void)
{
    return time_delay;
}

void parameter_write_twai_status(uint8_t value)
{   
    twai_status = value;
}

uint8_t parameter_read_twai_status(void)
{
    return twai_status;
}

void parameter_write_remote_xyz(double speed_x,double speed_y,double speed_z,double speed_axes3)
{
    remote_speed[0] = speed_x;  //atof(
    remote_speed[1] = speed_y;
    remote_speed[2] = speed_z;    //jiexian  fangxiang
    remote_speed[3] = speed_axes3; 
}

void parameter_write_robot_h_v(uint8_t horizontal,uint8_t vertical,uint8_t servo,uint8_t video,uint8_t bakup)
{
    robot_h_v[0] = horizontal;
    robot_h_v[1] = vertical;
    robot_h_v[2] = servo;
    robot_h_v[3] = video;
    robot_h_v[4] = bakup;
}

// void parameter_write_robot_h_v(uint8_t horizontal,uint8_t vertical,uint8_t servo)
// {
//     robot_h_v[0] = horizontal;
//     robot_h_v[1] = vertical;
//     robot_h_v[2] = servo;
// }
// void parameter_read_remote_xyz(double speed_x,double speed_y,double speed_z)
// {
//     speed_x = remote_speed[0];
//     speed_y = remote_speed[1];
//     speed_z = remote_speed[2];
// }



void parameter_write_air_pump(uint8_t value)
{   
    brush_para.air_pump = value;
    blister_para.air_pump = value;
    remote_para.air_pump = value;
}

uint8_t parameter_read_air_pump(void)
{
    #ifdef DEVICE_TYPE_BRUSH
    return brush_para.air_pump;
    #endif
    #ifdef DEVICE_TYPE_BLISTER
    return blister_para.air_pump;
    #endif
    #ifdef DEVICE_TYPE_REMOTE
    return remote_para.air_pump;
    #endif
    #ifdef DEVICE_TYPE_SUCTION_PUMP
    return 0;
    #endif
}

void parameter_write_vehicle_battery(uint16_t bat)
{
    vehicle_battery = bat;
}
uint16_t parameter_read_vehicle_battery(void)
{
    return vehicle_battery;
}



uint32_t DateTime2Stamp(PST_DATE_TIME pstDateTime)
{
    struct tm stTime;
    time_t TimeStamp;

    stTime.tm_year = pstDateTime->Year - 1900;
    stTime.tm_mon = pstDateTime->Mon - 1;
    stTime.tm_mday = pstDateTime->Day;
    stTime.tm_hour = pstDateTime->Hour;
    stTime.tm_min = pstDateTime->Min;
    stTime.tm_sec = pstDateTime->Sec;
    TimeStamp = mktime(&stTime);
    return (uint32_t)TimeStamp;
}

/********************************************************************
* name          : TimeStamp2DateTime
* description   : 根据时间戳获取时间
* Input         : Stamp
* Output        : none
* Return        : DateTime
********************************************************************/
ST_DATE_TIME TimeStamp2DateTime(uint32_t Stamp)
{
    ST_DATE_TIME stDateTime;

    time_t TimeStamp = Stamp;
    struct tm* pstTime = localtime(&TimeStamp);
    stDateTime.Year = pstTime->tm_year + 1900;
    stDateTime.Mon = pstTime->tm_mon + 1;
    stDateTime.Day = pstTime->tm_mday;
    stDateTime.Hour = pstTime->tm_hour;
    stDateTime.Min = pstTime->tm_min;
    stDateTime.Sec = pstTime->tm_sec;
    printf("%d,%d,%d,%d,%d,%d",stDateTime.Year,stDateTime.Mon,stDateTime.Day,stDateTime.Hour,stDateTime.Min,stDateTime.Sec);
    return stDateTime;
}

int standard_to_stamp(char *str_time)
{
	struct tm stm;
	int iY,iM,iD,iH,iMin,iS;
	memset(&stm,0,sizeof(stm));
	iY = atoi(str_time);
	iM = atoi(str_time+5);     //和实际输入的格式 有关系
	iD = atoi(str_time+7);
	iH = atoi(str_time+10);
	iMin = atoi(str_time+13);
	iS = atoi(str_time+16);
	stm.tm_year=iY-1900;
	stm.tm_mon=iM-1;
	stm.tm_mday=iD;
	stm.tm_hour=iH;
	stm.tm_min=iMin;
	stm.tm_sec=iS;
	printf("standard time:%d-%d-%d %d:%d:%d\n", iY, iM, iD, iH, iMin, iS);

	return (int)mktime(&stm);

}

Times stamp_to_standard(int stampTime,char *time_s)
{
	time_t tick = (time_t)stampTime;
	struct tm tm;
	char s[20];
	Times standard;
	tm = *localtime(&tick);
	strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &tm);
	printf("stampTime:%d standard time:%s\n", (int)tick, s);

	standard.Year = atoi(s);
	standard.Mon = atoi(s+5);
	standard.Day = atoi(s+8);
	standard.Hour = atoi(s+11);
	standard.Min = atoi(s+14);
	standard.Second = atoi(s+17);
	//printf("%d,%d,%d,%d,%d,%d",standard.Year,standard.Mon,standard.Day,standard.Hour,standard.Min,standard.Second);
    strcpy(time_s , s);
	return standard;
}

char *parameter_read_time_string(void)
{
#ifdef DEVICE_TYPE_BRUSH
return brush_para.time_string;
#endif
#ifdef DEVICE_TYPE_BLISTER
return blister_para.time_string;
#endif
#ifdef DEVICE_TYPE_REMOTE
return remote_para.time_string;
#endif
#ifdef DEVICE_TYPE_SUCTION_PUMP
return suction_pump_para.time_string;
#endif
}


void parameter_write_suction_para(uint8_t str_para[])
{   
    //for(uint8_t i=0;i<4;i++)    
    suction_pump_para.suction[0] = str_para[0];

}

// uint8_t *parameter_read_suction_para(void)
// {
//     return suction_pump_para.suction;
// }
