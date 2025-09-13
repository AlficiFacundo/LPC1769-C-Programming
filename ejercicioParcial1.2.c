/*
===============================================================================
 Name        : ejercicioParcial1.2.c
 Author      : Alfici, Facundo Ezequiel(author)
 Version     : 1.0
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/
/*
 * Consigna
 * Utilizando interrupciones por GPIO realizar un código en C que permita, mediante 4 pines de entrada GPIO
 * leer y guardar un número compuesto por 4 bits.
 * Dicho número puede ser cambiado por un usuario mediante 4 switches, los cuales cuentan con sus respectivas resistencias de pull up externas.
 * El almacenamiento debe realizarse en una variable del tipo array de forma tal que se asegure tener disponible siempre los últimos 10 números
 * elegidos por el usuario, garantizando además que el número ingresado más antiguo, de este conjunto de 10
 * se encuentre en el elemento 9 y el número actual en el elemento 0 de dicho array.
 * La interrupción por GPIO empezará teniendo la máxima prioridad de interrupción posible y cada 200 números ingresados deberá
 * disminuir en 1 su prioridad hasta alcanzar la mínima posible. Llegado este momento, el programa deshabilitará todo tipo de interrupciones
 * producidas por las entradas GPIO.
 * Tener en cuenta que el código debe estar debidamente comentado.
 *
 */
#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

//Resistencias pull-up externas, es decir, logica negativa
volatile uint32_t arrayNumeros[10];
volatile uint32_t auxiliar=0;
volatile uint32_t cantidadNumeros = 0;
volatile uint32_t flagAlmacenar=0;

void configIntGPIO(void);
void EINT3_IRQHandler(void);

int main(void) {

	configIntGPIO();
	while(1)
	{
		if(flagAlmacenar)
		{
			for(int i = 9 ; i>0 ; i--)
			{
				arrayNumeros[i] = arrayNumeros[i-1];
			}
			arrayNumeros[0]=auxiliar;
			flagAlmacenar=0;
		}
    }
    return 0 ;
}

void configIntGPIO(void)
{
	LPC_PINCON->PINSEL0 &=~((3<<0)|(3<<2)|(3<<4)|(3<<6)); //Pines 0, 1, 2 y 3 como GPIO
	LPC_GPIO0->FIODIR &=~((1<<0)|(1<<1)|(1<<2)|(1<<3)); //Pines 0, 1, 2 y 3 como entradas.
	LPC_GPIOINT->IO0IntEnR |= ((1<<0)|(1<<1)|(1<<2)|(1<<3)); //Habilito interrupciones por rising edge
	LPC_GPIOINT->IO0IntEnF |= ((1<<0)|(1<<1)|(1<<2)|(1<<3)); //Habilito interrupciones por falling edge
	LPC_GPIOINT->IO0IntClr |= ((1<<0)|(1<<1)|(1<<2)|(1<<3)); //Limpio las banderas previas de todas las interrupciones de GPIO
	NVIC_EnableIRQ(EINT3_IRQn);
	NVIC_SetPriority(EINT3_IRQn,0);
}

void EINT3_IRQHandler(void)
{
	if(LPC_GPIOINT->IO0IntStatF & (1<<0)) // Testeo si fue por P0.0, es decir, el MSB. Como es lógica negativa, si es falling es 1.
	{
		auxiliar |= (1<<3);
		LPC_GPIOINT->IO0IntClr |=(1<<0);
	}
	else if (LPC_GPIOINT->IO0IntStatR & (1<<0))
	{
		auxiliar &= ~(1<<3);
		LPC_GPIOINT->IO0IntClr |=(1<<0);
	}
	else if(LPC_GPIOINT->IO0IntStatF & (1<<1)) // Testeo si fue por P0.1. Como es lógica negativa, si es falling es 1.
	{
		auxiliar |= (1<<2);
		LPC_GPIOINT->IO0IntClr |=(1<<1);
	}
	else if (LPC_GPIOINT->IO0IntStatR & (1<<1))
	{
		auxiliar &= ~(1<<2);
		LPC_GPIOINT->IO0IntClr |=(1<<1);
	}
	else if(LPC_GPIOINT->IO0IntStatF & (1<<2)) // Testeo si fue por P0.2. Como es lógica negativa, si es falling es 1.
	{
		auxiliar |= (1<<1);
		LPC_GPIOINT->IO0IntClr |=(1<<2);
	}
	else if (LPC_GPIOINT->IO0IntStatR & (1<<2))
	{
		auxiliar &= ~(1<<1);
		LPC_GPIOINT->IO0IntClr |=(1<<2);
	}
	else if(LPC_GPIOINT->IO0IntStatF & (1<<3)) // Testeo si fue por P0.3, es decir, el LSB. Como es lógica negativa, si es falling es 1.
	{
		auxiliar |= (1<<0);
		LPC_GPIOINT->IO0IntClr |=(1<<3);
	}
	else if (LPC_GPIOINT->IO0IntStatR & (1<<3))
	{
		auxiliar &= ~(1<<0);
		LPC_GPIOINT->IO0IntClr |=(1<<3);
	}
	else{}
	cantidadNumeros++;
	flagAlmacenar = 1;
	if(cantidadNumeros>199)
	{
		static volatile int i=1;
		cantidadNumeros=0;
		if(i<30)
		{
			NVIC_SetPriority(EINT3_IRQn,i);
			i++;
		}
		else
		{
			NVIC_DisableIRQ(EINT3_IRQn);
		}
	}
}
