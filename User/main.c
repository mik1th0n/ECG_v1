/**
  ******************************************************************************
  */
 
#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "adc.h"
#include "bsp_gpio.h"
#include "bsp_usart1.h"
#include "bsp_usart2.h"
#include "bsp_usart3.h"
#include "bsp_uart4.h"
#include "bsp_uart5.h"
#include "bsp_SysTick.h"
#include "dma.h"
#include "iwdg.h"
#include "SPI.h"
#include "delay.h"
#include "timers.h"
#include "timer4_cap.h"
#include "NVIC_CONFIG.H"
#include "cJSON.h"
#include "stm_flash.h"
#include "PWM.h"
#include "CommunicationConfig.h"
#include "CommunicationProtocol.h"

#include "ESP8266_config.h"
#include "ESP8266_function.h"
#include "E30TTLUART.h"
#include "W5500.h"


#include "contiki-conf.h"
#include <stdint.h>
#include <stdio.h>
#include <debug-uart.h>
#include <process.h>
#include <procinit.h>
#include <etimer.h>
#include <autostart.h>
#include <clock.h>
#include "contiki_delay.h"
#include "ProcessTask.h"


void BSP_Config(void)
{
    /* 初始化 */
    delay_init();
    clock_init();
    srand(STMFLASH_Read_OneWordData(STM32_FLASH_END_PAGE));
    STMFLASH_Write_OneWordData(STM32_FLASH_END_PAGE,rand());
    NVIC_Configuration_Init();
    USART1_Config(921600);
    printf("Start Contiki OS\r\n");
#ifdef __LED_BLINK_ON__
    LED_GPIO_Config(); 
#endif

#ifdef __W5500_MODULE_ON__
    W5500_Init();
#endif    

#ifdef __WIFI_MODULE_ON__
    WiFi_Config();
    ESP8266_STA_TCP_Client();
    ChangeUSART2ReceiveMode();// 关闭串口2空闲中断 使能串口2接收中断 
#endif  

#ifdef __E30TTLUART_MODULE_ON__
		E30TTLUART_Init();
    E30TTLUART_MultiCountConfig(0x0002,0x50,DISABLE,5);		
    printf("E30-TTL-100 OK.\r\n");
#endif

}


int main(void)
{
		
    BSP_Config();    
    printf("hello world.\r\n");
    IWDG_Start(2);  // wifi模块透传之后开启看门狗
    process_init();
    autostart_start(autostart_processes);

#ifdef __LED_BLINK_ON__
    process_start(&red_blink_process,NULL);
    process_start(&green_blink_process,NULL);
#endif    
    
#ifdef __CJSON_LIB_TEST__
    process_start(&cJSON_test_process,NULL);
#endif

#ifdef __COMMUNICAT_PROTOCOL__
    process_start(&Communication_Protocol_Load_process,NULL);
    process_start(&Communication_Protocol_Send_process,NULL);
#endif  
        
#ifdef __CLOCK_TICK_TEST__
    process_start(&clock_test_process,NULL);
#endif    

#ifdef __WIFI_MODULE_TEST__     
    process_start(&wifi_send_test_process,NULL);
#endif
    
#ifdef __COMMUNICAT_PROTOCOL_SENSOR_DATA__
    process_start(&CommunicatProtocol_Send_Sensor_Data,NULL);
#endif

#ifdef __W5500_SEND_TEST_ON__
    process_start(&W5500_send_test_process,NULL);
#endif

    while (1)
    {
        do
        {
#ifdef __W5500_MODULE_ON__
            W5500_Daemon_Process();
#endif  
        }while (process_run()>0);
    }
}



