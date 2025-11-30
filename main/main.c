#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "vl53l5cx_api.h" 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "esp_system.h"

typedef struct {
    VL53L5CX_Configuration Dev;
    VL53L5CX_ResultsData Results;
    uint8_t status;
    uint8_t loop;
    uint8_t isReady;
    uint8_t i;
} TaskParameters;

void Vl53l5cxTaskCode(void *pvParameters)
{
    vTaskDelay(pdMS_TO_TICKS(100));
    printf(">>>>Entered the vl53l5cx task function<<<<<")
    TaskParameters *p = (TaskParameters *)pvParameters; //pointer for task

    /*********************************/
    /*         Ranging loop          */
    /*********************************/
    
    p->status = vl53l5cx_start_ranging(&(p->Dev));
    
    p->loop = 0;
    while(p->loop < 10)
    {
        p->status = vl53l5cx_check_data_ready(&(p->Dev), &(p->isReady)); //It is an adress hence the &
        
        if(p->isReady)
        {
            vl53l5cx_get_ranging_data(&(p->Dev), &(p->Results));
            
            printf("Print data no: %3u\n", p->Dev.streamcount);
            for(p->i = 0; p->i < 16; p->i++)
            {
                printf("Zone: %3d, Status: %3u, Distance: %4d mm\n",
                       p->i,
                       p->Results.target_status[VL53L5CX_NB_TARGET_PER_ZONE * p->i],
                       p->Results.distance_mm[VL53L5CX_NB_TARGET_PER_ZONE * p->i]);
            }
            printf("\n");
            p->loop++;
        }
        
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    
    p->status = vl53l5cx_stop_ranging(&(p->Dev));
    printf("End of ULD demo\n");
    
    free(p);
    
    // Delete task when done
    vTaskDelete(NULL);
}

void app_main(void)
{
    printf("\n>>>>>> STARTING APP_MAIN <<<<<<\n");

    vTaskDelay(pdMS_TO_TICKS(100));
    
    printf("Initializing VL53L5CX\n");  
    
    //Defining the i2c bus configuration
    i2c_port_t i2c_port = I2C_NUM_1;
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = i2c_port,
        .scl_io_num = 2,
        .sda_io_num = 1,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));
    printf("I2C bus created\n");
    
    //Defining the i2c device configuration
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = VL53L5CX_DEFAULT_I2C_ADDRESS >> 1,
        .scl_speed_hz = VL53L5CX_MAX_CLK_SPEED,
    };
    
    /*********************************/
    /*   VL53L5CX ranging variables  */
    /*********************************/
    
    uint8_t status, isAlive;
    
    // Allocate memory for task parameters
    TaskParameters *pointer = malloc(sizeof(TaskParameters));
    if (pointer == NULL) {
        printf("Failed to allocate memory for task parameters\n");
        return;
    }
    
    // Initialize structures to zero
    memset(pointer, 0, sizeof(TaskParameters));
    
    /*********************************/
    /*      Customer platform        */
    /*********************************/
    
    pointer->Dev.platform.bus_config = i2c_mst_config;
    
    //Register the device
    i2c_master_bus_add_device(bus_handle, &dev_cfg, &(pointer->Dev.platform.handle));
    printf("I2C device registered\n");
    
    /* (Optional) Reset sensor */
    pointer->Dev.platform.reset_gpio = GPIO_NUM_36;
    VL53L5CX_Reset_Sensor(&(pointer->Dev.platform));
    printf("Sensor reset complete\n");
    
    /*********************************/
    /*   Power on sensor and init    */
    /*********************************/
    
    /*
    status = vl53l5cx_is_alive(&(pointer->Dev), &isAlive);
    if(!isAlive || status)
    {
        printf("VL53L5CX not detected at requested address (status=%d, isAlive=%d)\n", status, isAlive);
        free(pointer);
        return;
    }
    printf("Sensor is alive\n");
    */
    
    /* (Mandatory) Init VL53L5CX sensor */
    printf(">>>>Initializing sensor<<<<\n");
    
    status = vl53l5cx_init(&(pointer->Dev));
    
    if(status)
    {
        printf("VL53L5CX ULD Loading failed (status=%d)\n", status);
        free(pointer);
        return;
    }
    
    printf("VL53L5CX ULD ready! (Version: %s)\n", VL53L5CX_API_REVISION);
    
    printf("Creating the task\n");

    BaseType_t result_create;

    result_create = xTaskCreate(
        Vl53l5cxTaskCode,              // Function 
        "Basic ranging of Vl53l5cx",   // Task name
        4096,                          // Stack size --- Change this could be why corrupting
        (void *)pointer,               // Parameters passed to task, in this case is a struct
        5,                             // Priority: 9 is high priority(WiFi) and 1 is low(background tasks)
        NULL                           // Task handle, dm here 
    );
    
    if (result_create == pdPASS) {    //Debugging to see if task was being created, it is 
        printf("Task has been created\n");
    } else {
        printf("Task has not been created\n");
        free(pointer);
    }
    
    printf(">>>>Exiting the app_main<<<<\n\n"); //Output prints this, know it isnt the baud rate now causing the issue
    vTaskDelay(pdMS_TO_TICKS(100));
}
