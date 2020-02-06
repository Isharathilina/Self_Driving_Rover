/*
* keypad.cpp
*
* Created: 19/01/2018 16:08:16
*/



#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>


static volatile int pulse = 0, pulse1 =0;
static volatile int i = 0, i2 = 0;
static volatile uint8_t tot_overflow;
//int sonar_type=0;

void timer1_init()
{
	TCCR1B |= (1 << CS11);
	TCNT1 = 0;
	TIMSK |= (1 << TOIE1);
	sei();
}

ISR(TIMER1_OVF_vect)
{
	TCNT1=0;
}

ISR(INT0_vect)
{

		if (i==1)
		{
			pulse=TCNT1;
			TCNT1=0;
			i=0;
		}
		if (i==0)
		{
			TCNT1 = 0;
			i=1;
		}
	
}

ISR(INT1_vect)
{

		if (i2==1)
		{
			pulse1=TCNT1;
			TCNT1=0;
			i2=0;
		}
		if (i2==0)
		{
			TCNT1 = 0;
			i2=1;
		}
}

int getSensorData1(){
	//sonar_type=1;
	PORTD|=(1<<PIND0);
	_delay_us(15);
	PORTD &=~(1<<PIND0);
	_delay_ms(200);
	//sonar_type=0;
	return pulse/(58);
}

int getSensorData2(){
	//sonar_type=2;
	PORTD|=(1<<PIND1);
	_delay_us(15);
	PORTD &=~(1<<PIND1);
	
	_delay_ms(200);
	//sonar_type=0;
	return pulse1/(58);
}

void forward(){
	PORTA |= (1<<PINA4);
	PORTA &= ~(1<<PINA5);
	PORTA |= (1<<PINA6);
	PORTA &= ~(1<<PINA7);
}

void reverse(){
	PORTA |= (1<<PA5);
	PORTA &= ~(1<<PA4);
	PORTA |= (1<<PA7);
	PORTA &= ~(1<<PA6);
}

void turn_left(){
	PORTA |= (1<<PA4);
	PORTA &= ~(1<<PA5);
	PORTA &= ~(1<<PA6);
	PORTA |= (1<<PA7);
}

void turn_right(){
	PORTA &= ~(1<<PA4);
	PORTA |= (1<<PA5);
	PORTA |= (1<<PA6);
	PORTA &= ~(1<<PA7);
}

void stop(){
	PORTA &= ~(1<<PA4);
	PORTA &= ~(1<<PA5);
	PORTA &= ~(1<<PA6);
	PORTA &= ~(1<<PA7);
}



int main(void)

{

	//DDRA = 0xFF;
	DDRA |= (1<<4);
	DDRA |= (1<<5);
	DDRA |= (1<<6);
	DDRA |= (1<<7);
	
	DDRB = 0xFF;
	DDRD = 0b11110011; //1 trig , d3 echo
	_delay_ms(50);



	GICR|=(1<<INT0);
	GICR|=(1<<INT1);

	MCUCR|=(1<<ISC00);
	MCUCR|=(1<<ISC10);

	TCCR1A = 0;
	timer1_init();

	int16_t COUNTA = 0;
	int16_t COUNTB = 0;
	char SHOWA [16];
	char SHOWB [16];

	sei();
	
	int data1;
	int data2;
	int Workingtimer=0;
	
	while(Workingtimer<7200) // one cirecle get 1/2 second, we system stop 30 min

	{
		int delaytime=30;
		data1 = getSensorData1();
		data2 = getSensorData2();
		
		if(data1<2 && data2<2)
		{
			for (int a=0; a<6; a++)
			{
				PORTB |= 1<<PINB0;
				turn_right();
				_delay_ms(delaytime);
				stop();
				_delay_ms(delaytime);
			}
			
			forward();
			_delay_ms(delaytime);
			stop();
			forward();
			_delay_ms(delaytime);
			stop();
			
			
			
			
		}else if(data1<2)
		{
			for (int a=0; a<3; a++)
			{
				PORTB &= ~(1<<PINB0);
				turn_right();
				_delay_ms(delaytime);
				stop();
			}
			forward();
			_delay_ms(delaytime);
			stop();
			forward();
			_delay_ms(delaytime);
			stop();
			
			
		}
		else if(data2<1)
		{
			PORTB |= 1<<PINB0;
			turn_right();
			_delay_ms(delaytime);
			stop();
		}
		else if(data2<2)
		{
			forward();
			_delay_ms(delaytime);
			stop();
			
		}else
		{
			turn_left();
			_delay_ms(20);
			stop();
			
			forward();
			_delay_ms(delaytime);
			stop();
			
			
		}
		
		Workingtimer++;
		
		
		/*
		forward();
		reverse();
		_delay_ms(15);
		stop();
		*/
		
	}
}

