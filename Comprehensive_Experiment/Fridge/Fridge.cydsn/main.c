/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include <project.h>
#include "stdio.h"
#include <string.h>
#include <math.h>
/* Define constants for capsense button and slider */
#define ON           (1)
#define OFF          (0)
#define NO_FINGER    (0xFF)
#define PUSH (1)
#define Release (0)
#define BUFFER_LEN  64u

#if defined (__GNUC__)
    /* Add an explicit reference to the floating point printf library */
    /* to allow the usage of floating point conversion specifiers. */
    /* This is not linked in by default with the newlib-nano library. */
    asm (".global _printf_float");
#endif

#define BUFFER_LEN  64u

char8 *parity[] = { "None", "Odd", "Even", "Mark", "Space" };
char8 *stop[] = { "1", "1.5", "2" };
uint8 interruptCnt = 0;
uint8 UART_GetString(char8 str[],char8 EOM)
{
    uint8 i =0;
    char8 data;
    while ((data = UART_GetChar()) != 0x0D)
    {
        if(!data)
            return 0;
    }
    while((data = UART_GetChar()) != 0x0A);
    
    while(EOM != (data = UART_GetChar()))
    {
        if( data != 0x0D && data != 0x0A && data)
        {
            str[i++] = data;
        }
    }
    return i;  
}
typedef struct PID
{
float SetPoint; // 设定目标 Desired Value
float Proportion; // 比例系数 Proportional Const
float Integral; // 积分系数 Integral Const
float Derivative; // 微分系数 Derivative Const
int LastError; // 上次偏差
int SumError; // 历史误差累计值
int PrevError;
} PID;

PID stPID; // 定义一个stPID 变量

float PIDCalc( PID *pp, int NextPoint )
{
int dError,Error;
Error = pp->SetPoint - NextPoint; // 偏差，设定值减去当前采样值
dError = Error-pp->LastError; // 当前微分，偏差相减
pp->PrevError = pp->LastError; // 保存
pp->LastError = Error;
return (pp->Proportion * Error // 比例项
- pp->Derivative * dError // 微分项
);
}

void convert2string(char *temperature_str , float32 temperature)
{
	uint16 unit;
    uint16 ten;
    uint16 tens;
	ten = temperature/10;
	unit = temperature - ten*10;
	tens = (temperature - ten*10 - unit)*10;
	temperature_str[0] = ten+48;
	temperature_str[1] = unit+48;
	temperature_str[2] = '.';
	temperature_str[3] = tens+48;
	temperature_str[4] = ' ';
	temperature_str[5] = 'C';
}

float convert2float(unsigned char *str)
{
    int a,b,c;
    float fnum;
    a = str[0]-48;
    b = str[1]-48;
    c = str[3]-48;
    fnum = a*10 + b + (float)c/10;
    return fnum;
}

uint8 convString2Int(unsigned char *str)
{
    uint8 num;
    uint8 a,b,c;
    a = str[0] - 48;
    b = str[1] - 48;
    num = a * 10+ b;
    if(str[3]=='#')
    {
        c = str[2]-48;
        num = num*10 + c;
    }
    return num;
}

int cnt = 0;
int on_off = 0;
char8 blue_rec;
int i = 0;

int power_on = 1; // 1为启动，0为关机
int main()
{
    uint8 status_Button_up = OFF;
    uint8 status_Button_down = OFF;
    uint16 sliderPosition = NO_FINGER;
    uint8 pushState_Button_up = Release;
    uint8 pushState_Button_down = Release;
    uint16 LastsliderPosition = NO_FINGER;
    
    float set_temperatute = 28.0;
    float temperature = 25.0;
    float last_set_temp = set_temperatute;
    float last_temp = temperature;
    int16 voltageRawCount = 0;
    
    float i_temp[50] = {0};
    float ave_temp = 0;
    
    uint8 command_str[100];
    int16 command_count;
    int feedback = 0;
    
    int is_calling = 0;
    stPID.LastError = 0;
    
   char8 Message1[] = {"Current Temperature: "};
   char8 Message2[] = {" Set Temperature: "};
   char8 Message3[] = {" Please come back."};
    CyGlobalIntEnable; /* Enable global interrupts. */
    PWM_1_Start();
    ADC_Start();
    LCD_Start();
    CapSense_Start();
    UART_Start();
    UART_Blue_Start();
    
    char8 temp[10]="25.0 C";
    char8 temp_set[10] = "28.0 C";
    
    char8 buffer[100];
    LCD_Position(0u,0u);
    LCD_PrintString("Current:");
    LCD_Position(1u,0u);
    LCD_PrintString("Set:");
    
    CapSense_InitializeAllBaselines();
    /* Start USBFS Operation with 3V operation */
    USBUART_1_Start(0u, USBUART_1_3V_OPERATION);
    
    /* Wait for Device to enumerate */
    while(!USBUART_1_GetConfiguration());
  
    /* Enumeration is done, enable OUT endpoint for receive data from Host */
    USBUART_1_CDC_Init();
    ADC_StartConvert();
    
   /* while(USBUART_1_CDCIsReady() == 0);
    USBUART_1_PutString("begin\n");
    UART_PutString("AT+CMGF=1\r");
    CyDelay(200);
    UART_GetString(buffer,'\r');
    while(USBUART_1_CDCIsReady() == 0);
    USBUART_1_PutString(buffer);
    UART_PutString("AT+CSCS=\"GSM\"\r");
    CyDelay(500);
    UART_PutString("AT+CNMI=2,1\r");
    CyDelay(500);
*/
    for(;;)
    {       
      //   while(USBUART_1_CDCIsReady() == 0);
      //   USBUART_1_PutString("begin\n");
      //   UART_PutString("AT+CMGF=1\r");
      //   CyDelay(200);
      //   a = UART_GetChar();
      //   //UART_GetString(buffer,'\r');
      //   while(USBUART_1_CDCIsReady() == 0);
      //   USBUART_1_PutChar(a);
      //  
      // a = UART_GetChar();
      //   //UART_GetString(buffer,'\r');
      //   while(USBUART_1_CDCIsReady() == 0);
      //   USBUART_1_PutChar(a);
       //UART_PutString("AT+CNMI=2\r");
      // UART_PutString("AT+CMGR=1\r");
      // CyDelay(500);
      // UART_GetString(buffer,'\r');
      // while (USBUART_1_CDCIsReady()==0);
      // USBUART_1_PutString(buffer);
    char8  a = UART_Blue_GetChar();
    
      if(a)
    {
      switch(a)
       {
          case 'p':
          case 'P':
          {
            if(power_on == 1)
            {
                LCD_ClearDisplay();
                power_on = 0;   
                UART_Blue_PutString(" OFF");
                PWM_1_WriteCompare1(0);    
            }
            else
            {
                    power_on = 1;
                    LCD_Position(0u,0u);
                    LCD_PrintString("Current:");
                    LCD_Position(1u,0u);
                    LCD_PrintString("Set:");
                    LCD_Position(0u,10u);
                    LCD_PrintString(temp);
                    LCD_Position(1u,10u);
                    LCD_PrintString(temp_set); 
   
                    UART_Blue_PutString(" ON");
            }
            break;
          }
        case 's':
        case 'S':
        {
            int i = 0;
            for(; i < 4 ; i++)
            {
               char b = UART_Blue_GetChar();
               if(b)
            {
                UART_Blue_PutChar(b);   
            }
            }

          //    uint8 ttmp_str[4];
          //    int i = 0 ;
          //    for( ; i < 4 ; i++)
          //  {
          //     ttmp_str[i] = command_str[i+4]; 
          //  }
          //  set_temperatute = convert2float(ttmp_str); 
          //  UART_Blue_PutString("DONE");
            break;   
        }
        case 'c':
        case 'C':
        {
            UART_Blue_PutString(temp);
            break;
        }
        case 'i':
        case 'I':
        {
            set_temperatute += 0.1;
            convert2string(temp_set,set_temperatute);
            UART_Blue_PutString(temp_set);
            
            break;
        }
        case 'd':
        case 'D':
        {
            set_temperatute -= 0.1;
            convert2string(temp_set,set_temperatute);
            UART_Blue_PutString(temp_set);
            break;
        }
       }
    }
      UART_ClearRxBuffer();
      UART_ClearTxBuffer();
       CapSense_UpdateEnabledBaselines();
       CapSense_ScanEnabledWidgets(); 
     if(power_on == 1)
    {
    if(CapSense_IsBusy()!=0)
    {
        status_Button_down = CapSense_CheckIsWidgetActive(CapSense_BUTTON_DOWN__BTN);
        status_Button_up = CapSense_CheckIsWidgetActive(CapSense_BUTTON_UP__BTN);
        sliderPosition = (uint8)CapSense_GetCentroidPos(CapSense_LINEARSLIDER0__LS);
        if(status_Button_up == ON)
            {
                if((pushState_Button_up == Release)&&(set_temperatute < 100))
                {
                    
                    set_temperatute += 0.1;
                    pushState_Button_up = PUSH;
                    pushState_Button_down = Release;
                }
            }
            
            else if(status_Button_down == ON)
            {
                if(pushState_Button_down == Release)
                {
                   
                    set_temperatute -= 0.1;
                    pushState_Button_down = PUSH;
                    pushState_Button_up = Release;
                }
            }
            else if(sliderPosition < 100 && sliderPosition >= 0)
            {
               if(LastsliderPosition != sliderPosition)
            {
                set_temperatute = 1.0 * sliderPosition / 100 * 35;
                pushState_Button_down = Release;
                pushState_Button_up = Release;
                LastsliderPosition = sliderPosition;
            }
            }
            else
            {
                pushState_Button_down = Release;
                pushState_Button_up = Release;

            }    
         
        ADC_IsEndConversion(ADC_WAIT_FOR_RESULT);
        voltageRawCount = ADC_GetResult16();
        temperature = voltageRawCount * 0.1019 + 3.5;
        /*i = (i + 1) % 20;
        if(!i)
        {
            int k = 0;
            for(; k < 50 ; k++)
            {
                i_temp[k] = temperature;
            }
        }*/
        int j = 0;
        float sum_temp = 0;
        for(; j < 49 ; j++)
        {
            sum_temp += i_temp[j+1];
            i_temp[j] = i_temp[j+1]; 
        }
        i_temp[49] = temperature;
        sum_temp += temperature;
        ave_temp = sum_temp / 50;
    }
    }
        //串口控制
        if(USBUART_1_DataIsReady() != 0u)
        {
            command_count = USBUART_1_GetAll(command_str);
            if(command_count != 0u)
            {
                switch(command_str[3])
                {
                    case (uint)'u':
                    {
                       while(USBUART_1_CDCIsReady() == 0u);
                       USBUART_1_PutString("\n**Menu:    **\n**Current**\n**Set_XX.X**\n**Power**\n");
                       break;
                    }
                    case (uint)'_':
                    {
                       while(USBUART_1_CDCIsReady() == 0u);
                       command_count = USBUART_1_GetAll(command_str);  
                       uint8 ttmp_str[4];
                       int i = 0 ;
                       for( ; i < 4 ; i++)
                     {
                        ttmp_str[i] = command_str[i+4]; 
                     }
                     set_temperatute = convert2float(ttmp_str);
                     break;
                    }
                    case (uint)'r':
                    {
                       while(USBUART_1_CDCIsReady() == 0u);
                       USBUART_1_PutString(temp);
                       break;
                    }
                    case (uint) 'e':
                    {
                        if(power_on == 0)
                        {
                            power_on = 1;
                            LCD_Position(0u,0u);
                            LCD_PrintString("Current:");
                            LCD_Position(1u,0u);
                            LCD_PrintString("Set:");
                            LCD_Position(0u,10u);
                            LCD_PrintString(temp);
                            LCD_Position(1u,10u);
                            LCD_PrintString(temp_set); 
                            
                        }
                        else
                        {
                            power_on = 0;
                            LCD_ClearDisplay();
                            PWM_1_WriteCompare1(0);
                        }
                    }
                }
            }
        }
        convert2string(temp,ave_temp);
        convert2string(temp_set,set_temperatute);
        if(power_on == 1)
        {
        // 温度超限报警
        if(ave_temp-set_temperatute > 5 || set_temperatute - ave_temp > 5)
        {
            cnt = (cnt+1)%5;
            if(!cnt)
            {
                if(on_off == 0) on_off = 1024;
               else if (on_off == 1024) on_off = 0;
            }
            PWM_1_WriteCompare1(on_off);
        }
        else
        {
           PWM_1_WriteCompare1(0);
        }
        }
        // 通信模块
        if(ave_temp - set_temperatute > 5)
        {
            i++;
            if(is_calling == 0)
            {
               //发短信
               UART_ClearRxBuffer();
               UART_PutString("AT+CSCS=\"GSM\"\r");
               CyDelay(500);
               UART_ClearRxBuffer();
               UART_PutString("AT+CMGF=1\r");
               CyDelay(500);
               UART_PutString("AT+CMGS=\"18801315596\"\r");
               UART_ClearRxBuffer();
               UART_GetString(buffer,'>');
               CyDelay(500);
               UART_ClearRxBuffer();
               UART_PutString(Message1);
               UART_PutString(temp);
               UART_PutString(Message2);
               UART_PutString(temp_set);
               UART_PutString(Message3);
               CyDelay(500);
               UART_ClearRxBuffer();
               UART_PutChar(0x1A);
               CyDelay(500);
 
               is_calling = 1;
            }
            if(i == 50)
            {
               //拨打电话
               UART_ClearRxBuffer();
               UART_ClearTxBuffer();
               UART_PutString("ATE1\r");
               CyDelay(500);
               UART_PutString("AT+COLP=1\r");
               CyDelay(500);
               UART_PutString("ATD18801315596;\r");
            }

        }
        if(power_on == 1)
        {
       // //温度控制PID
       // stPID.Proportion = 2; //PID比例值
       // stPID.Integral = 0;  //设置PID积分值
       // stPID.Derivative = 5; //设置PID微分值
       // stPID.SetPoint = set_temperatute ;
       // //float fOut = PIDCalc(&stPID,(int)(temperature));
        float fOut = ave_temp - set_temperatute;
       // if(fOut > 0)
       // {
       //     feedback = 1024;
       // }
       // else
       // {
       //     feedback = 0;
       // }
        if(fOut > 0.3)
        {
            feedback = 1024;
        }
        else if(fOut > 0.2)
        {
            feedback = (uint16)((fOut - 0.2)/(0.1)*120+900); 
        }
        else if (fOut > 0.05)
        {
            feedback = (uint16)(fOut/(0.5)*100+800);
        }
        else if(fOut <= 0)
        {
            feedback = 0;
        }
        if(feedback < 0)
        {
            feedback = 0;
        }
        else if(feedback > 1024)
        {
            feedback = 1024;
        }
        PWM_1_WriteCompare2(feedback);
        
        //显示LCD屏与串口传输
        if(( last_set_temp != set_temperatute) || (ave_temp - last_temp > 0.005) || (last_temp - ave_temp > 0.005) )
        {
           LCD_Position(0u,10u);
           LCD_PrintString(temp);
           LCD_Position(1u,10u);
           LCD_PrintString(temp_set);
           
        //   while (USBUART_1_CDCIsReady()==0);
        //   USBUART_1_PutString(temp);
        //   while (USBUART_1_CDCIsReady()==0);
        //   USBUART_1_PutString("\n");
        } 
        last_set_temp = set_temperatute;
        last_temp = ave_temp;
        }
        CyDelay(80);
    }
    
    
}

/* [] END OF FILE */
