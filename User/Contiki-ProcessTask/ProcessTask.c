#include "ProcessTask.h"


//PROCESS(red_blink_process, "Red Blink");
PROCESS(green_blink_process, "Green Blink");
PROCESS(IWDG_Feed_process, "Timing to feed dog");
PROCESS(clock_test_process, "Test system delay");
PROCESS(cJSON_test_process, "Test cJSON Lib");
PROCESS(Communication_Protocol_Send_process, "Communication protocol send packet serviced");
PROCESS(Communication_Protocol_Load_process, "Communication protocol load bytes to packet serviced");
PROCESS(CommunicatProtocol_Send_Sensor_Data, "Communication protocol send sensor data");
PROCESS(LCD_display_waveform_process, "LCD module display waveform data");


PROCESS(wifi_send_test_process, "Wifi module send data test");
PROCESS(W5500_send_test_process, "Test W5500 module send data");


AUTOSTART_PROCESSES(&etimer_process,&IWDG_Feed_process);

/* 此处应和网页的用户信息=》传感器类型的名字相匹配 */
float lightIntensityGlobalData;
uint32_t CardID_GlobalData;

/*******************PROCESS************************/

PROCESS_THREAD(green_blink_process, ev, data)
{
    static struct etimer et;
    PROCESS_BEGIN();
    while(1)
    {        
        Contiki_etimer_DelayMS(200);
        LED_Green_Off();
        Contiki_etimer_DelayMS(200);
        LED_Green_On();
    }
    PROCESS_END();
}

PROCESS_THREAD(wifi_send_test_process, ev, data)
{
    uint8_t *USART2_SendBuff;
    static struct etimer et;
    PROCESS_BEGIN();
    while(1)
    {
        
        Contiki_etimer_DelayMS(500);
        USART2_SendBuff = " asdfafasdf";
        SendUSART2BytesBuf(USART2_SendBuff, 11);
        
        Contiki_etimer_DelayMS(500);        
        USART2_SendBuff = " 21398416hy";
        SendUSART2BytesBuf(USART2_SendBuff, 11);
    }
    PROCESS_END();
}

PROCESS_THREAD(W5500_send_test_process, ev, data)
{
    static struct etimer et;
    PROCESS_BEGIN();
    while(1)
    {
        W5500_Push_Socket0_SendDataIntoFIFO("Hello W5500 is run!\r\n", 21);
        W5500_Push_Socket0_SendDataIntoFIFO(" Love Live Rewrite Fate/Zreo Angel Beats!\r\n", 43);
        W5500_Push_Socket0_SendDataIntoFIFO("ABBB1234BB345634BBBBCC\r\n", 24);
        W5500_Push_Socket0_SendDataIntoFIFO("Kggggg5678ggggggggggPP\r\n", 24);
        W5500_Push_Socket0_SendDataIntoFIFO("TianTianTianMaoMaoMaoMao\r\n", 26);
        Contiki_etimer_DelayMS(500);
    }
    PROCESS_END();
}

PROCESS_THREAD(LCD_display_waveform_process, ev, data)
{
    static struct etimer et;
		float temp1;
    PROCESS_BEGIN();
		while (1)
		{
			  Get_Adc_Val();
//				printf("aaa.\n");
				Draw_Oscillogram();									// 画波形
				temp1=Get_Vpp();										// 峰峰值mv	
				LCD_ShowxNum(49,210,temp1,4,24,0);	// 显示峰峰值mv			
//				printf("bbb.\n");
				Contiki_etimer_DelayMS(1);
		}
    PROCESS_END();
}

PROCESS_THREAD(clock_test_process, ev, data)
{
    static uint16_t i,start_count,end_count,diff;
    PROCESS_BEGIN();

    printf("Clock delay test, (10,000 x i) cycles:\n");
    i = 1;
    while(i<16)
    {
        start_count = clock_time();                   // 记录开始timer
        Delay_NOP_ms(10 * i);                       // 软件延时
        end_count = clock_time();                     // 记录结束timer
        diff = end_count - start_count;               // 计算差值，单位为tick
        printf("Delayed %u \n%u ticks =~ %u ms\n", 10 * i, diff, diff);
        i++;
    }

    printf("\r\nDone!\r\n");

    PROCESS_END();
}

PROCESS_THREAD(cJSON_test_process, ev, data)
{

    cJSON *root;char* cJSONout;
    PROCESS_BEGIN();

	root=cJSON_CreateObject();	
    
	cJSON_AddItemToObject(root, "Device", cJSON_CreateString("ContikiOS on STM32F103"));
    cJSON_AddItemToObject(root, "Address", cJSON_CreateNumber(0xFFFF));
    cJSON_AddItemToObject(root, "InfoType", cJSON_CreateString("Information"));
	cJSON_AddItemToObject(root, "DataName", cJSON_CreateNull());

    cJSONout = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);	
    AssembleProtocolPacketPushSendQueue(0x0000, FunctionWord_StartUP, strlen(cJSONout), (uint8_t*)cJSONout);
    free(cJSONout);
    PROCESS_END();
}


PROCESS_THREAD(CommunicatProtocol_Send_Sensor_Data, ev, data)
{
    static struct etimer et;
		static uint32_t count = 0;
    cJSON *root;char* cJSONout;
    PROCESS_BEGIN();
    Contiki_etimer_DelayMS(2000);
    while(1)
    {
//				printf("asd.\r\n");
        root=cJSON_CreateObject();	
        cJSON_AddItemToObject(root, "InfoType", cJSON_CreateString("Data"));
        cJSON_AddItemToObject(root, "Owner", cJSON_CreateString("admin"));

        cJSON_AddItemToObject(root, "Address", cJSON_CreateNumber(Protocol_LocalhostAddress));

//#ifdef __SDS01_MODULE_ON__
//        cJSON_AddItemToObject(root, "PM2_5", cJSON_CreateNumber(PM2_5_GlobalData));
//        cJSON_AddItemToObject(root, "PM10", cJSON_CreateNumber(PM10_GlobalData));
//#endif

        cJSONout = cJSON_PrintUnformatted(root);
        cJSON_Delete(root);
				// 发送队列的地址：0x0001(不能改)发送到主机服务器
        AssembleProtocolPacketPushSendQueue(0x0001, FunctionWord_Data, strlen(cJSONout), (uint8_t*)cJSONout);		
//				printf("Temperature = %f.\n", temperatureGlobalData);
				printf("count = %d\n", count++);
        Contiki_etimer_DelayMS(2000);
    }
    PROCESS_END();
}

PROCESS_THREAD(Communication_Protocol_Load_process, ev, data)
{
    static struct etimer et;
    PROCESS_BEGIN();
    while(1)
    {
        Contiki_etimer_DelayMS(10);
        LoadReceiveQueueByteToPacketBlock();
        DealWithReceivePacketQueue();
    }
    PROCESS_END();
}

PROCESS_THREAD(Communication_Protocol_Send_process, ev, data)
{
    static struct etimer et;
    PROCESS_BEGIN();
    while(1)
    {
        Contiki_etimer_DelayMS(100);
        SendUnsentPacketQueue();
        SendUnackedPacketQueue();
//        IncreaseUnackedPacketQueueResendTime();
    }
    PROCESS_END();
}

PROCESS_THREAD(IWDG_Feed_process, ev, data)
{
    static struct etimer et;
    PROCESS_BEGIN();
    while(1)
    {
        IWDG_Feed();
        Contiki_etimer_DelayMS(1000);
    }
    PROCESS_END();
}







