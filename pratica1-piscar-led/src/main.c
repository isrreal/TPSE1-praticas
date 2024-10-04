/*
	CÓDIGO PARA O BARRAMENTO P8
	
	
	
	Qual o procedimento para os leds ligarem?
	
	1. O módulo de controle (no caso, GPIO0) é responsável pela configuração inicial e pode responder ao POR (Power on Reset), além de outras funções de configuração de pinos.
	
	2. Na inicialização, somente os componentes que serão utilizados serão ligados. 
		O registrador SOC_CM_PER_REGS juntamente com o registrador do módulo CM_conf_gpmc_* 
		gerenciam a desativação e a ativação dos clocks aos módulos da placa, no caso, os módulos GPIO1-3.
	
	
	3. Após o POR, o programa DEVE definir quais as funcionalidades dos registradores a serem utilizados.
	
	4. O registrador GPIO_OE carrega a configuração de saída dos pinos da GPIO referenciada.
	
	5. O registrador GPIO_DATAOUT é manipulado pelos registradores GPIO_SETDATAOUT
		(atribui 1 a algum dos 32 pinos do registrador GPIO_DATAOUT);
	   	GPIO_CLEARDATAOUT (atribui 0 a algum dos 32 pinos do registrador GPIO_DATAOUT).
	   

*/

/*
	O que um pino faz?
	Resposta: É a interface principal entre o microcontrolador e o mundo externo, referida como PAD (Pad ou ponto de acesso físico).
	
	
	O que é GPIO? 
		Resposta: (General Purpose Input Output) é um módulo com registradores 
		A BBB tem 4 módulos GPIO:
			GPIO0: utilizado para "acordar" o dispositivo por meio de fontes externas
			GPIO1 ao GPIO4: São do domínio periférico, são configurados para aplicações como I/O de dados e interface de teclado. 

	O que é um controlador de memória? O que faz?
		Resposta: É um componente essencial dentro de um microcontrolador ou microprocessador que gerencia 
			a leitura e escrita na memória.
	
	
	O que é um GPMC?
		Resposta: GPMC (General Purpose Memory Controller) é um controlador de memória de uso geral 
			utilizado para interfaces de memória externa como SRAM, NOR Flash, e NAND Flash. 
			Os registradores conf_<módulo>_<pino> configuram os pinos associados a 
			diferentes funções do sistema, podendo incluir GPMC, mas não se limitando a ele.
			
	Afinal, o que esse gpmc faz com os pinos a_[0-27]?
		Resposta: gerencia o fluxo de bits de saída a esses pinos específicos

	O que é um módulo de controle?
		Resposta: Região que gerencia e coordena operações de diferentes partes do sistema.
		
*/


#include "soc_AM335x.h"
#include "hw_types.h"

#define CM_PER_GPIO1_CLKCTRL                                       (0xAC) // Esse registrador controla o 'power domain state', que é uma região do chip que pode ser controlado independentemente das outras. Controla o clock da GPIO1, liga seus componentes, especificamente.

// Controla a forma que os clocks são gerenciados.
#define CM_PER_GPIO1_CLKCTRL_MODULEMODE_ENABLE                     (0x2u) // Define o módulo como habilitado.
#define CM_PER_GPIO1_CLKCTRL_OPTFCLKEN_GPIO_1_GDBCLK               (0x000040000u) // Habilita o clock opcional para a GPIO1.


// Endereços dos módulos de controle GPMC 
#define CM_conf_gpmc_a5                                            (0x0854) // Controla a configuração do pino a5 do módulo gpmc.
#define CM_conf_gpmc_a6                                            (0x0858) // Controla a configuração do pino a6 do módulo gpmc. 
#define CM_conf_gpmc_a7                                            (0x085C) // Controla a configuração do pino a7 do módulo gpmc.
#define CM_conf_gpmc_a8                                            (0x0860) // Controla a configuração do pino a8 do módulo gpmc.

// Manipulam os bits dos pinos da GPIO, desligando ou ligando.

// O que é esse registrador GPIO_DATAOUT?
// Resposta: é o registrador utilizado para definir o valor dos pinos GPIO de saída.
// Esse registrador pode ser acessado com operações diretas de escrita e leitura.

// Os registradores GPIO (General Purpose Input Output), têm 32 pinos e 32 bits
 
#define GPIO_OE                                                    (0x134) // O registrador é usado para habilitar as capacidades de saída do pino
#define GPIO_SETDATAOUT                                            (0x194) // Seta 1 em algum dos pinos do registrador GPIO_DATAOUT.
#define GPIO_CLEARDATAOUT                                          (0x190) // Seta 0 em algum dos pinos do registrador GPIO_DATAOUT.

void delay(unsigned int count) {
    while(count--) {
        __asm("nop"); 	// A instrução "nop" (No Operation) é usada para garantir que o compilador não otimize um loop vazio, forçando uma pausa real no código.
    }
}

void inicializarLed() {
	
	// SOC_CM_PER_REGS é um registrador que gerencia o direcionamento de energia aos outros módulos da placa.
	
	// CM_PER_GPIO1_CLKCTRL é o registrador que controla o direcionamento de energia do módulo de controle GPIO1.
	
	// o endereço dado por SOC_CM_PER_REGS + CM_PER_GPIO1_CLKCTRL gerencia a ativação e desativação dos clocks ao módulo GPIO1.
	
    HWREG(SOC_CM_PER_REGS + CM_PER_GPIO1_CLKCTRL) |= CM_PER_GPIO1_CLKCTRL_OPTFCLKEN_GPIO_1_GDBCLK | CM_PER_GPIO1_CLKCTRL_MODULEMODE_ENABLE;

    // O que significa o esse 7?
    	// Resposta: Que o pino referenciado será configurado como interface de saída.
    // SOC_CONTROL_REGS é o endereço base do módulo de controle de memória dos registradores mapeados	
    // Configura os pinos a5, a6, a7, a8 do módulo gpmc como GPIO de saída.
    HWREG(SOC_CONTROL_REGS + CM_conf_gpmc_a5) = 7;
    HWREG(SOC_CONTROL_REGS + CM_conf_gpmc_a6) = 7;
    HWREG(SOC_CONTROL_REGS + CM_conf_gpmc_a7) = 7;
    HWREG(SOC_CONTROL_REGS + CM_conf_gpmc_a8) = 7;
   // SOC_GPIO_1_REGS endereço base doS registradores mapeados na memória
   // Desligando os bits dos leds GPIO1_21, GPIO1_22, GPIO1_23, GPIO1_24
    HWREG(SOC_GPIO_1_REGS + GPIO_OE) &= ~(1 << 21);
    HWREG(SOC_GPIO_1_REGS + GPIO_OE) &= ~(1 << 22);
    HWREG(SOC_GPIO_1_REGS + GPIO_OE) &= ~(1 << 23);
    HWREG(SOC_GPIO_1_REGS + GPIO_OE) &= ~(1 << 24);
}

void ligarLed() {
    static unsigned char blink = 0;
 
    if (blink) {
        // Liga os LED
        HWREG(SOC_GPIO_1_REGS + GPIO_SETDATAOUT) = (1 << 21) | (1 << 22) | (1 << 23) | (1 << 24);
    } else {
        // Desliga os LEDs
        HWREG(SOC_GPIO_1_REGS + GPIO_CLEARDATAOUT) = (1 << 21) | (1 << 22) | (1 << 23) | (1 << 24);
    }

    // Atraso para criar o efeito de piscar
    delay(100000);
    // Alterna o estado dos LEDs
    blink = !blink;
}

int main(void) {
    inicializarLed();
    while(1)
        ligarLed();
    return 0;
}




































