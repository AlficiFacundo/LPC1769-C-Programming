/*
===============================================================================
 Name        : ejercicioPractica.c
 Author      : Alfici, Facundo Ezequiel(author)
 Version     : 1.0
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/
/*	Un estacionamiento automatizado utiliza una barrera que se abre y cierra en función
 *  de la validación de un ticket de acceso utilizando una LPC1769 Rev. D trabajando a
 *   una frecuencia de CCLK a 70 [MHz]
 *   Cuando el sistema detecta que un automóvil se ha posicionado frente a la barrera,
 *   se debe activar un sensor conectado al pin P2[4] mediante una interrupción externa (EINT).
 *   Una vez validado el ticket, el sistema activa un motor que abre la barrera usando el pin P0[15].
 *   El motor debe estar activado por X segundos y luego apagarse, utilizando el temporizador
 *   Systick para contar el tiempo. Si el ticket es inválido, se encenderá un LED rojo
 *   conectado al pin P1[5].
 *   Para gestionar el tiempo de apertura de la barrera, existe un switch conectado al pin P3[4]
 *   que dispone de una ventana de configuración de 3 segundos gestionada por
 *   el temporizador Systick.
 *   Durante dicha ventana, se debe contar cuantas veces se presiona el switch y
 *   en función de dicha cantidad, establecer el tiempo de la barrera.
 */
#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

volatile uint32_t motorActivado=0;
const uint32_t valorTicks=13999999; //200mS por SysTick. Para que cuente 3s deben ser 15 veces
volatile uint32_t ticketValido=0;
volatile uint32_t cantidadInt3s=0;
volatile uint32_t estadoAnterior = 0;
volatile uint32_t contarSwitch =0;
const uint32_t valoresSysTick[5]={
		5599999, 		//80mS por int
		6999999,		//100mS por int
		8399999,		//120mS por int
		10499999,		//150mS por int
		13999999		//200mS por int
};

void configGPIO(void);
void configIntGPIO(void);
void configSysTick(uint32_t ticks);
void SysTick_Handler(void);
void EINT3_IRQHandler(void);

int main(void) {

	configGPIO();
	configIntGPIO();
	while(1)
	{
		uint8_t estadoActual = ((LPC_GPIO3->FIOPIN &(1<<25))>>25);
		if(ticketValido)
		{
			if (estadoAnterior == 0 && estadoActual == 1) {
				contarSwitch++;   //Detecta flanco ascendente
			}
			estadoAnterior = estadoActual;
			if(contarSwitch>5)
			{
				contarSwitch=5; //Defino el valor máximo posible
			}
		}
	}
    return 0 ;
}

void configGPIO(void)
{
	LPC_PINCON->PINSEL4 &=~(3<<8); //P2.4
	LPC_PINCON->PINSEL0 &=~(3<<30); // P0.15
	LPC_PINCON->PINSEL2 &=~(3<<30); //P1.15 led
	LPC_PINCON->PINSEL7 &=~(3<<18); //P3.25
	LPC_GPIO2->FIODIR &=~(1<<8);
	LPC_GPIO0->FIODIR |=(1<<15);
	LPC_GPIO1->FIODIR |=(1<<15);
	LPC_GPIO3->FIODIR &=~(1<<25);
	//Considero resistencias externas colocadas, desabilito las internas
	LPC_PINCON->PINMODE0 &=~(1<<30);
	LPC_PINCON->PINMODE2 &=~(1<<30);
	LPC_PINCON->PINMODE7 &=~(1<<18);
}

void configIntGPIO(void)
{
	LPC_GPIOINT->IO2IntEnR |=(1<<4); //P2.4 como interrupción por Rising Edge
	LPC_GPIOINT->IO2IntClr |=(1<<4); //Limpio banderas previas
	NVIC_EnableIRQ(EINT3_IRQn);
	NVIC_SetPriority(EINT3_IRQn, 1);
}

void configSysTick(uint32_t ticks)
{
	SysTick-> LOAD = ticks;
	SysTick-> VAL = 0;
	SysTick-> CTRL = (1<<0)|(1<<1)|(1<<2);
	NVIC_SetPriority(SysTick_IRQn, 0);
}

void EINT3_IRQHandler(void)
{
	LPC_GPIO1->FIOCLR |= (1<<15);//Apago el led de invalidez
	ticketValido=1;
	configSysTick(valorTicks);
	if(ticketValido==0)
		LPC_GPIO1->FIOSET |= (1<<15);
	if(cantidadInt3s>19)
	{
		cantidadInt3s=0;
		ticketValido=0;
		configSysTick(valoresSysTick[contarSwitch]);
		motorActivado=1;
		LPC_GPIO0->FIOSET|=(1<<15);
	}
	LPC_GPIOINT->IO0IntClr |=(1<<0);
}

void SysTick_Handler(void)
{
	cantidadInt3s++;
	if(cantidadInt3s>30 && motorActivado)
	{
		motorActivado=!motorActivado;
		LPC_GPIO0->FIOCLR|=(1<<15);
	}
}
