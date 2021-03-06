/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : syn6288.c
 @brief  : syn6288  audio driver
 @author : gexueyuan
 @history:
           2015-8-6    gexueyuan    Created file
           ...
******************************************************************************/
#include <rthw.h>
#include <rtthread.h>
    
#include "stm32f4xx.h"
#include "board.h"
#include "usart.h"
#include "gpio.h"
#include <rtthread.h>
#include "finsh.h"
#include "stm32f429i_discovery_sdram.h"
#include "stm32f4xx_fmc.h"

#define syn6288_DEVICE_NAME	"uart4"




void audio_io_init(void)
{

    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    /* Sound en enable*/
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    GPIO_SetBits(GPIOE, GPIO_Pin_2);
}
void video_io_init(void)
{

    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    /* Sound en enable*/
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    GPIO_SetBits(GPIOF, GPIO_Pin_10);
}

void play_test(void)
{

    uint8_t test[] = {0xFD,0x00,0x0B,0x01,0x00,0xD3,0xEE,0xD2,0xF4,0xCC,0xEC,0xCF,0xC2,0xC1};
    uint8_t serch[] = {0xFD,0x00,0x02,0x21,0xDE};
    
    rt_device_t dev;
    uint8_t tmp;

    dev = rt_device_find(syn6288_DEVICE_NAME);

    tmp = rt_device_write(dev,0,serch,sizeof(serch));

    if(tmp == sizeof(serch))
        rt_kprintf("play success!!\n");

    rt_kprintf("return %d\n",tmp);
}

FINSH_FUNCTION_EXPORT(play_test, sound test);

void play(char *txt)
{
    char tempbuff[205];
    uint8_t datalength;
    int i = 0;
    int j = 0;
    uint8_t xorcrc=0;
    rt_device_t dev;
    uint8_t tmp;
    RT_ASSERT(txt != NULL);
    
    i = 5;
    
    while(*txt != '\0'){
        
        tempbuff[i++] = *txt++;

    }
    datalength = i - 5;
    
    tempbuff[0] = 0xFD;

    tempbuff[1] = 0x00;

    tempbuff[2] = datalength + 3;
    
    tempbuff[3] = 0x01;
    
    tempbuff[4] = 0x00;

    for(j = 0;j < (datalength + 5);j++){
        xorcrc=xorcrc ^ tempbuff[j];      
    }
    tempbuff[datalength + 5] = xorcrc;
/*
    for(i = 0;i < (datalength + 6);i++){

        rt_kprintf("%X\n",tempbuff[i]);

    }
    */
    dev = rt_device_find(syn6288_DEVICE_NAME);
    rt_device_open(dev,RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM);
    tmp = rt_device_write(dev,0,tempbuff,datalength + 6);
    
    rt_kprintf("return %d\n",tmp);
}


FINSH_FUNCTION_EXPORT(play, input:string);

void rt_audio_thread_entry(void * parameter)
{

	rt_device_t dev ;
    
	dev = rt_device_find(syn6288_DEVICE_NAME);
	rt_device_open(dev, RT_DEVICE_OFLAG_RDWR);
	
	while(1){
        //if(rt_device_read(dev, 0, &tmp, 1) == 1)
            //rt_kprintf("%d\n",tmp);
       }  


}


int audio_init(void)
{

  //  rt_thread_t tid;
    
    audio_io_init();

    /*
    tid = rt_thread_create("audio",
        rt_audio_thread_entry, RT_NULL,
        2048, RT_THREAD_PRIORITY_MAX-2, 20);

    if (tid != RT_NULL)
        rt_thread_startup(tid);
*/
    return 0;

}
#define MT48LC8M16A2_SIZE 0x800000
void sdram_test(void)
{
  uint8_t ubWritedata_8b = 0x3C, ubReaddata_8b = 0;  
  uint16_t uhWritedata_16b = 0x1E5A, uhReaddata_16b = 0;  
  uint32_t uwReadwritestatus = 0;
  uint32_t counter = 0x0;
  /* Erase SDRAM memory */
  for (counter = 0x00; counter < MT48LC8M16A2_SIZE; counter++)
  {
    *(__IO uint8_t*) (SDRAM_BANK_ADDR + counter) = (uint8_t)0x0;
  }
  
  /* Write data value to all SDRAM memory */
  for (counter = 0; counter < MT48LC8M16A2_SIZE; counter++)
  {
  //rt_enter_critical();
    *(__IO uint8_t*) (SDRAM_BANK_ADDR + counter) = (uint8_t)(ubWritedata_8b + counter);
   // rt_exit_critical();
  }
  
  /* Read back SDRAM memory and check content correctness*/
  counter = 0;
  uwReadwritestatus = 0;
  while ((counter < MT48LC8M16A2_SIZE) && (uwReadwritestatus == 0))
  {
      //rt_enter_critical();
    ubReaddata_8b = *(__IO uint8_t*)(SDRAM_BANK_ADDR + counter);
    //rt_exit_critical();
    if ( ubReaddata_8b != (uint8_t)(ubWritedata_8b + counter))
    {
      uwReadwritestatus = 1;

      rt_kprintf("counter is %X %d:%d",counter,ubReaddata_8b,(ubWritedata_8b + counter));
     // STM_EVAL_LEDOn(LED4);
      //STM_EVAL_LEDOff(LED3);              
    }
    else
    {
      //STM_EVAL_LEDOn(LED3);
      //STM_EVAL_LEDOff(LED4);
    }
    counter++;
  } 
  
  if(uwReadwritestatus == 0)
  {
    rt_kprintf("  8-bits AHB      \n");
    rt_kprintf("  Transaction     \n"); 
    rt_kprintf("  Test-> OK       \n");   
  }
  else
  {
    rt_kprintf("    8-bits AHB     \n");
    rt_kprintf("   Transaction     \n"); 
    rt_kprintf("   Test-> NOT OK   \n");     
  }
  
  /*********************** 16-bits AHB transaction test ***********************/    
  /* Wait for User button to be pressed */
 // while (STM_EVAL_PBGetState(BUTTON_USER) != Bit_SET)
  {}
  /* Wait for User button is released */
 // while (STM_EVAL_PBGetState(BUTTON_USER) != Bit_RESET)
  {}
  
  /* Turn Off Leds */   
  //STM_EVAL_LEDOff(LED3);
  //STM_EVAL_LEDOff(LED4);
  
  /* Erase SDRAM memory */
  for (counter = 0x00; counter < MT48LC8M16A2_SIZE; counter++)
  {
    *(__IO uint16_t*) (SDRAM_BANK_ADDR + 2*counter) = (uint16_t)0x00;
  }
  
  /* Write data value to all SDRAM memory */
  for (counter = 0; counter < MT48LC8M16A2_SIZE; counter++)
  {
    *(__IO uint16_t*) (SDRAM_BANK_ADDR + 2*counter) = (uint16_t)(uhWritedata_16b + counter);
  }
  
  /* Read back SDRAM memory and check content correctness*/
  counter = 0;
  uwReadwritestatus = 0;
  while ((counter < MT48LC8M16A2_SIZE) && (uwReadwritestatus == 0))
  {
    uhReaddata_16b = *(__IO uint16_t*)(SDRAM_BANK_ADDR + 2*counter);
    if ( uhReaddata_16b != (uint16_t)(uhWritedata_16b + counter))
    {
      uwReadwritestatus = 1;
      //STM_EVAL_LEDOn(LED4);
      //STM_EVAL_LEDOff(LED3);
      
      rt_kprintf("counter is %X %d:%d",counter,ubReaddata_8b,(ubWritedata_8b + counter));
    }
    else
    {
      //STM_EVAL_LEDOn(LED3);
      //STM_EVAL_LEDOff(LED4);
    }
    counter++;
  }
  if(uwReadwritestatus == 0)
  {
    rt_kprintf("  16-bits AHB     \n");
    rt_kprintf("  Transaction     \n"); 
    rt_kprintf("  Test-> OK       \n");   
  }
  else
  {
    rt_kprintf("    16-bits AHB    \n");
    rt_kprintf("   Transaction     \n"); 
    rt_kprintf("   Test-> NOT OK   \n");     
  }
  
}

FINSH_FUNCTION_EXPORT(sdram_test, input:string);

