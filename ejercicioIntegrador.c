/*
===============================================================================
 Name        : ejercicioIntegrador.c
 Author      : Alfici, Facundo Ezequiel(author)
 Version     : 1.0
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/
/*
 * En la dirección de memoria 0x20080000 se tiene almacenado un valor de 32 bits que representa cuatro formas de onda binarias de 8 bits cada una.
 * Utilizando los registros de configuración y el systick del microcontrolador LPC1769 generar por el pin P2.8 las formas de onda binarias en serie de 8 bits almacenadas en la dirección anteriormente mencionada, y generar por el puerto P2 el promedio de la forma de onda binaria seleccionada.
 * El pin asociado a EINT0 presenta una resistencia de pull-down externa(Lógica positiva).
 * Configurar la interrupción de dicho pin con prioridad 3, para que, cada vez que interrumpa, termine la forma de onda actual y cambie a la siguiente (una vez que llega a la última, debe volver a comenzar la primera).
 * El periodo de la señal debe ser establecido mediante la interrupción (prioridad 2) del pin asociado a EINT1, el cual presenta una resistencia de pull-up(Lógica negativa).
 * De manera que se pueda cambiar el periodo de la señal entre 80[ms] (por defecto) y 160[ms]. Considerando un cclk de 65[MHz].
 * */
#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif
#include <cr_section_macros.h>
//Variables
volatile uint32_t eleccionDeF = 0;
volatile uint32_t eleccionDeOnda = 0;
volatile uint32_t cantidadST = 0;
volatile uint8_t* formasDeOnda = (uint8_t*) 0x20080000; //Puntero que apunta en la dirección de memoria donde están las formas de onda
//De paso, las divide en 8 bits. También se puede hacer de la siguiente forma:
//#define formasDeOnda (*(volatile uint8_t*) 0x20080000)

void configSysTick(void);
void configGPIO(void);
void configEXTInt(void);
void EINT0_IRQHandler(void);
void EINT1_IRQHandler(void);
void SysTick_Handler(void);

int main(void)
{
	configSysTick();
	configGPIO();
	configEXTInt();
	while(1)
	{}
    return 0 ;
}

void configSysTick(void)
{
	SysTick -> LOAD = 5199999;//Fórmula : STimer = (LOAD+1)*CClk ----> LOAD = (STimer/CClk)-1 -------> LOAD = 80mS*65MHz -1 = 5199999 (80mS)
	SysTick -> VAL = 0;
	SysTick -> CTRL = (1<<0) 	//Habilito la interrupcion
						| (1<<1) //Habilito la request a excepciones
						| (1<<2) ;	//Uso el clock de la CPU
	NVIC_EnableIRQ(SysTick_IRQn);
	NVIC_SetPriority(SysTick_IRQn, 0);
}

void configGPIO(void)
{
	LPC_PINCON -> PINSEL4 &= ~((3<<16)|(3<<18)); //Pines P2.8 y P2.9 como GPIO.
	LPC_GPIO2->FIODIR |= (1<<8)|(1<<9); //Pines P2.8 y P2.9 como salidas.
}

void configEXTInt(void)
{
	LPC_PINCON->PINSEL4 |= (1<<20)|(1<<22); //Pines P2.10 y P2.11 como EINT0 y EINT1 respectivamente. (01)
	LPC_SC->EXTMODE |= (1<<0) | (1<<1); //EINT0 y EINT1 por flanco.
	LPC_SC->EXTPOLAR |=(1<<0); // Lógica positiva en el EINT0 (Flanco de subida colocando un 1)
	LPC_SC->EXTPOLAR &=~(1<<1); // Lógica negativa en el EINT1 (Flaco de bajada colocando un 0)
	NVIC_EnableIRQ(EINT0_IRQn);
	NVIC_SetPriority(EINT0_IRQn, 3);
	NVIC_EnableIRQ(EINT1_IRQn);
	NVIC_SetPriority(EINT1_IRQn, 2);
}

void EINT0_IRQHandler(void)
{
	eleccionDeOnda++;
	if(eleccionDeOnda>3)
		eleccionDeOnda=0;
	LPC_SC->EXTINT |= (1<<0);
}

void EINT1_IRQHandler(void)
{
	eleccionDeF++;
	if(eleccionDeF>1)
		eleccionDeF=0;
	LPC_SC->EXTINT |= (1<<1);
}

void SysTick_Handler(void)
{
	volatile uint8_t indiceBit = 0;
	if(eleccionDeF==0)
	{
			for(int i=0 ; i<8 ; i++)
			{
				uint8_t bit = (formasDeOnda[eleccionDeOnda]>>(7-indiceBit) & 0x01);
				if(bit)
					LPC_GPIO2->FIOSET = (1<<8);
				else
					LPC_GPIO2->FIOCLR = (1<<8);
				indiceBit++;
			}
			indiceBit=0;
	}
	else
	{
			if(cantidadST==0)
			{
				uint8_t bit = (formasDeOnda[eleccionDeOnda]>>(7-indiceBit) & 0x01);
				if(bit)
					LPC_GPIO2->FIOSET = (1<<8);
				else
					LPC_GPIO2->FIOCLR = (1<<8);
				indiceBit++;
				if(indiceBit>=8)
					indiceBit=0;
			}
			cantidadST= !cantidadST;
	}
}
