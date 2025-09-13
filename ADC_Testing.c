/*
===============================================================================
 Name        : ADC_Testing.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/
/* Este codigo es la configuracion de un ADC que mide la tension en un potenciometro por la entrada analogica
 * y luego lo convierte a digital. Cada 1 segundo se hace la conversion de cada medicion usando el SysTick
 * La Vref+- va a ser de entre 3V3 y masa. Si tenemos de 0-1V, prende el led azul. Si tenemos de 1-2V, se enciende el
 * led verde y si tenemos de 2-3V se enciende el led rojo. Finalmente, si se tiene un valor mayor a 3V
 * parpadearan los 3 leds.
 *
 * Formula para calcular la tension:
 *
 * Vmedido = (((Ref+) - (Ref-))/(4096))*dato_convertido
 *
 * 			 Ref+  -  Ref-
 * Vmedido = --------------  *  dato_convertido
 * 				  4096
 *
 * */
#ifdef __USE_CMSIS
#include "LPC17xx.h"
#include <stdint.h>  // Para usar uint32_t
#endif
#include <cr_section_macros.h>
volatile uint32_t cantidadIntST = 0;
volatile uint32_t datoMedido = 0;
volatile uint32_t valorTension = 0;

void configADC(void);
void configSysTick(void);
void delay(uint32_t tiempo);

int main(void)
{
	configADC();
	configSysTick();
    while(1)
    {}
    return 0 ;
}

void configGPIO(void)
{
	LPC_PINCON->PINSEL1 &=~(1<<12); //(22-16)*2 == bits 12:13
	LPC_GPIO0->FIODIR |= (1<<22);//Led Rojo
	LPC_PINCON->PINSEL3 &=~(1<<4); //(18-16)*2 == bits 4:5
	LPC_GPIO1->FIODIR |= (1<<18);//Led Verde
	LPC_PINCON->PINSEL3 &=~(1<<8); //(20-16)*2 == bits 8:9
	LPC_GPIO1->FIODIR |= (1<<20);//Led Azul
	return;
}

void configADC(void)
{
	LPC_SC ->PCONP |= (1<<12); 				//Enciendo ADC.
	LPC_ADC ->ADCR |= (1<<21); 				//Habilito el ADC.
	LPC_SC ->PCLKSEL0 |=(3<<8); 			//CoreCLK/8 para perifericos.
	LPC_ADC ->ADCR &= ~(255<<21); 			//CLK para el ADC (Divisor Interno).
	LPC_ADC ->ADCR |= (1<<0);				//Habilito el canal 0.
	LPC_ADC ->ADCR |= (1<<16);				//Activo Modo BURST.
	LPC_PINCON -> PINMODE1 |= (1<<15);		//Pines sin pull-up ni pull-down.
	LPC_PINCON -> PINSEL1 |= (1<<14);		//Seleccion de la funcion del pin 14 en ADC.
	LPC_ADC -> ADINTEN |= (1<<0);			//Activo las interrupciones por AD0.
	LPC_ADC -> ADINTEN &= ~(1<<8); 			//Desabilito la interrupcion por flag DONE (Uso por canal).

	NVIC_EnableIRQ(ADC_IRQn);				//Habilito Handler del ADC
	return;
}

void configSysTick(void)
{
	SysTick->LOAD = 1000000;		//Cargo el valor que va a tomar el systick (10mS)
	SysTick->VAL = 0;			//Limpio la bandera
	SysTick->CTRL = (1<<0) 	//Habilito la interrupcion
					| (1<<1) //Habilito la request a excepciones
					| (1<<2) ;	//Uso el clock de la CPU
	NVIC_EnableIRQ(SysTick_IRQn);	//Habilito el handler del systick
	NVIC_SetPriority(SysTick_IRQn, 0);  // Pongo la prioridad mas alta (0 es la mayor) al systick
	// En caso de tener otra interrupcion a la que quiero ponerle mayor prioridad, puedo hacer lo siguiente
	// NVIC_SetPriority(SysTick_IRQn, n); Donde n es la prioridad que quiero usar para el systick
}

void SysTick_Handler(void)
{
	cantidadIntST++;
	if(cantidadIntST==100)
	{
		cantidadIntST=0;
		if(valorTension<1)
		{
			LPC_GPIO1->FIOSET = (1<<20);
			LPC_GPIO1->FIOCLR = (1<<18);
			LPC_GPIO0->FIOCLR = (1<<22);
		}
		else if(valorTension>=1 && valorTension<2)
		{
			LPC_GPIO1->FIOSET = (1<<18);
			LPC_GPIO1->FIOCLR = (1<<20);
			LPC_GPIO0->FIOCLR = (1<<22);
		}
		else if(valorTension>=2 && valorTension<3)
		{
			LPC_GPIO0->FIOSET = (1<<22);
			LPC_GPIO1->FIOCLR = (1<<20)|(1<<18);
		}
		else
		{
			while(1)
			{
				LPC_GPIO1->FIOSET = (1<<18)|(1<<20);
				LPC_GPIO0->FIOSET = (1<<22);
				delay(10000000);
				LPC_GPIO1->FIOCLR = (1<<18)|(1<<20);
				LPC_GPIO0->FIOCLR = (1<<22);
			}
		}
	cantidadIntST=0;
	}
	else
	{}
	return;
}

void ADC_IRQHandler(void)
{
	datoMedido = (LPC_ADC -> ADDR0>>4) &0xFFF ; //Guardado del valor en una variable auxiliar
	valorTension = (datoMedido*(3300-0)/4096); //Formula para sacar el valor de tension para saber el led
	//Al colocar 3300 en vez de 3.3 obtengo en mV
	return;
}

void delay(uint32_t tiempo) {
    for (uint32_t i = 0; i < tiempo; i++) {
        __NOP();  // Instrucción que no hace nada, pero consume ciclos
    }
}

