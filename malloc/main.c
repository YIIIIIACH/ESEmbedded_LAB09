#include <stdint.h>
#include <stdlib.h>
#include "reg.h"
#include "blink.h"
#define HEAP_MAX (64 * 1024) //64 KB
void *_sbrk(int incr)
{
	extern uint8_t _mybss_vma_end; //Defined by the linker script
	static uint8_t *heap_end = NULL;
	uint8_t *prev_heap_end;

	if (heap_end == NULL)
		heap_end = &_mybss_vma_end;

	prev_heap_end = heap_end;
	if (heap_end + incr > &_mybss_vma_end + HEAP_MAX)
		return (void *)-1;

	heap_end += incr;
	return (void *)prev_heap_end;
}
void init_usart1(void)
{
	//PB6->tx PB7->rx
	
	
	//RCC EN GPIOB

	SET_BIT(RCC_BASE + RCC_AHB1ENR_OFFSET , GPIO_EN_BIT(GPIO_PORTB));

	//GPIO Configurations
	SET_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_MODER_OFFSET , MODERy_1_BIT(7));
	CLEAR_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_MODER_OFFSET , MODERy_0_BIT(7));
	SET_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_MODER_OFFSET , MODERy_1_BIT(6));
	CLEAR_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_MODER_OFFSET , MODERy_0_BIT(6));   // GPIO MODE
	
	CLEAR_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_OTYPER_OFFSET, OTy_BIT(6));
	CLEAR_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_OTYPER_OFFSET, OTy_BIT(7)); // OT TYPE
	
	CLEAR_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_OSPEEDR_OFFSET, OSPEEDRy_1_BIT(6));
	SET_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_OSPEEDR_OFFSET, OSPEEDRy_0_BIT(6));
	CLEAR_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_OSPEEDR_OFFSET, OSPEEDRy_1_BIT(7));
	SET_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_OSPEEDR_OFFSET, OSPEEDRy_0_BIT(7));

	CLEAR_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_PUPDR_OFFSET, PUPDRy_1_BIT(6));
	CLEAR_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_PUPDR_OFFSET, PUPDRy_0_BIT(6));
	CLEAR_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_PUPDR_OFFSET, PUPDRy_1_BIT(7));
	CLEAR_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_PUPDR_OFFSET, PUPDRy_0_BIT(7));


	//RCC EN USART1
	SET_BIT(RCC_BASE + RCC_APB2ENR_OFFSET , USART1EN);

	WRITE_BITS( GPIO_BASE(GPIO_PORTB) + GPIOx_AFRL_OFFSET, AFRLy_3_BIT(7), AFRLy_0_BIT(7), 7);  // 0111  0111
	WRITE_BITS( GPIO_BASE(GPIO_PORTB) + GPIOx_AFRL_OFFSET, AFRLy_3_BIT(6), AFRLy_0_BIT(6), 7);

	//Baud
	const unsigned int BAUD = 115200;
	const unsigned int SYSCLK_MHZ = 168;
	const double USARTDIV = SYSCLK_MHZ * 1.0e6 / 16 / BAUD;  //因為是 const 且 uint32_t double ok

	const uint32_t DIV_MANTISSA = (uint32_t)USARTDIV;   
	const uint32_t DIV_FRACTION = (uint32_t)((USARTDIV-DIV_MANTISSA)*16);  //actually 2.32

	//USART  Configurations
	
	WRITE_BITS(USART1_BASE + USART_BRR_OFFSET , DIV_MANTISSA_11_BIT , DIV_MANTISSA_0_BIT , DIV_MANTISSA);
	WRITE_BITS(USART1_BASE + USART_BRR_OFFSET , DIV_FRACTION_3_BIT , DIV_FRACTION_0_BIT , DIV_FRACTION);

	SET_BIT(USART1_BASE + USART_CR1_OFFSET , UE_BIT);
	SET_BIT(USART1_BASE + USART_CR1_OFFSET , TE_BIT);
	SET_BIT(USART1_BASE + USART_CR1_OFFSET , RE_BIT);
}

void usart_interupt_ini()
{
	SET_BIT( USART1_BASE + USART_CR1_OFFSET , RXNEIE_BIT);  //Set RXNEIE bit in USART_CR1
	
	SET_BIT(NVIC_ISER_BASE + NVIC_ISERn_OFFSET(1), 5); //configure NVIC  ISQ = 37  => ISR1 5 BIT	
}
void usart1_send_char(const char ch)
{
	while(!READ_BIT(USART1_BASE + USART_SR_OFFSET, TXE_BIT))
		;
	REG(USART1_BASE + USART_DR_OFFSET) = ch;
}

char usart1_receive_char(void)
{
	while(!READ_BIT(USART1_BASE + USART_SR_OFFSET, RXNE_BIT))
		;		
	return (char)REG(USART1_BASE + USART_DR_OFFSET);
}

void usart1_handler(void)
{	
	CLEAR_BIT(USART1_BASE + USART_SR_OFFSET , RXNE_BIT);
	CLEAR_BIT(USART1_BASE + USART_SR_OFFSET , ORE_BIT);  //handle the overrun error in your handler (the ORE bit in USART_SR register)
	blink(15);
}

int main(void)
{
	init_usart1();

	char *ch = malloc(3 * sizeof(char));
    	if (ch != NULL)
    	{
        	ch[0] = 'A';
        	ch[1] = 'B';
        	ch[2] = 'C';

        	usart1_send_char(ch[0]);
        	usart1_send_char(ch[1]);
        	usart1_send_char(ch[2]);

        	free(ch);
    	}

	blink(LED_BLUE);
}



