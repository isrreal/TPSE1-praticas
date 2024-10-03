#include "hw_types.h"
#include "soc_AM335x.h"

#define TIME                                            100000000000
#define TOGGLE                                          (0x01u)

// Definindo a ativação do clock.
#define SOC_PRCM_REGS		                        (0x44E00000)
#define SOC_CM_PER_REGS		                        (SOC_PRCM_REGS + 0)

#define CM_PER_GPIO1_CLKCTRL                            0xAC
#define CM_PER_GPIO1_CLKCTRL_MODULEMODE_ENABLE	        (0x2u)
#define CM_PER_GPIO1_CLKCTRL_OPTFCLKEN_GPIO_1_GDBCLK	(0x00040000u)

// Definindo a configuração do MUX.
#define CM_conf_gpmc_a5                                 0x0854
#define CM_conf_gpmc_a6                                 0x0858
#define CM_conf_gpmc_a7                                 0x085C
#define CM_conf_gpmc_a8                                 0x0860

#define CM_conf_gpmc_ben1                              0x878 // LED externo 1
#define CM_conf_gpmc_ad6                               0x818 // LED externo 2
#define CM_conf_gpmc_ad7                               0x81C // LED externo 3


#define CM_conf_gpmc_a0                                 0x840
#define CM_conf_gpmc_a1                                 0x844

// Definição da direção do pino.

#define GPIO_OE                                         0x134
#define GPIO_CLEARDATAOUT                               0x190
#define GPIO_SETDATAOUT                                 0x194
#define GPIO_DATAIN                                     0x138

// Watchdog
#define WDT1 						0x44E35000
#define WDT_WSPR 					0x48
#define WDT_WWPS					0x34
#define W_PEND_WSPR					(1 << 0x4u)

// Interrupção

#define INTC_CONTROL    				0x48
#define GPIO_IRQSTATUS_RAW_0                            0x24
#define INTC_SIR_IRQ    				0x40
#define GPIO_IRQSTATUS_SET_0                            0x34
#define GPIO_IRQSTATUS_1                                0x30
#define GPIO_IRQSTATUS_0                                0x2C
#define GPIO_IRQSTATUS_SET_1                            0x38
#define GPIO_RISINGDETECT                               0x148
#define GPIO_FALLINGDETECT                              0x14C
#define INTC_BASE       				0x48200000
#define MIR_CLEAR0                                      0x88
#define MIR_CLEAR1                                      0xA8
#define MIR_CLEAR2 					0xC8
#define MIR_CLEAR3 					0xE8

unsigned int flagBlink0;

static void watchdog();
void pinInterrup(bool x, int pino);
void delay();
void ledInitEX();
void initButton();
void ledToggle0();
bool buttonPressed1();
bool buttonPressed2();
int Interrupt_Setup(unsigned int inter);
int gpioIsrHandler(bool x, int pino);
void ISR_Handler();

int main(void){
    watchdog();
    flagBlink0 = 0;

    ledInitEX();
    initButton();

    Interrupt_Setup(98); // Configura o sistema para lidar com a interrupção do módulo GPIO1
    pinInterrup(buttonPressed1(), 16); // Configura interrupção para o botão 1 (GPIO1_16)
    pinInterrup(buttonPressed2(), 17); // Configura interrupção para o botão 2 (GPIO1_17)

    while(1) {
        ledToggle0(); // Alterna o estado do LED
        delay(10000000); // Aguarda um tempo para a próxima alternância
    }
    
    return (0);
}

// Essa função é o manipulador de interrupções (ISR), chamada quando ocorre uma interrupção.
// O número da interrupção é obtido de INTC_SIR_IRQ e a interrupção ativa é identificada.
// 0x00 prioridade máxima de interrupção, 0x7F, a mínima.

void ISR_Handler() { 
    unsigned int irq_number = HWREG(INTC_BASE + INTC_SIR_IRQ) & 0x7F; // Obtém o tipo de interrupção ativa (entre 0 e 127)

    if(irq_number == 98) {	// 98 é o número de interrupção para o módulo GPIO1 (GPIO1_16 e GPIO1_17)
        if(HWREG(SOC_GPIO_1_REGS + GPIO_IRQSTATUS_RAW_0) & (1 << 16)) { // Verifica se a interrupção foi causada pelo pino GPIO1_16
            gpioIsrHandler(buttonPressed1(), 16); // Trata e atualiza os status da interrupção do botão 1

            HWREG(SOC_GPIO_1_REGS + GPIO_SETDATAOUT) = 1 << 6;
	    HWREG(SOC_GPIO_1_REGS + GPIO_SETDATAOUT) = 1 << 7;
	    HWREG(SOC_GPIO_1_REGS + GPIO_SETDATAOUT) = 1 << 28;
        }
        
        else if(HWREG(SOC_GPIO_1_REGS + GPIO_IRQSTATUS_RAW_0) & (1 << 17)) { // Verifica se a interrupção foi causada pelo pino GPIO1_17
            gpioIsrHandler(buttonPressed2(), 17); // Trata e atualiza os status da interrupção do botão 1

            HWREG(SOC_GPIO_1_REGS + GPIO_CLEARDATAOUT) = 1 << 6;
	    HWREG(SOC_GPIO_1_REGS + GPIO_CLEARDATAOUT) = 1 << 7;
	    HWREG(SOC_GPIO_1_REGS + GPIO_CLEARDATAOUT) = 1 << 28;
        }
    }

    HWREG(INTC_BASE + INTC_CONTROL) = 0x1; // Finaliza a interrupção e prepara o sistema para a próxima
}

// Essa função ativa o status da interrupção para o pino específico que a causou
int gpioIsrHandler(bool x, int pino) {
    if(!x)
        HWREG(SOC_GPIO_1_REGS + GPIO_IRQSTATUS_0) |= (1 << pino); // Ativa o status da interrupção para GPIO_IRQSTATUS_0
    else	
        HWREG(SOC_GPIO_1_REGS + GPIO_IRQSTATUS_1) |= (1 << pino); // Ativa o status da interrupção para GPIO_IRQSTATUS_1
    return 1;
}

// Configura a interrupção para o pino selecionado
void pinInterrup(bool x, int pino) {
    if(!x) {
        HWREG(SOC_GPIO_1_REGS + GPIO_IRQSTATUS_SET_0) |= (1 << pino); // Ativa a interrupção para o pino no GPIO_IRQSTATUS_SET_0
        HWREG(SOC_GPIO_1_REGS + GPIO_RISINGDETECT) |= (1 << pino); // Habilita detecção de subida no pino (quando o botão é pressionado)
    }
    else {
        HWREG(SOC_GPIO_1_REGS + GPIO_IRQSTATUS_SET_1) |= (1 << pino); // Ativa a interrupção para o pino no GPIO_IRQSTATUS_SET_1
        HWREG(SOC_GPIO_1_REGS + GPIO_RISINGDETECT) |= (1 << pino); // Habilita detecção de subida no pino (quando o botão é pressionado)
    }
}

// Configura o sistema de interrupção com o número de interrupção
int Interrupt_Setup(unsigned int inter) {
    if(inter < 0 || inter > 127){
        return false;
    }
    
    unsigned int aux = inter >> 5; // Define o índice de qual MIR_CLEAR (0-3) será usado, já que cada um controla 32 interrupções

    // Limpa o bit correspondente ao número da interrupção nos registradores MIR_CLEAR
    switch(aux){
        case 0:
            HWREG(INTC_BASE + MIR_CLEAR0) |= (1 << (inter % 32)); 
            break;
        case 1:
            HWREG(INTC_BASE + MIR_CLEAR1) |= (1 << (inter % 32));
            break;
        case 2:
            HWREG(INTC_BASE + MIR_CLEAR2) |= (1 << (inter % 32));
            break;
        case 3:
            HWREG(INTC_BASE + MIR_CLEAR3) |= (1<< (inter % 32));
            break;
        default:
            break;
    }
    return true;
}

// Função de delay para criar uma pausa no código
void delay(unsigned int count) {
    for (int i = 0; i < count; ++i)
        __asm("nop"); 	// A instrução "nop" (No Operation) é usada para garantir que o compilador não otimize um loop vazio, forçando uma pausa real no código.
}

// Inicializa o LED externo (GPIO1_28) como saída
void ledInitEX(){
    unsigned int val_temp;
    HWREG(SOC_CM_PER_REGS + CM_PER_GPIO1_CLKCTRL) |= CM_PER_GPIO1_CLKCTRL_OPTFCLKEN_GPIO_1_GDBCLK | CM_PER_GPIO1_CLKCTRL_MODULEMODE_ENABLE;
    HWREG(SOC_CONTROL_REGS + CM_conf_gpmc_ben1) |= 7;
    HWREG(SOC_CONTROL_REGS + CM_conf_gpmc_ad6) |= 7;
    HWREG(SOC_CONTROL_REGS + CM_conf_gpmc_ad7) |= 7;

    
    val_temp = HWREG(SOC_GPIO_1_REGS + GPIO_OE);
    val_temp &= ~(1 << 28); // Configura o GPIO1_28 como saída
    HWREG(SOC_GPIO_1_REGS + GPIO_OE) = val_temp;
    
    val_temp = HWREG(SOC_GPIO_1_REGS + GPIO_OE);
    val_temp &= ~(1 << 6); // Configura o GPIO1_6 (P8) como saída
    HWREG(SOC_GPIO_1_REGS + GPIO_OE) = val_temp;
    
    val_temp = HWREG(SOC_GPIO_1_REGS + GPIO_OE);
    val_temp &= ~(1 << 7); // Configura o GPIO1_7 (P8) como saída
    HWREG(SOC_GPIO_1_REGS + GPIO_OE) = val_temp;
}

// Alterna o estado do LED (acende/apaga)

void ledToggle0(){
    flagBlink0 ^= TOGGLE;

    if(flagBlink0) {
        HWREG(SOC_GPIO_1_REGS + GPIO_SETDATAOUT) = 1 << 6;
        HWREG(SOC_GPIO_1_REGS + GPIO_SETDATAOUT) = 1 << 7;
        HWREG(SOC_GPIO_1_REGS + GPIO_SETDATAOUT) = 1 << 28;
    }
    else {
        HWREG(SOC_GPIO_1_REGS + GPIO_CLEARDATAOUT) = 1 << 6;
        HWREG(SOC_GPIO_1_REGS + GPIO_CLEARDATAOUT) = 1 << 7;
        HWREG(SOC_GPIO_1_REGS + GPIO_CLEARDATAOUT) = 1 << 28;
    }	
}

// Verifica se o botão 1 (GPIO1_16) está pressionado
bool buttonPressed1() {
    if((HWREG(SOC_GPIO_1_REGS + GPIO_DATAIN) & (1 << 16)) != 0)
        return true;
    return false;
}

// Verifica se o botão 2 (GPIO1_17) está pressionado
bool buttonPressed2() {
    if((HWREG(SOC_GPIO_1_REGS + GPIO_DATAIN) & (1 << 17)) != 0)
        return true;
    return false;
}

// Inicializa os botões conectados aos pinos GPIO1_16 e GPIO1_17 como entrada
void initButton() {
    unsigned int val_temp;
    
    HWREG(SOC_CM_PER_REGS + CM_PER_GPIO1_CLKCTRL) |= CM_PER_GPIO1_CLKCTRL_OPTFCLKEN_GPIO_1_GDBCLK | CM_PER_GPIO1_CLKCTRL_MODULEMODE_ENABLE;
    HWREG(SOC_CONTROL_REGS + CM_conf_gpmc_a0) |= 0x27;
    HWREG(SOC_CONTROL_REGS + CM_conf_gpmc_a1) |= 0x27;
    
    val_temp = HWREG(SOC_GPIO_1_REGS + GPIO_OE);
    val_temp |= (1 << 16); // Configura o pino 16 do GPIO1 como entrada
    val_temp |= (1 << 17); // Configura o pino 17 do GPIO1 como entrada
    HWREG(SOC_GPIO_1_REGS + GPIO_OE) = val_temp;
}

// Desabilita o watchdog
static void watchdog(){
	HWREG(WDT_WSPR) = 0xAAAA;
	while((HWREG(WDT_WWPS) & (1 << 4)));
	
	HWREG(WDT_WSPR) = 0x5555;
	while((HWREG(WDT_WWPS) & (1 << 4)));
}

