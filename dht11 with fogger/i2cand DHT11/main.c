
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "i2cmaster.h"
#include "i2c_lcd.h"


#define DHT11_PIN 0 // A0 for dht 11
uint8_t c=0,I_RH,D_RH,I_Temp,D_Temp,CheckSum;
uint8_t fogCount=0, fogPin=1;
char data[5];
uint8_t foggerTime=5; // for 10 seconds

void Request()				/* Microcontroller send start pulse/request */
{
	DDRA |= (1<<DHT11_PIN);
	PORTA &= ~(1<<DHT11_PIN);	/* set to low pin */
	_delay_ms(20);			/* wait for 20ms */
	PORTA |= (1<<DHT11_PIN);	/* set to high pin */
}

void Response()				/* receive response from DHT11 */
{
	DDRA &= ~(1<<DHT11_PIN);
	while(PINA & (1<<DHT11_PIN));
	while((PINA & (1<<DHT11_PIN))==0);
	while(PINA & (1<<DHT11_PIN));
}

uint8_t Receive_data()			/* receive data */
{
	for (int q=0; q<8; q++)
	{
		while((PINA & (1<<DHT11_PIN)) == 0);  /* check received bit 0 or 1 */
		_delay_us(30);
		if(PINA & (1<<DHT11_PIN))/* if high pulse is greater than 30ms */
		c = (c<<1)|(0x01);	/* then its logic HIGH */
		else			/* otherwise its logic LOW */
		c = (c<<1);
		while(PINA & (1<<DHT11_PIN));
	}
	return c;
}

void runFogger(int runTime)
{
	fogCount++;
	if (fogCount>5 && fogCount<(fogCount+runTime))
	{
		//on fogger
		PORTA |= (1<<fogPin);	/* set to high pin */
		lcd_clear();
		lcd_goto_xy(0,0);
		lcd_puts("Run Fogger ");
		itoa(fogPin,data,10);
		lcd_puts(data);
		
		lcd_goto_xy(0,1);
		lcd_puts("Duration ");
		itoa(runTime*2,data,10);
		lcd_puts(data);
		//_delay_ms(1000);
		//lcd_clear();
	}else
	{
		PORTA &= ~(1<<fogPin);	/* set to low pin */
		//lcd_clear();
	}
	
	// reset fogger
	if (fogCount==15 )
	{
		fogCount=0;
	}
	
}

int main(void)
{
	DDRA |= (1<<1); // for fogger 1
	DDRA |= (1<<2); // for foger 2
	
	DDRB &= ~(1 << PINB0); // for  input1
	DDRB &= ~(1 << PINB1); // for  input2
	_delay_ms(100);
	
	
	lcd_init(LCD_BACKLIGHT_ON);			/* Initialize LCD */
	lcd_clear();			/* Clear LCD */
	_delay_ms(2000);
	
	while(1)
	{
		Request();		/* send start pulse */
		Response();		/* receive response */
		
		I_RH=Receive_data();	/* store first eight bit in I_RH */
		D_RH=Receive_data();	/* store next eight bit in D_RH */
		I_Temp=Receive_data();	/* store next eight bit in I_Temp */
		D_Temp=Receive_data();	/* store next eight bit in D_Temp */
		CheckSum=Receive_data();/* store next eight bit in CheckSum */
		
		lcd_goto_xy(0,0);		/* Enter column and row position */
		
		if ( PINB & (1 << 0))  //PINA & (1<<DHT11_PIN)
		{
			// pin is high
			fogPin=1;
			lcd_goto_xy(0,0);
			lcd_puts("Set Mode 1");
			_delay_ms(1000);
			lcd_clear();

		}
		
		if ( PINB & (1 << 1) )
		{
			// pin is high
			fogPin=2;
			lcd_goto_xy(0,0);
			lcd_puts("Set Mode 2");
			_delay_ms(1000);
			lcd_clear();
		}
		
		
		lcd_goto_xy(0,0);
		lcd_puts("Humidity =");
		lcd_goto_xy(0,1);
		lcd_puts("Temp = ");
		
		if ((I_RH + D_RH + I_Temp + D_Temp) != CheckSum)
		{
			lcd_goto_xy(0,0);
			lcd_puts("Error");
		}
		else
		{
			itoa(I_RH,data,10);
			lcd_goto_xy(11,0);
			lcd_puts(data);
			lcd_puts(".");
			
			itoa(D_RH,data,10);
			lcd_puts(data);
			lcd_puts("%");

			itoa(I_Temp,data,10);
			lcd_goto_xy(6,1);
			lcd_puts(data);
			lcd_puts(".");
			
			itoa(D_Temp,data,10);
			lcd_puts(data);
			//lcd_sendData(0xDF);
			lcd_puts("C ");
			
			
		/*	itoa(CheckSum,data,10);
			lcd_puts(data);
			lcd_puts(" ");
			*/
			
			// adjust fogger time-------------------------------------
			if (I_Temp>34) //I_Temp>30
			{
				foggerTime = foggerTime-2;
			}else if (I_Temp>32) //I_Temp>30
			{
				foggerTime--;
			}else if(I_Temp>30)
			{
				// No default time change
			}else if(I_Temp>28)
			{
				foggerTime++;
			}
			
			
			if (I_RH>95) //I_Temp>30
			{
				foggerTime++;
			}else if(I_RH>90)
			{
				// no fogger time change
			}else if (I_RH>85)
			{
				foggerTime--;
			}
			
			//run fogger
			//foggerTime=2;
			runFogger(foggerTime);
			foggerTime =5; // set default
		
	
		
		}
	
		_delay_ms(2000);
		lcd_clear();
		
		
	} 
	
}