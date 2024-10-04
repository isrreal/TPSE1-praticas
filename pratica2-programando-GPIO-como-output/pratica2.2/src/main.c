#include "soc_AM335x.h"
#include "hw_types.h"

#define CM_PER_GPIO1_CLKCTRL                                       (0xAC)
#define CM_PER_GPIO1_CLKCTRL_MODULEMODE_ENABLE                     (0x2u)
#define CM_PER_GPIO1_CLKCTRL_OPTFCLKEN_GPIO_1_GDBCLK               (0x000040000u)

#define CM_conf_gpmc_b1                                            (0x087C) // Controla a configuração do pino GPIO28 (P9_12).
#define GPIO_OE                                                    (0x134)
#define GPIO_SETDATAOUT                                            (0x194)
#define GPIO_CLEARDATAOUT                                          (0x190)

// Função de delay para criar uma pausa no código
void delay(volatile unsigned int count) {
    for (int i = 0; i < count; ++i)
        __asm("nop"); 	// A instrução "nop" (No Operation) é usada para garantir que o compilador não otimize um loop vazio, forçando uma pausa real no código.
}

void inicializarLedExterno() {
    // Habilita o clock para o módulo GPIO1
    HWREG(SOC_CM_PER_REGS + CM_PER_GPIO1_CLKCTRL) |= 
        CM_PER_GPIO1_CLKCTRL_OPTFCLKEN_GPIO_1_GDBCLK |
        CM_PER_GPIO1_CLKCTRL_MODULEMODE_ENABLE;

    // Configura o pino GPIO28 (P9_12) como GPIO de saída
    HWREG(SOC_CONTROL_REGS + CM_conf_gpmc_b1) = 7;

    // Configura o GPIO28 como saída
    HWREG(SOC_GPIO_1_REGS + GPIO_OE) &= ~(1 << 28);
    HWREG(SOC_GPIO_1_REGS + GPIO_OE) |= (0 << 28);
}

void piscarLedExterno() {
    HWREG(SOC_GPIO_1_REGS + GPIO_SETDATAOUT) = (1 << 28); // Liga o LED
    delay(1000000);
    HWREG(SOC_GPIO_1_REGS + GPIO_CLEARDATAOUT) = (1 << 28); // Desliga o LED
    delay(1000000);
}

int main(void) {
    inicializarLedExterno();
    while(1) {
        piscarLedExterno();
    }
    return 0;
}









































































