/*
===============================================================================
 Name        : ejercicioParcial1.c
 Author      : Alfici, Facundo Ezequiel(author)
 Version     : 1.0
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/
/*
 * Consigna
 * Utilizando Systick e interrupciones externas escribir un código en C que cuente indefinidamente de 0 a 9.
 *Un pulsador conectado a Eint0 reiniciará la cuenta a 0 y se mantendrá en ese valor mientras el pulsador se encuentre presionado.
 *Un pulsador conectado a Eint1 permitirá detener o continuar la cuenta cada vez que sea presionado
 *Un pulsador conectado a Eint2 permitirá modificar la velocidad de incremento del contador.
 *En este sentido, cada vez que se presione ese pulsador el contador pasará a incrementar su cuenta de cada 1 segundo a cada 1 milisegundo y viceversa.
 *Considerar que el microcontrolador se encuentra funcionando con un reloj (cclk) de 16 Mhz.
 *El código debe estar debidamente comentado y los cálculos realizados claramente expresados.
 *En la siguiente figura se muestra una tabla que codifica el display y el esquema del hardware sobre el que funcionará el programa.
 */
#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

volatile uint8_t pausarCuenta = 0;
volatile uint8_t cambiarVelocidad = 0;
volatile uint32_t ticks1ms= 15999;
const uint8_t tablaSegmentos[16] =
	{
		0x3F, // 0
	    0x06, // 1
	    0x5B, // 2
	    0x4F, // 3
	    0x66, // 4
	    0x6D, // 5
	    0x7D, // 6
	    0x07, // 7
	    0x7F, // 8
	    0x6F, // 9
	};

void configSysTick(uint32_t ticks);
void configEInt(void);
void configGPIO(void);
void SysTick_Handler(void);
void EINT0_IRQHandler(void);
void EINT1_IRQHandler(void);
void EINT2_IRQHandler(void);

int main(void)
{
	configSysTick(ticks1ms);
	configEInt();
	configGPIO();
    while(1)
    {}
    return 0 ;
}

void configGPIO(void)
{
	LPC_PINCON->PINSEL1 &= ~((3<<0)|(3<<2)|(3<<4)|(3<<6)|(3<<8)|(3<<10)|(3<<12)); //Pines GPIO para el display
	LPC_GPIO0->FIODIR |= (1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<6); //Todos los pines del display como salidas.
}

void configEInt(void)
{
	LPC_PINCON->PINSEL4 |= ((1<<20)|(1<<22)|(1<<24)); // Configuracion de las 3 interrupciones
	LPC_SC->EXTMODE |=((1<<1)|(1<<2)); //EINT1 y EINT2 por flanco
	LPC_SC->EXTMODE &= ~(1<<0); //EINT0 por nivel (Viene en la lógica del programa)
	LPC_SC->EXTPOLAR &= ~((1<<1)|(1<<2)); //Dado que son por flanco, elijo ambas en logica negativa por el grafico.
	LPC_SC->EXTPOLAR |= (1<<0);//Por nivel alto la EINT0
	NVIC_EnableIRQ(EINT0_IRQn);
	NVIC_EnableIRQ(EINT1_IRQn);
	NVIC_EnableIRQ(EINT2_IRQn);
	NVIC_SetPriority(EINT0_IRQn,1);
	NVIC_SetPriority(EINT1_IRQn,2);
	NVIC_SetPriority(EINT2_IRQn,3);
}

void configSysTick(uint32_t ticks)
{
	//Considerando 16MHz de CCLK
	SysTick->LOAD = ticks;//Fórmula : (CCLK*T)-1 = (16MHz*1mS)-1 = 15999
	SysTick->VAL = 0;
	SysTick->CTRL = (1<<0)|(1<<1)|(1<<2);
	NVIC_EnableIRQ(SysTick_IRQn);
	NVIC_SetPriority(SysTick_IRQn,4);
}

void EINT0_IRQHandler(void)
{
	LPC_GPIO0->FIOCLR |= (1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<6);
	LPC_GPIO0->FIOSET |= tablaSegmentos[0];
	LPC_SC->EXTINT|= (1<<0); //Bajo la bandera de la EINT0
}

void EINT1_IRQHandler(void)
{
	pausarCuenta=!pausarCuenta;
	LPC_SC->EXTINT|= (1<<1); //Bajo la bandera de la EINT1
}

void EINT2_IRQHandler(void)
{
	cambiarVelocidad=!cambiarVelocidad;
	if(cambiarVelocidad==0)
		configSysTick(ticks1ms);
	else
		configSysTick(15999999); // Ticks para 1s
	LPC_SC->EXTINT|= (1<<2); //Bajo la bandera de la EINT2
}

void SysTick_Handler(void)
{
	static volatile uint8_t indice=0;
	if(pausarCuenta==0)
	{
		LPC_GPIO0->FIOSET|= tablaSegmentos[indice];
		indice++;
		if(indice>9)
			indice=0;
	}
	else
	{}
}
