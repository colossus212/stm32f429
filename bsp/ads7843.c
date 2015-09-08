#ifndef TOUCH_H
#define TOUCH_H
#define SPI   0           //ͨ���궨����ѡ��SPI����������IO��ģ��
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_spi.h"
#include "math.h"
#include "TFT.h"
#define TCS_HIGH     GPIO_SetBits(GPIOB,GPIO_Pin_12)   // NSS Soft Mode 
#define TCS_LOW      GPIO_ResetBits(GPIOB,GPIO_Pin_12) // NSS Soft Mode
#define PEN          GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0) //INT state
#define TOUT         GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14)
#define TDIN_HIGH    GPIO_SetBits(GPIOB,GPIO_Pin_15)
#define TDIN_LOW     GPIO_ResetBits(GPIOB,GPIO_Pin_15)
#define TCLK_HIGH    GPIO_SetBits(GPIOB,GPIO_Pin_13)
#define TCLK_LOW     GPIO_ResetBits(GPIOB,GPIO_Pin_13)
#define TXMAX        4000         //�����趨��У׼ֵ��������Χ ������Ч
#define TYMAX        4000
#define TXMIN        100
#define TYMIN        100
#define SCREEN_W      240
#define SCREEN_H      320
#define ERROR_RANGE   100
#define KEY_UP        0x01 
#define KEY_DOWN      0x00
#define CHX      0xd0
#define CHY      0x90
#define EXTI_ENABLE		     EXTI->IMR|=0X0001          //������0�ж�
#define EXTI_DISABLE       EXTI->IMR&=~0X0001        //�ر���0�ж�         //ads7843оƬ�ڵ�һ��ʱ�ӵ����������룬��һ��ʱ�ӵ�
typedef struct sldkf                         //�½������������SPIҪ����Ϊ1ʱ�ӣ������ء��ڶ�ȡ��ʱ��
{                                            //Ҳ����1ʱ�ӣ������ض�ȡ����ads�����½�����������ԣ���һ��ֵ���Ƿ�ֵ
	u16 x0,y0;     //ԭʼ����,��ADֵ         //��������3λ
	u16 x,y;       //�������꣬���ص�ֵ
	u8  flag;      //��ǰ״̬����ʵ�����ж��жϷ����ı�־
	float xfac,yfac,xoff,yoff; //ƫ�Ʋ���
}Hand;
Hand pence;
u16 TX,TY;
int ABS(int x)
{
	return x>0?x:-x;
}
void Touch_SPI_inti()  //SPI����ADS���õ��ĳ�ʼ��
{
	SPI_InitTypeDef spi;
	GPIO_InitTypeDef gpio;
	NVIC_InitTypeDef nvic;
	EXTI_InitTypeDef exit;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	gpio.GPIO_Pin=GPIO_Pin_12;  //nss �������
	gpio.GPIO_Mode=GPIO_Mode_Out_PP;
	gpio.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&gpio);
	
	gpio.GPIO_Pin=GPIO_Pin_13|GPIO_Pin_15; // sck��mosi �����������
	gpio.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB,&gpio);
	
	gpio.GPIO_Pin=GPIO_Pin_14;             //miso ��������
	gpio.GPIO_Mode=GPIO_Mode_IPU;
	//	gpio.GPIO_Mode=GPIO_Mode_IN_FLOATING;//miso ����Ϊ������������߸�������Ч��û��𣬲���
	GPIO_Init(GPIOB,&gpio);	
	
	SPI_Cmd(SPI2,DISABLE);
	spi.SPI_Direction=SPI_Direction_2Lines_FullDuplex;  //ȫ˫��
	spi.SPI_Mode=SPI_Mode_Master;                       //��ģʽ 
	spi.SPI_DataSize=SPI_DataSize_8b;                  //����16λ 
	spi.SPI_CPOL=SPI_CPOL_Low;                         //ʱ�ӿ��иߵ�Դ
	spi.SPI_CPHA=SPI_CPHA_1Edge;                        //1�����ز�׽
	spi.SPI_NSS=SPI_NSS_Soft;                           //NSS���ģʽ?
	spi.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_64;    //64��Ƶ��
	spi.SPI_FirstBit=SPI_FirstBit_MSB;                  //��λ��ǰ
	spi.SPI_CRCPolynomial=7;
	SPI_Init(SPI2,&spi);
	SPI_Cmd(SPI2,ENABLE);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource0); //ѡ��PB.0��Ϊ�ж�0������
	
	exit.EXTI_Line=EXTI_Line0;               //�ⲿ�ж�0
	exit.EXTI_Mode=EXTI_Mode_Interrupt;       //�ж�
	exit.EXTI_Trigger=EXTI_Trigger_Falling;   //�½��ش���
	exit.EXTI_LineCmd=ENABLE;
	EXTI_Init(&exit);
	
	nvic.NVIC_IRQChannel=EXTI0_IRQChannel;  //nvic �ж�����
 	nvic.NVIC_IRQChannelPreemptionPriority=0;
	nvic.NVIC_IRQChannelSubPriority=2;
	nvic.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&nvic);
}
u8 SPI2_Byte(u8 cmd)
{
	while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_TXE)==RESET);
	
	SPI_I2S_SendData(SPI2,cmd);
	
	while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_RXNE)==RESET);
	
	return SPI_I2S_ReceiveData(SPI2);
}
void Touch_IO_inti()  //IO ��ģ��
{
	GPIO_InitTypeDef gpio;
	NVIC_InitTypeDef nvic;
	EXTI_InitTypeDef exit;

//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB||RCC_APB2Periph_AFIO,ENABLE); //����һ���ã�����ԭ��֪��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	
	
	gpio.GPIO_Pin=GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_15; //cs,clk,mosi
	gpio.GPIO_Mode=GPIO_Mode_Out_PP;
	gpio.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&gpio);
	
	gpio.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_14;     //�������� ��spi�����Ѿ�ʹ����PB��ʱ��,int,miso 
	gpio.GPIO_Mode=GPIO_Mode_IPU;
	gpio.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&gpio);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource0); //ѡ��PB.0��Ϊ�ж�0������
	
	exit.EXTI_Line=EXTI_Line0;               //�ⲿ�ж�0
	exit.EXTI_Mode=EXTI_Mode_Interrupt;       //�ж�
	exit.EXTI_Trigger=EXTI_Trigger_Falling;   //�½��ش���
	exit.EXTI_LineCmd=ENABLE;
	EXTI_Init(&exit);
	
	nvic.NVIC_IRQChannel=EXTI0_IRQChannel;  //nvic �ж�����
 	nvic.NVIC_IRQChannelPreemptionPriority=0;
	nvic.NVIC_IRQChannelSubPriority=2;
	nvic.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&nvic);
}
#if SPI
void CMD_Write(u8 cmd)   //SPI д
{
	SPI2_Byte(cmd);
}
u16 CMD_Read()          //SPi ��
{
	u16 ans=0,temp;
	temp=SPI2_Byte(0x00);
	ans=temp<<8;
	temp=SPI2_Byte(0x00);
	ans|=temp;
	ans>>=3;
	return ans&0x0fff;
}
#else 
void CMD_Write(u8 num)     //IO ģ��
{  
	u8 count=0;   
	for(count=0;count<8;count++)  
	{ 	  
		if(num&0x80)TDIN_HIGH;  
		else TDIN_LOW;   
		num<<=1;    
		TCLK_LOW;//��������Ч	   	 
		TCLK_HIGH;      
	} 			    
}

u16 CMD_Read()      //IO ģ��
{
	u16 i,ans;
	ans=0;
	for(i=0;i<12;i++)
	{
		ans<<=1;
		TCLK_HIGH;
		TCLK_LOW;
		if(TOUT)ans++;
	}
	return ans;
}
#endif
u8 Read_IO_ADS()  //��ȡһ��
{
	TCS_LOW;
	
	CMD_Write(CHX);   
	#if SPI          //��SPI��˵������������̫���Ǳ��س���ɢ���ԭ��
	SysTick_us(30); //��SPI��˵��Ƶ��Ϊ64��Ƶʱ�����ʵ�����ʱ���Դﵽ�ܺõ�Ч�����õ�SPI���������Բ�Ҫ���½������BUSY
	                //Ӳ����������ʱ������ϲ��ÿ��ǣ�ΨһҪ���ǵ���SPI��Ƶ�ʹ��ߣ����ܵ��²�����׼
	               //���ԣ�SPI��ʼ����ɺ��ʱ���ǲ��ÿ��ǵģ���ads��ʱ��ͼ��Ҳ���Կ�����һ��
	#else    //IOģ�����Ҫ�½��أ�ԭ����,�½��غ�Ч���úܶ�
	         //���û�У���Ļ��������ϼ��������Ͻ�1/4����?
	SysTick_us(10);//�����о�ʱ��ͼ���֣��˴���һ���½��أ���Ϊ�����BUSY��־��ͬʱ��������
 	TCLK_HIGH;     //���û�У���ô��CMD_Read��������Ķ��������ĵ�12λ���Ϊ0
	SysTick_us(10);//���������ˣ�ΪʲôX,Y�ķ�Χ������롣��CMD_Read�������棬�ڵ�һ���½��ع������ݲſ�ʼ����
 	TCLK_LOW;     //���ԣ���һ���½��ص����þ����������������BUSY��־�������ڵ�һ���½��صõ������϶�Ϊ0����Ϊ�ոտ�ʼ����
	SysTick_us(10);
	#endif
	TX=CMD_Read();
	
	CMD_Write(CHY);
	#if  SPI
	SysTick_us(30);
	#else         //<span style="background-color: rgb(255, 255, 255); ">IOģ�����Ҫ�½��أ�ԭ����</span>

	
	SysTick_us(10);
 	TCLK_HIGH;
	SysTick_us(10);
 	TCLK_LOW;
	SysTick_us(10);
	#endif
	TY=CMD_Read();
	
	TCS_HIGH;
	
	if(TX>TXMAX||TY>TXMAX||TX<TXMIN||TY<TYMIN)
	{
		return 0;
	}
	return 1;
}
void Read_IO_XY(u16 *x,u16 *y)
{
	u16 xy[2][10],cnt,i,j,temp,t,XY[2];
	cnt=0;
	do                    //�ɼ�10������ȡ�м��4��
	{
		if(Read_IO_ADS()) //�ɼ�10�κϷ�����
		{
			xy[0][cnt]=TX;
			xy[1][cnt]=TY;
			cnt++;
		}
	}while(PEN==0&&cnt<9);
	if(cnt<9)
	{
		return ;
	}
	for(t=0;t<2;t++)
	{
		for(i=0;i<cnt;i++)   //ѡ������ 
		{
			temp=i;
			for(j=i+1;j<cnt;j++)
			{
				if(xy[t][j]<xy[t][temp])
				{
					temp=j;
				}
			}
			if(temp!=i)
			{
				xy[t][i]^=xy[t][temp];
				xy[t][temp]^=xy[t][i];
				xy[t][i]^=xy[t][temp];
			}
		}
		XY[t]=(xy[t][3]+xy[t][4]+xy[t][5]+xy[t][6])/4;
	}
	*x=XY[0];
	*y=XY[1];
}
u8 GetXY(u16 *x,u16 *y)
{
	u16 x1,y1,x2,y2;
	Read_IO_XY(&x1,&y1);
	Read_IO_XY(&x2,&y2);
	if(ABS(x1-x2)>ERROR_RANGE||ABS(y1-y2)>ERROR_RANGE)//�������������ȥ��
	{
		return 0;
	}
	*x=(x1+x2)/2;
	*y=(y1+y2)/2;
	return 1;
}
u8 Read_TP()
{
	u8 t=0;
	EXTI_DISABLE;                //�жϹر�
	pence.flag=KEY_UP;           //״̬�ı� 
	GetXY(&pence.x0,&pence.y0);
	while(t<250&&PEN==0)
	{
		t++;
		SysTick_ms(10);
	}
	EXTI_ENABLE;
	if(t>=250)return 0;
	return 1;
}
void Touch_correct() //У׼�����ܵõ�һ�������ϱȽ�׼ȷ��ֵ�����ǻ��ǻ���Щ���
{                   //��������õ���ֵ���Ǹ���ʵ��������ɴ˺����õ���ֵ�޸Ķ������޸ĵķ�Χ�Ǻ�С��
       //����У׼�õ���ֵ��xfac=0.0771,xoff=-18.1920,yfac=0.1014,yoff=-2.8338
	   
                   //�޸�Ϊ xfac=0.0671,xoff=-18.1920,yfac=0.0914,yoff=-18.8
	int x[4],y[4],cnt,temp1,temp2,ans;
	float d1,d2;
	TFT_Pant(BLUE);          //��һ��У׼��
	Draw_Touch_Point(20,20);  
	pence.flag=KEY_UP;          //�����־
	cnt=0;
	while(1)
	{
		if(pence.flag==KEY_DOWN) //��Ļ���� 
		{
			if(Read_TP())      //��ȡADֵ
			{
				x[cnt]=pence.x0;
				y[cnt]=pence.y0;
				cnt++;
			}
			switch(cnt)
			{
				case 1:
					TFT_Pant(BLUE);             //��2��У׼��
					Draw_Touch_Point(220,20);
					break;
				case 2:
					TFT_Pant(BLUE);
					Draw_Touch_Point(20,300);//��3��?
				  break;
				case 3:
					TFT_Pant(BLUE);
					Draw_Touch_Point(220,300); //��4��
					break;
				case 4:                  //У׼����ϣ����кϷ��Լ��
					temp1=ABS(x[0]-x[1]);
					temp2=ABS(y[0]-y[1]);
					d1=sqrt(temp1*temp1+temp2*temp2); //1,2��ľ���
				
				  temp1=ABS(x[2]-x[3]);             //3.4��ľ���
					temp2=ABS(y[2]-y[3]);
					d2=sqrt(temp1*temp1+temp2*temp2);
					if(d1==0||d2==0||d1/d2>1.05||d1/d2<0.95) //�����̫��
					{
						cnt=0;
						TFT_Pant(BLACK);
						TFT_ShowString(35,100,"Adjust Failed(1)!!!");
						SysTick_ms(1000);
						TFT_Pant(BLUE);
						Draw_Touch_Point(20,20);
					//	continue;
						break;
					}
					
					temp1=ABS(x[0]-x[2]);
					temp2=ABS(y[0]-y[2]);
					d1=sqrt(temp1*temp1+temp2*temp2); //1,3��ľ���	
					temp1=ABS(x[1]-x[3]);             //2.4��ľ���
					temp2=ABS(y[1]-y[3]);
					d2=sqrt(temp1*temp1+temp2*temp2);					
					if(d1==0||d2==0||d1/d2>1.05||d1/d2<0.95) //�����̫��
					{
						cnt=0;
						TFT_Pant(BLACK);
						TFT_ShowString(35,100,"Adjust Failed(2)!!!");
						SysTick_ms(1000);
						TFT_Pant(BLUE);
						Draw_Touch_Point(20,20);
					//	continue;
						break;
					}
					
					temp1=ABS(x[0]-x[3]);
					temp2=ABS(y[0]-y[3]);
					d1=sqrt(temp1*temp1+temp2*temp2); //1,4��ľ���	
					temp1=ABS(x[1]-x[2]);             //2.3��ľ���
					temp2=ABS(y[1]-y[2]);
					d2=sqrt(temp1*temp1+temp2*temp2);					
					if(d1==0||d2==0||d1/d2>1.05||d1/d2<0.95) //�����̫��
					{
						cnt=0;
						TFT_Pant(BLACK);
						TFT_ShowString(35,100,"Adjust Failed(3)!!!");
						SysTick_ms(1000);
						Draw_Touch_Point(20,20);
					//	continue;
						break;
					}
					//���������ϣ���ָ����û��̫������
					pence.xfac=200.0/(x[1]-x[0]);               //��2Ԫ1�η�����
					pence.xoff=(240.0-pence.xfac*(x[1]+x[0]))/2;
					pence.yfac=280.0/(y[2]-y[0]);
					pence.yoff=(320.0-pence.yfac*(y[0]+y[2]))/2;
					TFT_Pant(BLUE);
					TFT_ShowString(35,100,"Touch Screen Adjust OK!");
					SysTick_ms(1000);
					TFT_Pant(WHITE);
					TFT_ShowNum(0,180,x[0]);
					TFT_ShowNum(0+CHARSIZE_W*5,180,y[0]);
					TFT_ShowNum(0,200,x[1]);
					TFT_ShowNum(0+CHARSIZE_W*5,200,y[1]);
					TFT_ShowNum(0,220,x[2]);
					TFT_ShowNum(0+CHARSIZE_W*5,220,y[2]);
					TFT_ShowNum(0,240,x[3]);
					TFT_ShowNum(0+CHARSIZE_W*5,240,y[3]);
					TFT_ShowFloat(100,100,pence.xfac);
					TFT_ShowFloat(100,120,pence.xoff);
					TFT_ShowFloat(100,140,pence.yfac);
					TFT_ShowFloat(100,160,pence.yoff);
					return ;
					default :break;
			}
		}
	}
}
void ConvertXY()
{
	float x,y;
	GetXY(&pence.x0,&pence.y0); //����С��0
	x=pence.x0*pence.xfac+pence.xoff;
	y=pence.y0*pence.yfac+pence.yoff;
	pence.x=(u16)x;
	pence.y=(u16)y;
}
void Pence_inti()
{
	pence.flag=KEY_UP;
	pence.x=pence.y=0;
	pence.x0=pence.y0=0;
	pence.xfac=pence.xoff=0;
	pence.yfac=pence.yoff=0;
}
void Pence_adjust() //�������У׼�˵ıȽϾ�ȷ�Ĳ���
{
	pence.xfac=0.0671;
	pence.xoff=-18.1920;
	pence.yfac=0.0914;
	pence.yoff=-18.8;
}
void Touch_inti() //ͨ����SPI����ѡ��Ӳ��SPI����as,����IO��ģ��
{                //ʵ������˵����SPI��û��Ҫ�ģ����������ٶ�Ҫ�󲻸�
	Pence_inti();  //�����SPI����ô��Ƶϵ��С�ˣ�������Read_IO_ADS�����е���ʱ
	#if SPI        //���ˣ��ᵼ�²ɼ����ĵ���࣬�����ɢ�������
	               //�����������飬����64��Ƶ����ʱ30us��Ч������
      	Touch_SPI_inti();  //SPI
	#else
	      Touch_IO_inti();  //IOģ��
	#endif
	Pence_adjust();
//	Touch_correct();
}
void EXTI0_IRQHandler()
{
	static u32 count=0;
	if(EXTI_GetITStatus(EXTI_Line0)==SET)
	{
		count++;
		EXTI_ClearITPendingBit(EXTI_Line0);
		pence.flag=KEY_DOWN;    //��Ļ����
		SysTick_ms(50);        //��ʱ����
	}
}
#endif
	   
	   
	   
	   
	   
	   
	   
	   