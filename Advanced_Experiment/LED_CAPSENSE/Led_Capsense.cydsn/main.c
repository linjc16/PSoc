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

#if defined (__GNUC__)
    /* Add an explicit reference to the floating point printf library */
    /* to allow the usage of floating point conversion specifiers. */
    /* This is not linked in by default with the newlib-nano library. */
    asm (".global _printf_float");
#endif

/* The size of the buffer is equal to maximum packet size of the 
*  IN and OUT bulk endpoints. 
*/
#define BUFFER_LEN  64u

char8 *parity[] = { "None", "Odd", "Even", "Mark", "Space" };
char8 *stop[] = { "1", "1.5", "2" };

#define ON (1)
#define OFF (0)
#define NO_FINGER (0xFF)
#define PUSH (1)
#define Release (0)
int main()
{
    char8 display_str[6] = {"  0%"};
    uint8 status_Button_up = OFF;
    uint8 status_Button_down = OFF;
    uint16 sliderPosition = NO_FINGER;
    uint8 brightness = 0;
    uint8 pushState_Button_up = Release;
    uint8 pushState_Button_down = Release;
    uint16 LastsliderPosition = NO_FINGER;
    uint8 last_brightness = 0;
    CyGlobalIntEnable; /* Enable global interrupts. */
    PWM_Start();
    LCD_Start();
    LCD_Position(0u,0u);
    LCD_PrintString("LED Brightness: ");
    CapSense_Start();
    CapSense_InitializeAllBaselines();
        /* Start USBFS Operation with 3V operation */
    USBUART_Start(0u, USBUART_3V_OPERATION);
    
    /* Wait for Device to enumerate */
    while(!USBUART_GetConfiguration());
  
    /* Enumeration is done, enable OUT endpoint for receive data from Host */
    USBUART_CDC_Init();
    
    for(;;)
    {
       CapSense_UpdateEnabledBaselines();
       CapSense_ScanEnabledWidgets(); 
       
        if(CapSense_IsBusy()!= 0)
        {
            /* Update baseline for all the sensors */ 

            status_Button_down = CapSense_CheckIsWidgetActive(CapSense_BUTTON_DOWN__BTN);
            status_Button_up = CapSense_CheckIsWidgetActive(CapSense_BUTTON_UP__BTN);
            sliderPosition = (uint8)CapSense_GetCentroidPos(CapSense_LINEARSLIDER0__LS);
            if(status_Button_up == ON)
            {
                if((pushState_Button_up == Release)&&(brightness < 100))
                {
                    
                    brightness++;
                    pushState_Button_up = PUSH;
                    pushState_Button_down = Release;
                }
            }
            
            else if(status_Button_down == ON)
            {
                if((pushState_Button_down == Release)&&( brightness > 0))
                {
                   
                    brightness -- ;
                    pushState_Button_down = PUSH;
                    pushState_Button_up = Release;
                }
            }
            else if(sliderPosition <= 100)
            {
               if(LastsliderPosition != sliderPosition)
            {
                brightness = sliderPosition;
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
        if(brightness != last_brightness)
        {
           PWM_WriteCompare(brightness);
          if(brightness >= 100)
        {
          display_str[0] = '1';
          display_str[1] = '0';
          display_str[2] = '0';
          display_str[3] = '%';
          display_str[4] = 0;
        }
        else 
        {
          display_str[0]=((brightness > 9)?(brightness/10+48):(' '));
          display_str[1]=brightness%10+48;
          display_str[2]='%';
          display_str[3]=' ';
          display_str[4]=0;
          
        }
        
          LCD_Position(1u,0u);
          LCD_PrintString(display_str);
          display_str[4]= '\n';
          while (USBUART_CDCIsReady()==0);
          USBUART_PutString(display_str);
       }
        last_brightness = brightness;
      }
      CyDelay(80);  
    }
}

/* [] END OF FILE */
