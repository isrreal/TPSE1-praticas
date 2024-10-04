#include "soc_AM335x.h"
#include "hw_types.h"

#define TIME                                            100000000000
#define TOGGLE                                          (0x01u)

// Definindo a ativação do clock.
#define SOC_PRCM_REGS                                   (0x44E00000)
#define SOC_CM_PER_REGS                                 (SOC_PRCM_REGS + 0)

#define CM_PER_GPIO1_CLKCTRL                            0xAC
#define CM_PER_GPIO1_CLKCTRL_MODULEMODE_ENABLE          (0x2u)
#define CM_PER_GPIO1_CLKCTRL_OPTFCLKEN_GPIO_1_GDBCLK    (0x00040000u)

// Definindo a configuração do MUX.
#define CM_conf_gpmc_a5                                 0x0854
#define GPMC_BEn1                                       0x878

// Definição da direção do pino.
#define GPIO_OE                                         0x134
#define GPIO_CLEARDATAOUT                               0x190
#define GPIO_SETDATAOUT                                 0x194
#define GPIO_DATAIN                                     0x138

// Definição do botão.
#define BUTTON_PIN                                      (1 << 28)  // P9_12 corresponde ao GPIO60

#define WDT_WSPR                                        (0x44e35048)
#define WDT_WWPS                                        (0x44e35034)

static void delay(volatile unsigned int count);
static void ledInit();
static void ligarLed();
static int botaoPressionado();
static void initButton();
void watchdog();

int main(void) {
    watchdog();    
    ledInit();
    initButton();
    return 0;
}

// Função de delay para criar uma pausa no código
void delay(volatile unsigned int count) {
    for (int i = 0; i < count; ++i)
        __asm("nop"); 	// A instrução "nop" (No Operation) é usada para garantir que o compilador não otimize um loop vazio, forçando uma pausa real no código.
}

void ledInit() {
    HWREG(SOC_CM_PER_REGS+CM_PER_GPIO1_CLKCTRL) |= CM_PER_GPIO1_CLKCTRL_OPTFCLKEN_GPIO_1_GDBCLK | CM_PER_GPIO1_CLKCTRL_MODULEMODE_ENABLE;
    HWREG(SOC_CONTROL_REGS+CM_conf_gpmc_a5) |= 7;
}

void initButton() {
    HWREG(SOC_CM_PER_REGS+CM_PER_GPIO1_CLKCTRL) |= CM_PER_GPIO1_CLKCTRL_OPTFCLKEN_GPIO_1_GDBCLK | CM_PER_GPIO1_CLKCTRL_MODULEMODE_ENABLE;
    HWREG(SOC_CONTROL_REGS+GPMC_BEn1) |= 7;

    HWREG(SOC_GPIO_1_REGS + GPIO_OE) |= (1 << 28);
}

void ligarLed() {
    static int flagBlink = 0;
    flagBlink ^= TOGGLE;
    if(flagBlink) {
        HWREG(SOC_GPIO_1_REGS+GPIO_SETDATAOUT) = (1 << 21);
    } else {
        HWREG(SOC_GPIO_1_REGS+GPIO_CLEARDATAOUT) = (1 << 21);
    delay(100000);
}

void watchdog() {
    HWREG(WDT_WSPR) = 0xAAA;
    while (HWREG(WDT_WWPS) & (1 << 4));
    HWREG(WDT_WSPR) = 0x5555;
    while (HWREG(WDT_WWPS) & (1 << 4));
}

