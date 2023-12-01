/* TWAI Self Test Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/*
 * The following example demonstrates the self testing capabilities of the TWAI
 * peripheral by utilizing the No Acknowledgment Mode and Self Reception Request
 * capabilities. This example can be used to verify that the TWAI peripheral and
 * its connections to the external transceiver operates without issue. The example
 * will execute multiple iterations, each iteration will do the following:
 * 1) Start the TWAI driver
 * 2) Transmit and receive 100 messages using self reception request
 * 3) Stop the TWAI driver
 */

#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/twai.h"

#include "twai_app.h"
#include "para_list.h"
/* --------------------- Definitions and static variables ------------------ */

//Example Configurations
#define NO_OF_MSGS              10
#define NO_OF_ITERS             3
#define TX_GPIO_NUM             4//2//CONFIG_EXAMPLE_TX_GPIO_NUM
#define RX_GPIO_NUM             5//1//CONFIG_EXAMPLE_RX_GPIO_NUM
#define TX_TASK_PRIO            8       //Sending task priority
#define RX_TASK_PRIO            9       //Receiving task priority
#define CTRL_TSK_PRIO           10      //Control task priority
#define MSG_ID                  0x181   //11 bit standard format ID
#define MSG_ID_2                  0x182   //11 bit standard format ID
#define EXAMPLE_TAG             "TWAI Test"

#define SPEED_DEX 1000

static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();  //TWAI_TIMING_CONFIG_25KBITS
//Filter all other IDs except MSG_ID    
static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
// static const twai_filter_config_t f_config = {.acceptance_code = (MSG_ID << 21),
//                                              .acceptance_mask = ~(TWAI_STD_ID_MASK << 21),
//                                              .single_filter = true};
//Set to NO_ACK mode due to self testing with single module
static const twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NORMAL);  //TWAI_MODE_NORMAL //TWAI_MODE_NO_ACK

SemaphoreHandle_t tx_sem;
static SemaphoreHandle_t rx_sem;
static SemaphoreHandle_t ctrl_sem;
static SemaphoreHandle_t done_sem;

typedef enum
{
    GO = 0, //前进
    BACK, //后退
    LEFT_SHIFT,//左平移
    RIGHT_SHIFT,//右平移
    LEFT, //左转
    RIGHT, //右转
    STOP, //停止
}MM_DIRECTION;

#define MM_MIX_SPEED -1.0f //最小速度
#define MM_MAX_SPEED 1.0f //最大速度

uint8_t MM_MSG_BUFFER[8] = {0};
void Msg_Clear(uint8_t *MM_MSG);
void Speed_2_Msg(uint8_t *MM_MSG, float speed, MM_DIRECTION direction);
extern double remote_speed[4];
extern uint8_t robot_h_v[5];
void Speed_2_can(uint8_t *MM_MSG);
uint8_t flag_enable_send = 0;
/* --------------------------- Tasks and Functions -------------------------- */

static void twai_transmit_task(void *arg)
{
    uint8_t status = 0;
    twai_message_t tx_msg = {.data_length_code = 8, .identifier = MSG_ID, .self = 1};
    //for (int iter = 0; iter < 1; iter++)
    while(1) {
        if(flag_enable_send){
            xSemaphoreTake(tx_sem, portMAX_DELAY);
            
            if(robot_h_v[0]||robot_h_v[1]||robot_h_v[2]||robot_h_v[3]||robot_h_v[4]){
                Msg_Clear(tx_msg.data);
                msg_2_can(tx_msg.data);
                tx_msg.identifier = MSG_ID_2;
                ESP_ERROR_CHECK(twai_transmit(&tx_msg, portMAX_DELAY));
            }
            else{
                // status = parameter_read_twai_status();
                Msg_Clear(tx_msg.data);
                // Speed_2_Msg(tx_msg.data,0.5,status);
                Speed_2_can(tx_msg.data);
                //for (int iter = 0; iter < NO_OF_ITERS; iter++)
                tx_msg.identifier = MSG_ID;
                ESP_ERROR_CHECK(twai_transmit(&tx_msg, portMAX_DELAY));
            }
            //ESP_LOGI(EXAMPLE_TAG, "twai_transmit:%d",status);
            vTaskDelay(pdMS_TO_TICKS(10));
            // for (int i = 0; i < NO_OF_MSGS; i++) {
            //     //Transmit messages using self reception request
            //     tx_msg.data[0] = i;
            //     ESP_ERROR_CHECK(twai_transmit(&tx_msg, portMAX_DELAY));
            //     vTaskDelay(pdMS_TO_TICKS(10));
            // }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    //vTaskDelete(NULL);
}

static void twai_receive_task(void *arg)
{
    twai_message_t rx_message;// = {.data_length_code = 8, .identifier = 0x103};
    int voltage = 0;
    while(1)
    {
        //xSemaphoreTake(rx_sem, portMAX_DELAY);
        ESP_ERROR_CHECK(twai_receive(&rx_message, portMAX_DELAY));
        if(rx_message.identifier == 0x101){
            ESP_LOGI(EXAMPLE_TAG, "rx_message.identifier:%d",rx_message.identifier);
            ESP_LOG_BUFFER_HEX(EXAMPLE_TAG, rx_message.data, 8);
            voltage = (uint16_t)((rx_message.data[5]<<8) + rx_message.data[4]);
            ESP_LOGI(EXAMPLE_TAG, "voltage:%d",voltage);
            parameter_write_vehicle_battery(voltage);
            flag_enable_send = 1;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    // for (int iter = 0; iter < NO_OF_ITERS; iter++) {
    //     xSemaphoreTake(rx_sem, portMAX_DELAY);
    //     for (int i = 0; i < NO_OF_MSGS; i++) {
    //         //Receive message and print message data
    //         ESP_ERROR_CHECK(twai_receive(&rx_message, portMAX_DELAY));
    //         //ESP_LOGI(EXAMPLE_TAG, "Msg received - Data = %d", rx_message.data[0]);
    //         ESP_LOG_BUFFER_HEX(EXAMPLE_TAG, rx_message.data, 8);
    //     }
    //     //Indicate to control task all messages received for this iteration
    //     xSemaphoreGive(ctrl_sem);
    // }
    // vTaskDelete(NULL);
}

// static void twai_control_task(void *arg)
// {
//     tx_sem = xSemaphoreCreateBinary();
//     //rx_sem = xSemaphoreCreateBinary();
//     // ctrl_sem = xSemaphoreCreateBinary();
//     // done_sem = xSemaphoreCreateBinary();
//     ESP_ERROR_CHECK(twai_start());
//     ESP_LOGI(EXAMPLE_TAG, "Driver started");
//     //xSemaphoreGive(rx_sem);
//     while(1){
//         vTaskDelay(pdMS_TO_TICKS(100));
//     }
//     // xSemaphoreTake(ctrl_sem, portMAX_DELAY);
//     // for (int iter = 0; iter < 1; iter++) {
//     //     //Start TWAI Driver for this iteration
//     //     ESP_ERROR_CHECK(twai_start());
//     //     ESP_LOGI(EXAMPLE_TAG, "Driver started");

//     //     //Trigger TX and RX tasks to start transmitting/receiving
//     //     xSemaphoreGive(rx_sem);
//     //     //xSemaphoreGive(tx_sem);
//     //     //xSemaphoreTake(ctrl_sem, portMAX_DELAY);    //Wait for TX and RX tasks to finish iteration

//     //     ESP_ERROR_CHECK(twai_stop());               //Stop the TWAI Driver
//     //     ESP_LOGI(EXAMPLE_TAG, "Driver stopped");
//     //     vTaskDelay(pdMS_TO_TICKS(100));             //Delay then start next iteration
//     // }
//     //xSemaphoreGive(done_sem);

//         //Wait for all iterations and tasks to complete running
//     //xSemaphoreTake(done_sem, portMAX_DELAY);

//     //Uninstall TWAI driver
//     // ESP_ERROR_CHECK(twai_driver_uninstall());
//     // ESP_LOGI(EXAMPLE_TAG, "Driver uninstalled");

//     // //Cleanup
//     // vSemaphoreDelete(tx_sem);
//     // vSemaphoreDelete(rx_sem);
//     // vSemaphoreDelete(ctrl_sem);
//     // vQueueDelete(done_sem);

//     // vTaskDelete(NULL);

// }

void twai_init(void)
{
    //Create tasks and synchronization primitives
    tx_sem = xSemaphoreCreateBinary();
    //Install TWAI driver
    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
    ESP_LOGI(EXAMPLE_TAG, "Driver installed");

    //xTaskCreatePinnedToCore(twai_control_task, "TWAI_ctrl", 4096, NULL, CTRL_TSK_PRIO, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(twai_receive_task, "TWAI_rx", 4096, NULL, RX_TASK_PRIO, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(twai_transmit_task, "TWAI_tx", 4096, NULL, TX_TASK_PRIO, NULL, tskNO_AFFINITY);

    
    ESP_ERROR_CHECK(twai_start());
    ESP_LOGI(EXAMPLE_TAG, "Driver started");

    //Start control task
    //xSemaphoreGive(ctrl_sem);

}

void msg_2_can(uint8_t *MM_MSG)
{
    MM_MSG[0] = robot_h_v[0];
    MM_MSG[1] = robot_h_v[1];
    MM_MSG[2] = robot_h_v[2];
    MM_MSG[3] = robot_h_v[3];
    MM_MSG[4] = robot_h_v[4];
    MM_MSG[5] = 0;
    MM_MSG[6] = 0;
    MM_MSG[7] = 0;
}

void Speed_2_can(uint8_t *MM_MSG)
{
    for(uint8_t i=0;i<4;i++){
        if(remote_speed[i] < MM_MIX_SPEED) remote_speed[i] = MM_MIX_SPEED;
        if(remote_speed[i] > MM_MAX_SPEED) remote_speed[i] = MM_MAX_SPEED;
    }
    //double remote_speed[3] = {0};
    //parameter_read_remote_xyz(speed[0],speed[1],speed[2]);
    MM_MSG[0] = ((uint16_t)(remote_speed[0]*SPEED_DEX)>>8)&0xff;
    MM_MSG[1] = (uint8_t)(remote_speed[0]*SPEED_DEX)&0xff;
    MM_MSG[2] = ((uint16_t)(remote_speed[1]*SPEED_DEX)>>8)&0xff;
    MM_MSG[3] = (uint8_t)(remote_speed[1]*SPEED_DEX)&0xff;
    MM_MSG[4] = ((uint16_t)(remote_speed[2]*SPEED_DEX)>>8)&0xff;
    MM_MSG[5] = (uint8_t)(remote_speed[2]*SPEED_DEX)&0xff;
    MM_MSG[6] = ((uint16_t)(remote_speed[3]*SPEED_DEX)>>8)&0xff;//robot_h_v[0];
    MM_MSG[7] = (uint8_t)(remote_speed[3]*SPEED_DEX)&0xff;//robot_h_v[1];

}

void Speed_2_Msg(uint8_t *MM_MSG, float speed, MM_DIRECTION direction)
{
    if(MM_MSG == NULL) return;
    if(speed < MM_MIX_SPEED) speed = MM_MIX_SPEED;
    if(speed > MM_MAX_SPEED) speed = MM_MAX_SPEED;
    //报文结构 Xh Xl Yh Yl Zh Zl CRC1 CRC2
    //正数为正方向，负数为负方向

    switch (direction)
    {
        case GO:
            MM_MSG[0] = ((uint16_t)(speed*SPEED_DEX)>>8)&0xff;
            MM_MSG[1] = (uint8_t)(speed*SPEED_DEX)&0xff;
            break;
        case BACK:
            MM_MSG[0] = ((uint16_t)(-speed*SPEED_DEX)>>8)&0xff;
            MM_MSG[1] = (uint8_t)(-speed*SPEED_DEX)&0xff;
            break;
        case LEFT_SHIFT:
            MM_MSG[2] = ((uint16_t)(speed*SPEED_DEX)>>8)&0xff;
            MM_MSG[3] = (uint8_t)(speed*SPEED_DEX)&0xff;
            break;
        case RIGHT_SHIFT:
            MM_MSG[2] = ((uint16_t)(-speed*SPEED_DEX)>>8)&0xff;
            MM_MSG[3] = (uint8_t)(-speed*SPEED_DEX)&0xff;
            break;
        case LEFT:
            MM_MSG[4] = ((uint16_t)(speed*SPEED_DEX)>>8)&0xff;
            MM_MSG[5] = (uint8_t)(speed*SPEED_DEX)&0xff;
            break;
        case RIGHT:
            MM_MSG[4] = ((uint16_t)(-speed*SPEED_DEX)>>8)&0xff;
            MM_MSG[5] = (uint8_t)(-speed*SPEED_DEX)&0xff;
            break;

        default:
            Msg_Clear(MM_MSG);
            break;

    }   
    // ESP_LOGI(EXAMPLE_TAG, "direction:%d",direction);
    // ESP_LOGI(EXAMPLE_TAG, "MM_MSG[1]:%d",MM_MSG[1]);
}

//报文清零
void Msg_Clear(uint8_t *MM_MSG)
{
    if(MM_MSG == NULL) return;
    for(uint8_t i = 0; i < 8; i++)
    {
        MM_MSG[i] = 0;
    }
}