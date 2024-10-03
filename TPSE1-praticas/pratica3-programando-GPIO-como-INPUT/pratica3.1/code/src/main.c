#include "hw_types.h"
#include "soc_AM335x.h"

#define TIME                                            100000000000
#define TOGGLE                                          (0x01u)

// Definindo a ativação do clock.
#define SOC_PRCM_REGS		                            (0x44E00000)
#define SOC_CM_PER_REGS		                            (SOC_PRCM_REGS + 0)

#define CM_PER_GPIO1_CLKCTRL                            0xAC
#define CM_PER_GPIO1_CLKCTRL_MODULEMODE_ENABLE	        (0x2u)
#define CM_PER_GPIO1_CLKCTRL_OPTFCLKEN_GPIO_1_GDBCLK	(0x00040000u)

// Definindo a configuração do MUX.
#define CM_conf_gpmc_a5                                 0x0854
#define CM_conf_gpmc_a6                                 0x0858
#define CM_conf_gpmc_a7                                 0x085C
#define CM_conf_gpmc_a8                                 0x0860
#define GPMC_BEn1                                       0x878


// Definição da direção do pino.
#define GPIO_OE                                         0x134
#define GPIO_CLEARDATAOUT                               0x190
#define GPIO_SETDATAOUT                                 0x194
#define GPIO_DATAIN                                     0x138

// Definição do botão.
#define BUTTON_PIN                                      (1<<28)  // P9_12 corresponde ao GPIO60

unsigned int flagBlink0;

static void delay();
static void ledInit();
static void ledToggle0();
static int buttonPressed();
static void initButton();

int main(void) {
    flagBlink0 = 0;
    ledInit();
    initButton();
	
    while(1) {
        buttonPressed();
    }
    return (0);
}

// Função de delay para criar uma pausa no código
void delay(volatile unsigned int count) {
    for (int i = 0; i < count; ++i)
        __asm("nop"); 	// A instrução "nop" (No Operation) é usada para garantir que o compilador não otimize um loop vazio, forçando uma pausa real no código.
}

void ledInit() {
    unsigned int val_temp;
    HWREG(SOC_CM_PER_REGS + CM_PER_GPIO1_CLKCTRL) |= CM_PER_GPIO1_CLKCTRL_OPTFCLKEN_GPIO_1_GDBCLK | CM_PER_GPIO1_CLKCTRL_MODULEMODE_ENABLE;
    HWREG(SOC_CONTROL_REGS + CM_conf_gpmc_a5) |= 7;
    HWREG(SOC_CONTROL_REGS + CM_conf_gpmc_a6) |= 7;
    HWREG(SOC_CONTROL_REGS + CM_conf_gpmc_a7) |= 7;
    HWREG(SOC_CONTROL_REGS + CM_conf_gpmc_a8) |= 7;

    val_temp = HWREG(SOC_GPIO_1_REGS + GPIO_OE);
    val_temp &= ~(1 << 21);
    val_temp &= ~(1 << 22);
    val_temp &= ~(1 << 23);
    val_temp &= ~(1 << 24);
    HWREG(SOC_GPIO_1_REGS + GPIO_OE) = val_temp;
}

void initButton(){
    unsigned int val_temp;
    HWREG(SOC_CM_PER_REGS + CM_PER_GPIO1_CLKCTRL) |= CM_PER_GPIO1_CLKCTRL_OPTFCLKEN_GPIO_1_GDBCLK | CM_PER_GPIO1_CLKCTRL_MODULEMODE_ENABLE;
    HWREG(SOC_CONTROL_REGS + GPMC_BEn1) |= 7;

    val_temp = (SOC_CONTROL_REGS + GPIO_OE);
    val_temp |= (1 << 28);
    HWREG(SOC_CONTROL_REGS + GPIO_OE) = val_temp;
}

int buttonPressed() {
    if ((HWREG(SOC_GPIO_1_REGS + GPIO_DATAIN) & (1 << 28)) != 0)
        ledToggle0();
}

void ledToggle0(volatile unsigned int i) {
    flagBlink0 ^= TOGGLE;

    if(flagBlink0) 
        HWREG(SOC_GPIO_1_REGS + GPIO_SETDATAOUT) = 1 << 21;
    else 
        HWREG(SOC_GPIO_1_REGS + GPIO_CLEARDATAOUT) = 1 << 21;
}
