/* 

SDRAM code implementation
*/
//=================================================================================================
// STM32F429I-Discovery SDRAM configuration
// Author : Radoslaw Kwiecien
// e-mail : radek@dxp.pl
// http://en.radzio.dxp.pl/stm32f429idiscovery/
// Date : 24.11.2013
//=================================================================================================
#include "stm32f7.h"
//#include "gpiof4.h"
#include "MT48LC4M32_sdram.h"
//=================================================================================================
// Macros for SDRAM Timing Register
//=================================================================================================
#define TMRD(x) (x << 0)
#define TXSR(x) (x << 4)
#define TRAS(x) (x << 8)
#define TRC(x)  (x << 12)
#define TWR(x)  (x << 16)
#define TRP(x)  (x << 20)
#define TRCD(x) (x << 24)

#define FILL_PATTERN	0x001F //001F

//=================================================================================================
// GPIO configuration data (16bit data bus)
//=================================================================================================
static  GPIO_TypeDef * const GPIOInitTable[] = {
	GPIOC,
	GPIOD, GPIOD,GPIOD,GPIOD,GPIOD,GPIOD,GPIOD,
	GPIOE, GPIOE,GPIOE,GPIOE,GPIOE,GPIOE,GPIOE,GPIOE,GPIOE,GPIOE,GPIOE,
	GPIOF, GPIOF,GPIOF,GPIOF,GPIOF,GPIOF,GPIOF,GPIOF,GPIOF,GPIOF,GPIOF,
	GPIOG, GPIOG,GPIOG,GPIOG,GPIOG,GPIOG,
	GPIOH, GPIOH,
	0
};
static uint8_t const PINInitTable[] = {
	3,								// C
	0,1,8,9,10,14,15,				// D
	0,1,7,8,9,10,11,12,13,14,15,	// E
	0,1,2,3,4, 5,11,12,13,14,15,	// F
	0,1,4,5,8,15,					// G
	3,5,							// H
	0
};

//=================================================================================================
// SDRAM_Init function
//=================================================================================================
void SDRAM_Init(uint8_t databits)
{
	volatile uint32_t ptr = 0, pp = 0;
	volatile uint32_t i = 0;
	volatile uint8_t dbits = 0;

	uint32_t tmpmrd = (uint32_t) SDRAM_MODEREG_BURST_LENGTH_1 | \
								SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL | \
								SDRAM_MODEREG_CAS_LATENCY_2 | \
								SDRAM_MODEREG_OPERATING_MODE_STANDARD | \
								SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

	// io pin enable
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIODEN | RCC_AHB1ENR_GPIOEEN | RCC_AHB1ENR_GPIOFEN | RCC_AHB1ENR_GPIOGEN | RCC_AHB1ENR_GPIOHEN;

	while (GPIOInitTable[i] != 0) {
		gpio_conf(GPIOInitTable[i], PINInitTable[i], MODE_AF, TYPE_PUSHPULL, SPEED_100MHz, PULLUP_UP, 12);
		i++;
	}

	RCC->AHB3ENR |= RCC_AHB3ENR_FMCEN;

	// Initialization step 1
	//FMC_Bank5_6->SDCR[0] = FMC_SDCR1_SDCLK_1 | FMC_SDCR1_RBURST | FMC_SDCR1_RPIPE_1 | FMC_SDCR1_NR_0 | FMC_SDCR1_MWID_0 | FMC_SDCR1_NB | FMC_SDCR1_CAS;
	if (databits == 16)
		dbits = FMC_SDCR1_MWID_0;
	else if (databits == 32)
		dbits = FMC_SDCR1_MWID_1;
	else // databits == 8
		dbits = 0;

	FMC_Bank5_6->SDCR[0] = FMC_SDCR1_SDCLK_1 | FMC_SDCR1_RBURST | FMC_SDCR1_NR_0 | FMC_SDCR1_NB | FMC_SDCR1_CAS_1 | dbits;

	//FMC_Bank5_6->SDCR[1] = FMC_SDCR2_NR_0 | FMC_SDCR2_NB | FMC_SDCR2_CAS_1 | FMC_SDCR2_MWID_0;
	// Initialization step 2
	FMC_Bank5_6->SDTR[0] = TMRD(2) | TXSR(7) | TRAS(4) | TRC(7) |  TWR(2) | TRP(2) | TRCD(2);
	//FMC_Bank5_6->SDTR[1] = TMRD(2) | TXSR(7) | TRAS(4) | TWR(4) | TRCD(2);

	// Initialization step 3	
	while (FMC_Bank5_6->SDSR & FMC_SDSR_BUSY);

	FMC_Bank5_6->SDCMR = 1 | FMC_SDCMR_CTB1 | (1 << 5);
	// Initialization step 4
	for (i = 0; i < 1000000; i++);
	// Initialization step 5
	while (FMC_Bank5_6->SDSR & FMC_SDSR_BUSY);
	FMC_Bank5_6->SDCMR = 2 | FMC_SDCMR_CTB1 | (1 << 5);
	// Initialization step 6
	while (FMC_Bank5_6->SDSR & FMC_SDSR_BUSY);
	FMC_Bank5_6->SDCMR = 3 | FMC_SDCMR_CTB1 | (8 << 5);
	// Initialization step 7
	while (FMC_Bank5_6->SDSR & FMC_SDSR_BUSY);
	FMC_Bank5_6->SDCMR = 4 | FMC_SDCMR_CTB1 | (1 << 5) | (tmpmrd << 9);
	// Initialization step 8
	while (FMC_Bank5_6->SDSR & FMC_SDSR_BUSY);
	FMC_Bank5_6->SDRTR |= (REFRESH_COUNT << 1);
	while (FMC_Bank5_6->SDSR & FMC_SDSR_BUSY);
	
	// Clear SDRAM
	for (ptr = SDRAM_BASE; ptr < (SDRAM_BASE + SDRAM_SIZE); ptr += 2) {
		*((uint16_t *)ptr) = FILL_PATTERN;
		//pp = *((uint32_t *)ptr);
		//if (pp != FILL_PATTERN) {
		//	STM32F7_DebugLed(true);
		//}
	}
}

void gpio_conf(GPIO_TypeDef * GPIO, uint8_t pin, uint8_t mode, uint8_t type, uint8_t speed, uint8_t pullup, uint8_t af)
{
	GPIO->MODER = (GPIO->MODER   & MASK2BIT(pin)) | (mode << (pin * 2));
	GPIO->OTYPER = (GPIO->OTYPER  & MASK1BIT(pin)) | (type << pin);
	GPIO->OSPEEDR = (GPIO->OSPEEDR & MASK2BIT(pin)) | (speed << (pin * 2));
	GPIO->PUPDR = (GPIO->PUPDR   & MASK2BIT(pin)) | (pullup << (pin * 2));
	if (pin > 7)
	{
		GPIO->AFR[1] = (GPIO->AFR[1] & AFMASKH(pin)) | (af << ((pin - 8) * 4));
	}
	else
	{
		GPIO->AFR[0] = (GPIO->AFR[0] & AFMASKL(pin)) | (af << ((pin) * 4));
	}
}


//=================================================================================================
// End of file
//=================================================================================================
