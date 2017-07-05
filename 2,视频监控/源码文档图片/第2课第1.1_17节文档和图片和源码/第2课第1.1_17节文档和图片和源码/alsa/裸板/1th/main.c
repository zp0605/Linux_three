#include <stdio.h>
#include "serial.h"
#include "i2c.h"

unsigned char at24cxx_read(unsigned char address);
void at24cxx_write(unsigned char address, unsigned char data);


int main()
{
    char c;
    char str[200];
    int i;
	int address;
	int data;
    int volume = 50; /* 0~100 */
    
    uart0_init();   // 波特率115200，8N1(8个数据位，无校验位，1个停止位)


    read_wav();
        
    while (1)
    {
menu:        
        printf("\r\n##### Sound Menu #####\r\n");
        printf("[J] JZ2440\n\r");
        printf("[M] MINI2440\n\r");
        printf("[T] TQ2440\n\r");
        printf("Enter your selection: ");

        c = getc();
        printf("%c\n\r", c);
        switch (c)
        {
            case 'J':
            case 'j':
            {
                /* machine init */
                jz2440_init();
                
                /* codec init */
                wm8976_init();
                break;
            }
            
            case 'm':
            case 'M':
            {
                /* machine init */
                mini2440_init();

                /* codec init */
                uda1341_init();
                break;
            }

            case 't':
            case 'T':
            {
                /* machine init */
                tq2440_init();

                /* codec init */
                uda1341_init();
                break;
            }

            default:
            {
                goto menu;
            }
        }        

        /* soc init */
        iis_init();
        dma_init();

        dma_start();
        iis_start();

        printf("\r\n##### Operate Menu #####\r\n");
        printf("[-] volume down\n\r");
        printf("[+] volume up\n\r");
        printf("[Q] Quit\n\r");
        printf("Cur volume: %03d", volume);
        while (1)
        {
            c = getc();
            if (c == 'q' || c == 'Q')
            {
                printf("%c\n\r", c);
                break;
            }
            else if (c == '-')
            {
                if (volume > 0)
                    volume--;
                set_volume(volume);
            }
            else if (c == '+')
            {
                if (volume < 100)
                    volume++;
                set_volume(volume);
            }
            printf("\b\b\b%03d", volume);
        }
        
            
    }
    
    return 0;
}
