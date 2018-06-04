/* SSC-0112 - Organizacao de Computadores Digitais */
/* Turma A - 2018/01                               */
/* Prof. Paulo Sergio Lopes de Souza               */
/*                                                 */
/* Trabalho 2                                      */
/* Simulação de uma CPU MIPS Multiciclo de 32 bits */
/*                                                 */
/* Aluno                          NUSP             */
/* Felipe Scrochio Custodio       9442688          */
/* Gabriel Henrique Scalici       9292970          */
/* Juliano Fantozzi               9791218          */
/* Andre Luis Storino Junior      9293668          */


// +------------------------+
// |  DECISÕES DE PROJETO   |
// +------------------------+
/*
* MUX possui como identificador a unidade funcional que ele dá a
* resposta.
* MUX_IDENTIFICADOR*
*
*  ANDI usa código de operação 11 do ALUOp
*/

#include <stdio.h>
#include <stdlib.h>

// +--------+
// |  pow   |
// +--------+
#include <math.h>

#define DEBUG 1
#define IF_DEBUG if (DEBUG)

// obter bit na posição p do binário x
// shift de p posições para a direita
// bit da posição p agora está acima do 1
// and 1 retorna o bit p.
#define GETBIT(x,p) (((x)>>(p))&1)

#define TRUE  1
#define FALSE 0

typedef char boolean;
typedef unsigned char byte;
typedef unsigned char bit;
typedef unsigned int word;

#define STATUS_VALID          0
#define STATUS_INVALID_INSTR  1
#define STATUS_INVALID_ACCESS 2
#define STATUS_INVALID_ALU    3
#define STATUS_INVALID_REG    4
#define STATUS_INFINITE       5

#define MAX_RUNS 5000 // limite de execuções para depurar loops infinitos

/*******************************************************/

// +----------+
// | DEBUGGER |
// +----------+
int clocks = 0;
FILE* f_debug = NULL;

// +----------+
// |  STATUS  |
// +----------+
int status = 0; // status da saída
int last_valid = 0; // posição da última instrução válida do programa

// +----------------------+
// |  FUNÇÕES AUXILIARES  |
// +----------------------+

/*
 * bin2dec
 * ----------------------------
 *   Retorna valor decimal convertido de um binário
 *
 *   bit* binary: string que representa o binário
 *   int size: quantidade de bits desse binário
 *
 */
 int bin2dec(bit* binary, int size) {
	int i;
	int sum = 0;
	for (i = 0; i < size; i++) {
		if ((int)(binary[i])) {
			sum += pow(2, i);
		}
	}
	return sum;
 }


 /*
  * status_message
  * ----------------------------
  *   Retorna mensagem de erro de acordo com o
  *   status da saída.
  */
char* status_message() {
    char* exit_message = NULL;
    switch(status) {
        case STATUS_INVALID_INSTR:
            exit_message = "Término devido à tentativa de execução de instrução inválida.\n";
            break;
        case STATUS_INVALID_ACCESS:
            exit_message = "Término devido a acesso inválido de memória.\n";
            break;
        case STATUS_INVALID_ALU:
            exit_message = "Término devido à operação inválida da ULA.\n";
            break;
        case STATUS_INVALID_REG:
            exit_message = "Término devido a acesso inválido ao Banco de Registradores.\n";
            break;
        case STATUS_VALID:
            exit_message = ("Estado da saída ainda é válido.\n");
		    break;
        case STATUS_INFINITE:
            exit_message = ("Provável loop infinito.\n");
    }
    return exit_message;
}


/*******************************************************/

// +---------+
// | MEMÓRIA |
// +---------+
#define MAX_SIZE 128
byte MEMORY[MAX_SIZE];      // memória (vetor de bytes)
byte* memory_pointer;       // acessar bytes da memória individualmente
word* memory_word_pointer;  // acessar bytes em blocos de tamanho da palavra

/*******************************************************/

// +----+
// | IR |
// +----+
bit op_code[6];
bit function[6];
bit rs[5];
bit rt[5];
bit rd[5];
bit immediate[16];
//jump_addr armazenará 26 bits quando sair do IR
bit jump_addr[32];

// +------------------------+
// | BANCO DE REGISTRADORES |
// +------------------------+
typedef unsigned int reg;
reg zero;   // 0
reg at;     // 1
reg v0;     // 2
reg v1;     // 3
reg a0;     // 4
reg a1;     // 5
reg a2;     // 6
reg a3;     // 7
reg t0;     // 8
reg t1;     // 9
reg t2;     // 10
reg t3;     // 11
reg t4;     // 12
reg t5;     // 13
reg t6;     // 14
reg t7;     // 15
reg s0;     // 16
reg s1;     // 17
reg s2;     // 18
reg s3;     // 19
reg s4;     // 20
reg s5;     // 21
reg s6;     // 22
reg s7;     // 23
reg t8;     // 24
reg t9;     // 25
reg k0;     // 26
reg k1;     // 27
reg gp;     // 28
reg sp;     // 29
reg fp;     // 30
reg ra;     // 31

reg MAR;    // memory address register
reg IR;     // instruction register
reg MDR;    // memory data register

word reg_write_data; // conteúdo a ser escrito em um registrador
reg* write_reg; // ponteiro para o registrador que receberá write data

reg PC;             // program counter
word pc_write_data; // saida do mux_pc

word ir_data; // conteúdo que chega no IR

// +----------------------------------+
// | FUNÇÕES - BANCO DE REGISTRADORES |
// +----------------------------------+

/*
 * get_register
 * ----------------------------
 *   Retorna um ponteiro para um registrador do
 *   banco de registradores com base em seu número
 *
 *   int id: valor identificador do registrador
 *
 */
reg* get_register(int id) {
	switch (id) {
		case 0:
			return &zero;
			break;
		case 1:
			return &at;
			break;
		case 2:
			return &v0;
			break;
		case 3:
			return &v1;
			break;
		case 4:
			return &a0;
			break;
		case 5:
			return &a1;
			break;
		case 6:
			return &a2;
			break;
		case 7:
			return &a3;
			break;
		case 8:
			return &t0;
			break;
		case 9:
			return &t1;
			break;
		case 10:
			return &t2;
			break;
		case 11:
			return &t3;
			break;
		case 12:
			return &t4;
			break;
		case 13:
			return &t5;
			break;
		case 14:
			return &t6;
			break;
		case 15:
			return &t7;
			break;
		case 16:
			return &s0;
			break;
		case 17:
			return &s1;
			break;
		case 18:
			return &s2;
			break;
		case 19:
			return &s3;
			break;
		case 20:
			return &s4;
			break;
		case 21:
			return &s5;
			break;
		case 22:
			return &s6;
			break;
		case 23:
			return &s7;
			break;
		case 24:
			return &t8;
			break;
		case 25:
			return &t9;
			break;
		case 26:
			return &k0;
			break;
		case 27:
			return &k1;
			break;
		case 28:
			return &gp;
			break;
		case 29:
			return &sp;
			break;
		case 30:
			return &fp;
			break;
		case 31:
			return &ra;
			break;
	}
	// registrador inválido
    return NULL;
}

/*
 * register_name
 * ----------------------------
 *   Retorna o nome (string) do registrador
 *   com base em seu número
 *
 *   int id: valor identificador do registrador
 *
 */
char* register_name(int id) {
	switch (id) {
		case 0:
			return "$zero";
			break;
		case 1:
			return "$at";
			break;
		case 2:
			return "$v0";
			break;
		case 3:
			return "$v1";
			break;
		case 4:
			return "$a0";
			break;
		case 5:
			return "$a1";
			break;
		case 6:
			return "$a2";
			break;
		case 7:
			return "$a3";
			break;
		case 8:
			return "$t0";
			break;
		case 9:
			return "$t1";
			break;
		case 10:
			return "$t2";
			break;
		case 11:
			return "$t3";
			break;
		case 12:
			return "$t4";
			break;
		case 13:
			return "$t5";
			break;
		case 14:
			return "$t6";
			break;
		case 15:
			return "$t7";
			break;
		case 16:
			return "$s0";
			break;
		case 17:
			return "$s1";
			break;
		case 18:
			return "$s2";
			break;
		case 19:
			return "$s3";
			break;
		case 20:
			return "$s4";
			break;
		case 21:
			return "$s5";
			break;
		case 22:
			return "$s6";
			break;
		case 23:
			return "$s7";
			break;
		case 24:
			return "$t8";
			break;
		case 25:
			return "$t9";
			break;
		case 26:
			return "$k0";
			break;
		case 27:
			return "$k1";
			break;
		case 28:
			return "$gp";
			break;
		case 29:
			return "$sp";
			break;
		case 30:
			return "$fp";
			break;
		case 31:
			return "$ra";
			break;
	}
	printf("ERRO: Registrador de número %d não encontrado.\n", id);
	exit(0);
}

/*******************************************************/

// +--------------------+
// |        ULA         |
// +--------------------+
reg A; // read data 1
reg B; // read data 2
bit ALUInput[3]; // bits que definem quais operações a ULA deverá executar
word ALUResult; // saída da ULA
reg ALUOut; // registrador que armazena a saída da ULA
word immediate_extended; // saída do sign extend
word operator_1; // primeiro operando
word operator_2; // segundo operando
bit ALU_zero; // bit que indica se resultado é 0 ou não

/*******************************************************/

// +--------------------+
// | SINAIS DE CONTROLE |
// +--------------------+
bit RegDst0;       // 00
bit RegDst1;       // 01
bit RegWrite;      // 02
bit ALUSrcA;       // 03
bit ALUSrcB0;      // 04
bit ALUSrcB1;      // 05
bit ALUOp0;        // 06
bit ALUOp1;        // 07
bit PCSource0;     // 08
bit PCSource1;     // 09
bit PCWriteCond;   // 10
bit PCWrite;       // 11
bit IorD;          // 12
bit MemRead;       // 13
bit MemWrite;      // 14
bit BNE;           // 15
bit IRWrite;       // 16
bit MemtoReg0;     // 17
bit MemtoReg1;     // 18

// auxiliar para BNE:
// sinal de controle que chega ao PC
bit PCControl;

// +--------------------+
// | SINAIS DE ESTADO   |
// +--------------------+
bit state[5];
bit next_state[5];

/*******************************************************/

// +---------------------+
// | UNIDADES FUNCIONAIS |
// +---------------------+


/*
 * FUNÇÃO QUE SIMULA O MUX QUE FARÁ O ACESSO A MEMÓRIA
 * ----------------------------
 *   O que ela faz:
 *          SINAL DE CONTROLE: IorD
 *          RESULTADO: ALTERA O VALOR DE MAR
*/
void MUX_MEMORY() {
	switch (IorD) {
        case 0:
			MAR = PC;
			break;
		case 1:
			MAR = ALUOut;
			break;
	}
}


/*
 * FUNÇÃO QUE SIMULA A ESCRITA EM PC, SE NECESSÁRIO
 * ----------------------------
 *   O que ela faz:
 *          SINAL DE CONTROLE: PCControl
 *          RESULTADO: ALTERA O VALOR DE PC
 *
 */
void PROGRAM_COUNTER() {
	if (PCControl) {
		PC = pc_write_data;
	}
}


/*
 * FUNCAO QUE SIMULA A LEITURA E ESCRITA NA MEMÓRIA
 * ----------------------------
 *   O que ela faz:
 *          SINAL DE CONTROLE: MemRead, MemWrite
 *          RESULTADO: LÊ DA MEMÓRIA OU ESCREVE NA MEMÓRIA
 */
void MEMORY_BANK() {
	if (MemRead) {
        memory_word_pointer = (word*)(&(MEMORY[MAR]));
		// MDR = MEMORY[MAR];
        MDR = (*memory_word_pointer);
		// Fio que vai p/ IR = MEMORY[MAR];
        ir_data = (*memory_word_pointer);
	}

	if (MemWrite) {
		// B = registrador read data 2
        memory_word_pointer = (word*)(&(MEMORY[MAR]));
		// MEMORY[MAR] = B;
        (*memory_word_pointer) = B;
	}
}


/*
 * FUNÇÃO QUE SIMULA O MUX QUE SELECIONA EM QUAL REGISTRADOR
 * OCORRERÁ A ESCRITA NO BANCO DE REGISTRADORES
 * ----------------------------
 *   O que ela faz:
 *          SINAL DE CONTROLE: RegDst (2 bits)
 * 				0 - PEGA O VALOR DE INSTRUCTION[20 .. 16] (RT)
 * 				1 - PEGA O VALOR DE INSTRUCTION[15 .. 11] (RD)
 * 				2 - PEGA O VALOR 31 (ID DO REGISTRADOR $ra)
 *          RESULTADO: O REGISTRADOR PARA ESCRITA É SELECIONADO
 */
void MUX_WRITE_REG() {
	int i;
  switch (RegDst1) {
	  case 0:
			switch (RegDst0) {
				case 0:
					// escrever no registrador apontado por rt
					write_reg = get_register(bin2dec(rt, 5));
					break;
				case 1:
					// escrever no registrador apontado por rd
					write_reg = get_register(bin2dec(rd, 5));
					break;
			}
		  break;
	  case 1:
		  // registrador 31 = 11111 = $ra
		  write_reg = get_register(31);
		  break;
  }
}


/**
 * FUNÇÃO QUE SIMULA O MUX RESPONSÁVEL PELA SELEÇÃO DO VALOR A SER ESCRITO NO BANCO DE REGISTRADORES
 * ----------------------------
 *   O que ela faz:
 * 			SINAL DE CONTROLE: MemtoReg (2 bits)
 * 				0 - PEGA O VALOR DE ALUOut
 * 				1 - PEGA O VALOR DE MDR
 * 				2 - PEGA O VALOR DE PC
 * 			RESULTADO: O DADO A SER ESCRITO ESTÁ SELECIONADO
 */
void MUX_WRITE_DATA() {
    int i;
    switch (MemtoReg1) {
        case 0:
            switch (MemtoReg0) {
                case 0:
                    // escreve de ALUOut em Banco de Registradores
                    reg_write_data = ALUOut;
                    break;
                case 1:
                    // escreve de MDR em banco de registradores[write_register]
                    reg_write_data = MDR;
                    break;
            }
            break;
        case 1:
            // escreve de PC em banco de registradores[write_register]
            switch (MemtoReg0) {
                case 0:
                    reg_write_data = PC;
                    break;
            }
            break;
    }
}


/**
 * FUNÇÃO QUE SIMULA O MUX QUE SELECIONA O PRIMEIRO OPERANDO DA ULA
 * ----------------------------
 *   O que ela faz:
 * 			SINAL DE CONTROLE: ALUSrcA
 * 				0 - PEGA O VALOR DE PC
 * 				1 - PEGA O VALOR DO REGISTRADOR A
 * 			RESULTADO: O PRIMEIRO OPERANDO DA ULA ESTÁ SELECIONADO
 */
void MUX_ALU_1() {
	switch (ALUSrcA) {
		case 0:
			// PRIMEIRO OPERANDO DA ULA RECEBE PC
			operator_1 = PC;
			break;
		case 1:
			//PRIMEIRO OPERANDO DA ULA RECEBE A
			operator_1 = A;
		  	break;
	}
}


/**
 * FUNÇÃO QUE SIMULA O MUX QUE SELECIONA O SEGUNDO OPERANDO DA ULA
 * ----------------------------
 *   O que ela faz:
 * 			SINAL DE CONTROLE: ALUSrcB (2 bits)
 * 				0 - PEGA O VALOR DO REGISTRADOR B
 * 				1 - PEGA O VALOR 4 (USADO PARA INCREMENTAR O PC PARA A PRÓXIMA POSIÇÃO DA MEMÓRIA)
 *				2 - PEGA O VALOR IMEDIATO DE INSTRUCTION[15 .. 0] (DEPOIS DA EXTENSÃO DE SINAL)
 * 				3 - PEGA O VALOR IMEDIATO DE INSTRUCTION[15 .. 0] (DEPOIS DA EXTENSÃO DE SINAL E DO SHIFT LEFT 2)
 * 			RESULTADO: O SEGUNDO OPERANDO DA ULA ESTÁ SELECIONADO
 */
void MUX_ALU_2() {
  switch (ALUSrcB1) {
	  case 0:
			switch (ALUSrcB0) {
				case 0:
					//SEGUNDO OPERANDO DA ULA RECEBE B
					operator_2 = B;
					break;
				case 1:
					//SEGUNDO OPERANDO DA ULA RECEBE 4
					operator_2 = 4;
					break;
			}
			break;
	  case 1:
			switch (ALUSrcB0) {
				case 0:
					//SEGUNDO OPERANDO DA ULA RECEBE IMEDIATO EXTENDIDO
					operator_2 = immediate_extended;
					break;
				case 1:
					//SEGUNDO OPERANDO DA ULA RECEBE IMEDIATO EXTENDIDO << 2
					operator_2 = immediate_extended << 2;
					break;
			}
		  break;
    }
}


/**
 * FUNÇÃO QUE SIMULA O MUX QUE SELECIONA O QUE SERÁ ESCRITO EM PC
 * ----------------------------
 *   O que ela faz:
 * 			SINAL DE CONTROLE: PCSource
 * 				0 - PEGA O RESULTADO DA ULA
 * 				1 - PEGA O VALOR DE ALUOUT
 * 				2 - PEGA OS 4 PRIMEIROS BITS DE PC CONCATENADOS
 *					COM O VALOR IMEDIATO jump_addr SHIFTADO 2 À ESQUERDA
 * 				3 - PEGA O VALOR DO REGISTRADOR A
 * 			RESULTADO: O VALOR A SER ESCRITO EM PC ESTÁ SELECIONADO
 */
void MUX_PC() {
	int i;
	switch (PCSource1) {
		case 0:
			switch (PCSource0) {
				case 0:
					// PC RECEBE ALU RESULT (SAIDA DA ULA)
					pc_write_data = ALUResult;
					break;
				case 1:
					// PC RECEBE ALUOUT
					pc_write_data = ALUOut;
					break;
			}
			break;
		case 1:
			switch (PCSource0) {
				case 0:
					// PC RECEBE PC[31..28] + (jump_addr[26..0] << 2)
					pc_write_data = 0;
					for(i = 0; i < 4; i++)
						pc_write_data += GETBIT(PC, 31-i) * ((unsigned int)pow(2, 31-i));
	                    pc_write_data += (bin2dec(jump_addr, 26) << 2);
					break;
				case 1:
					// PC RECEBE A
					pc_write_data = A;
					break;
			}
			break;
	  }
}


/*
 * FUNÇÃO QUE SIMULA O MUX ADICIONADO PARA O BNE
 * ----------------------------
 *   O que ela faz:
 *          SINAL DE CONTROLE: BNE
 * 				0 - A ESCRITA EM PC SERÁ HABILITADA CASO O RESULTADO DA ULA SEJA 0
 *					E PCWriteCond ESTIVER HABILITADO OU SE PCWrite ESTIVER HABILITADO
 * 				1 - A ESCRITA EM PC SERÁ HABILITADA CASO O RESULTADO DA ULA NÃO SEJA 0
 *					E PCWriteCond ESTIVER HABILITADO OU SE PCWrite ESTIVER HABILITADO
 * 			RESULTADO: A ESCRITA EM PC SERÁ HABILITADO SE A CONDIÇÃO SELECIONADA FOR SATISFEITA
 *
 */
void MUX_BNE() {
	switch (BNE) {
	  case 0:
			PCControl = ((ALU_zero & PCWriteCond) | PCWrite);
			break;
		case 1:
			PCControl = (((!ALU_zero) & PCWriteCond) | PCWrite);
			break;
	}
}


/*
 * FUNÇÃO AUXILIAR PARA OBTER OS VALORES DE IR
 * ----------------------------
 *   O que ela faz:
 *      	SEPARA OS BITS DO VALOR DE IR EM VÁRIOS FIOS
 */
void IR_SET() {
	int i;

	for (i = 0; i < 6; i++) {
		op_code[i] = GETBIT(IR, 26+i);
		function[i] = GETBIT(IR, i);
	}

	for (i = 0; i < 5; i++) {
		rs[i] = GETBIT(IR, 21+i);
		rt[i] = GETBIT(IR, 16+i);
		rd[i] = GETBIT(IR, 11+i);
	}

	for (i = 0; i < 16; i++) {
		immediate[i] = GETBIT(IR, i);
	}

	for (i = 0; i < 26; i++) {
		jump_addr[i] = GETBIT(IR, i);
	}
}


/*
 * FUNÇÃO QUE SIMULA O IR
 * ----------------------------
 *   O que ela faz:
 *      Quando IRWrite está habilitado,
 *      o conteúdo que saiu de MEMORY_BANK é
 *      escrito no IR e então separado nos valores
 *      corretos por IR_SET.
 *
 */
void IR_UNIT() {
    if (IRWrite) {
        IR = ir_data;
        IR_SET();
    }

}


/*
 * FUNÇÃO QUE REPRESENTA A LEITURA E ESCRITA DO BANCO DE REGISTRADORES
 * ----------------------------
 *   O que ela faz:
 *			SINAL DE CONTROLE: RegWrite
 *			RESULTADO: O REGISTRADOR A RECEBE O VALOR DO REGISTRADOR APONTADO POR RS
 *					   O REGISTRADOR B RECEBE O VALOR DO REGISTRADOR APONTADO POR RT
 					   SE RegWrite ESTIVER HABILITADO, OCORRERÁ A ESCRITA NO BANCO DE REGISTRADORES
 *
 */
void REGISTER_BANK() {
    A = *(get_register(bin2dec(rs, 5)));
    B = *(get_register(bin2dec(rt, 5)));

	if (RegWrite) {
        // registrador recebe conteúdo
		(*write_reg) = reg_write_data;
	}
}


/*
 * EXTENSÃO DE SINAL DE 16 BITS PARA 32 BITS
 * ----------------------------
 *   O que ela faz:
 *          RECEBE O VALOR IMEDIATO DE INSTRUCTION [15 .. 0] COM 16 BITS E MANDA PARA O MUX
 *          DA ENTRADA B DA ULA O MESMO VALOR APÓS A EXTENSÃO DE SINAL, COM 32 BITS
 *
 */
void SIGNAL_EXTEND_16_TO_32() {
	immediate_extended = bin2dec(immediate, 16);
}


/*
 *  FUNÇÃO QUE SIMULA O CONTROLE DA ULA FEITO PELA UNIDADE ALU CONTROL
 * ----------------------------
 *   O que ela faz:
 *          SINAL DE CONTROLE: ALUOp, INSTRUCTION[5 ..0]
 *          RESULTADO: ENVIA PARA A ULA A INTRUÇÃO DE QUAL OPERAÇÃO DEVE SER REALIZADA
 *
 */
void ALU_CONTROL() {
    //ANALISE FEITA EM SWITCH POIS POSSUI DOIS BITS DE CONTROLE
    switch(ALUOp1) {
		// não precisa checar o campo de função (instruções LW, SW, Branch)
		case 0:
			switch(ALUOp0) {
				case 0:	// add
					ALUInput[2] = 0;
					ALUInput[1] = 1;
					ALUInput[0] = 0;
					break;
				case 1:	// subtract
					ALUInput[2] = 1;
					ALUInput[1] = 1;
					ALUInput[0] = 0;
					break;
			}
			break;
		// precisa checar o campo de função e ALUOp0 sempre será 0
		case 1:
			// (ALUOp = 10 e Function = 100000) operação = add
			if ((function[5] == 1) && ((function[4] == 0) && ((function[3] == 0) && ((function[2] == 0) && ((function[1] == 0) && ((function[0] == 0))))))) {
				ALUInput[2] = 0;
				ALUInput[1] = 1;
				ALUInput[0] = 0;
			}
			// (ALUOp = 10 e Function = 100010) operação = subtract
			else if ((function[5] == 1) && ((function[4] == 0) && ((function[3] == 0) && ((function[2] == 0) && ((function[1] == 1) && ((function[0] == 0))))))) {
				ALUInput[2] = 1;
				ALUInput[1] = 1;
				ALUInput[0] = 0;
			}
			// (ALUOp = 10 e Function = 100100) operação = and
			else if ((function[5] == 1) && ((function[4] == 0) && ((function[3] == 0) && ((function[2] == 1) && ((function[1] == 0) && ((function[0] == 0))))))) {
				ALUInput[2] = 0;
				ALUInput[1] = 0;
				ALUInput[0] = 0;
			}
			// (ALUOp = 10 e Function = 100101) operação = or
			else if ((function[5] == 1) && ((function[4] == 0) && ((function[3] == 0) && ((function[2] == 1) && ((function[1] == 0) && ((function[0] == 1))))))) {
				ALUInput[2] = 0;
				ALUInput[1] = 0;
				ALUInput[0] = 1;
			}
			// (ALUOp = 10 e Function = 101010) operação = set on less than
			else if ((function[5] == 1) && ((function[4] == 0) && ((function[3] == 1) && ((function[2] == 0) && ((function[1] == 1) && ((function[0] == 0))))))) {
				ALUInput[2] = 1;
				ALUInput[1] = 1;
				ALUInput[0] = 1;
			}
			break;
	}
}


/*
 * FUNÇÃO QUE SIMULA UMA ULA QUE REALIZA OPERAÇÕES COM DOIS OPERANDOS
 * ----------------------------
 *   O que ela faz:
 *          SINAL DE CONTROLE: ALUInput
 *          RESULTADO: O RESULTADO DAS OPERAÇÕES FICA DISPONÍVEL NO FIO ALUResult
 *
 */
void ALU() {
	// (ALUInput = 010) operação = add
	if ((ALUInput[2] == 0) && (ALUInput[1] == 1) && (ALUInput[0] == 0)) {
		ALUResult = operator_1 + operator_2;
	}
	// (ALUInput = 110) operação = subtract
	else if ((ALUInput[2] == 1) && (ALUInput[1] == 1) && (ALUInput[0] == 0)) {
		ALUResult = operator_1 - operator_2;
	}
	// (ALUInput = 000) operação = and
	else if ((ALUInput[2] == 0) && (ALUInput[1] == 0) && (ALUInput[0] == 0)) {
		ALUResult = operator_1 & operator_2;
	}
	// (ALUInput = 001) operação = or
	else if ((ALUInput[2] == 0) && (ALUInput[1] == 1) && (ALUInput[0] == 1)) {
		ALUResult = operator_1 | operator_2;
	}
	// (ALUInput = 111) operação = set on less than
	else if ((ALUInput[2] == 1) && (ALUInput[1] == 1) && (ALUInput[0] == 1)) {
		(operator_1 < operator_2) ? (ALUResult = 1) : (ALUResult = 0);
	}

    // setar bit ALU_ZERO
    if (ALUResult == 0) {
        ALU_zero = 1;
    } else {
        ALU_zero = 0;
    }

}


/*
 * FUNÇÃO QUE ESCEVE A SAÍDA DA ULA NO REGISTRADOR ALUOut
 * ----------------------------
 *   O que ela faz:
 * 			ESCREVE O VALOR PRESENTE NO FIO DE SAÍDA DA ULA NO
 *			REGISTRADOR ALUOut
 *
 */
void ALU_OUT() {
	ALUOut = ALUResult;
}


/*
 * FUNCAO RESPONSAVEL POR SETAR OS BITS DE CONTROLE
 * FUNCAO RESPOSAVEL POR CALCULAR O PROXIMO ESTADO
 * ----------------------------
 *   O que ela faz:
 *         ANALISA COM BASE EM UMA PLA, QUANDO OS BITS SERAO SETADOS DEPENDENDO DE
 *         CADA ESTADO.
 *
 *         UTILIZA SOMENTE AS INFORMACOES DO ESTADOS
 *         PARA CALCULAR PROXIMO ESTADO UTILIZA A INFORMACAO DO ESTADO ANTERIOR E
 *         OS BITS DE OPERACAO (OPCODE)
 *
 */
 void CONTROL() {
	// ANALISANDO OS SINAIS DE CONTROLE COM BASE NOS VALORES DOS ESTADOS

    // ****** SINAIS DE CONTROLE ******
    //REGDST POSSUI DOIS BITS

 	//RegDst0 = (S0 * S1 * S2 * !S3 * !S4)
	RegDst0 = (state[0] & state[1] & state[2] & !state[3] & !state[4]);

	//RegDst1 = (S0 * S1 * !S2 * S3 * !S4) + (S0 * !S1 * !S2 * !S3 * S4)
	RegDst1 = (state[0] & state[1] & !state[2] & state[3] & !state[4]) |
              (state[0] & !state[1] & !state[2] & !state[3] & state[4]);

    /*RegWrite = (!S0 * !S1 * S2 * !S3 * !S4) + (S0 * S1 * S2 * !S3 * !S4) +
     		     (S0 * S1 * !S2 * S3 * !S4) + (S0 * !S1 * S2 * S3 * !S4) +
     		     (!S0 * !S1 * !S2 * !S3 * S4) + (S0 * !S1 * !S2 * !S3 * S4)
    */
	RegWrite = (!state[0] & !state[1] & state[2] & !state[3] & !state[4]) |
               (state[0] & state[1] & state[2] & !state[3] & !state[4]) |
			   (state[0] & state[1] & !state[2] & state[3] & !state[4]) |
               (state[0] & !state[1] & state[2] & state[3] & !state[4]) |
			   (!state[0] & !state[1] & !state[2] & !state[3] & state[4]) |
               (state[0] & !state[1] & !state[2] & !state[3] & state[4]);

    /*ALUSrcA = (!S0 * S1 * !S2 * !S3 * !S4) + (!S0 * S1 * S2 * !S3 * !S4) +
     		    (!S0 * !S1 * !S2 * S3 * !S4) + (!S0 * !S1 * S2 * S3 * !S4) +
     		    (!S0 * S1 * S2 * S3 * !S4) + (S0 * S1 * S2 * S3 * !S4)
    */
	ALUSrcA = (!state[0] & state[1] & !state[2] & !state[3] & !state[4]) |
              (!state[0] & state[1] & state[2] & !state[3] & !state[4]) |
		      (!state[0] & !state[1] & !state[2] & state[3] & !state[4]) |
              (!state[0] & !state[1] & state[2] & state[3] & !state[4]) |
			  (!state[0] & state[1] & state[2] & state[3] & !state[4]) |
              (state[0] & state[1] & state[2] & state[3] & !state[4]);

    // ALUSRCB POSSUI DOIS BITS
    //ALUSrcB0 = (!S0 * !S1 * !S2 * !S3 * !S4) + (S0 * !S1 * !S2 * !S3 * !S4)
	ALUSrcB0 = (!state[0] & !state[1] & !state[2] & !state[3] & !state[4]) |
                (state[0] & !state[1] & !state[2] & !state[3] & !state[4]);

    /*ALUSrcB1 = (S0 * !S1 * !S2 * !S3 * !S4) + (!S0 * S1 * !S2 * !S3 * !S4) +
     		     (!S0 * !S1 * S2 * S3 * !S4) + (S0 * S1 * S2 * S3 * !S4)
    */
	ALUSrcB1 = (state[0] & !state[1] & !state[2] & !state[3] & !state[4]) |
                (!state[0] & state[1] & !state[2] & !state[3] & !state[4]) |
				(!state[0] & !state[1] & state[2] & state[3] & !state[4]) |
                (state[0] & state[1] & state[2] & state[3] & !state[4]);

    // ALUOP POSSUI DOIS BITS
    /*ALUOp0 = (!S0 * !S1 * !S2 * S3 * !S4) + (!S0 * S1 * S2 * S3 * !S4) +
     		   (S0 * S1 * S2 * S3 * !S4)
    */
	ALUOp0 = (!state[0] & !state[1] & !state[2] & state[3] & !state[4]) |
             (!state[0] & state[1] & state[2] & state[3] & !state[4]) |
             (state[0] & state[1] & state[2] & state[3] & !state[4]);

    //ALUOp1 = (!S0 * S1 * S2 * !S3 * !S4) + (S0 * S1 * S2 * S3 * !S4)
	ALUOp1 = (!state[0] & state[1] & state[2] & !state[3] & !state[4]) |
             (state[0] & state[1] & state[2] & state[3] & !state[4]);

    //PCSOURCE POSSUI 2 BITS
	/*PCSource0 = (!S0 * !S1 * !S2 * S3 * !S4) + (!S0 * S1 * !S2 * S3 * !S4) +
     		      (S0 * S1 * !S2 * S3 * !S4) + (!S0 * S1 * S2 * S3 * !S4)
    */
	PCSource0 = (!state[0] & !state[1] & !state[2] & state[3] & !state[4]) |
                (!state[0] & state[1] & !state[2] & state[3] & !state[4]) |
		        (state[0] & state[1] & !state[2] & state[3] & !state[4]) |
                (!state[0] & state[1] & state[2] & state[3] & !state[4]);
    /*PCSource1 = (S0 * !S1 * !S2 * S3 * !S4) + (!S0 * S1 * !S2 * S3 * !S4) +
     		      (S0 * S1 * !S2 * S3 * !S4) + (S0 * !S1 * !S2 * !S3 * !S4)
    */
	PCSource1 = (state[0] & !state[1] & !state[2] & state[3] & !state[4]) |
                (!state[0] & state[1] & !state[2] & state[3] & !state[4]) |
				(state[0] & state[1] & !state[2] & state[3] & !state[4]) |
                (state[0] & !state[1] & !state[2] & !state[3] & state[4]);

    //PCWriteCond = (!S0 * !S1 * !S2 * S3 * !S4) + (!S0 * S1 * S2 * S3 * !S4)
	PCWriteCond = (!state[0] & !state[1] & !state[2] & state[3] & !state[4]) |
                  (!state[0] & state[1] & state[2] & state[3] & !state[4]);

	/*PCWrite = (!S0 * S1 * !S2 * !S3 * !S4) + (!S0 * S1 * S2 * !S3 * !S4) +
     		     (!S0 * !S1 * !S2 * S3 * !S4) + (!S0 * !S1 * S2 * S3 * !S4) +
     		     (!S0 * S1 * S2 * S3 * !S4) + (S0 * S1 * S2 * S3 * !S4)
    */
	PCWrite = (!state[0] & !state[1] & !state[2] & !state[3] & !state[4]) |
              (state[0] & !state[1] & !state[2] & state[3] & !state[4]) |
			  (!state[0] & state[1] & !state[2] & state[3] & !state[4]) |
              (state[0] & state[1] & !state[2] & state[3] & !state[4]) |
			  (state[0] & !state[1] & !state[2] & !state[3] & state[4]);

    //IorD = (S0 * S1 * !S2 * !S3 * !S4) + (S0 * !S1 * S2 * !S3 * !S4)
	IorD = (state[0] & state[1] & !state[2] & !state[3] & !state[4]) |
           (state[0] & !state[1] & state[2] & !state[3] & !state[4]);

    //MemRead = (!S0 * !S1 * !S2 * !S3 * !S4) + (S0 * S1 * !S2 * !S3 * !S4)
	MemRead = (!state[0] & !state[1] & !state[2] & !state[3] & !state[4]) |
              (state[0] & state[1] & !state[2] & !state[3] & !state[4]);

    //MemWrite = (S0 * !S1 * S2 * !S3 * !S4)
	MemWrite = (state[0] & !state[1] & state[2] & !state[3] & !state[4]);

	//BNE = (!S0 * S1 * S2 * S3 * !S4)
	BNE = (!state[0] & state[1] & state[2] & state[3] & !state[4]);

    //IRWrite = (!S0 * !S1 * !S2 * !S3 * !S4)
	IRWrite = (!state[0] & !state[1] & !state[2] & !state[3] & !state[4]);

    // MEMTOREG POSSUI 2 BITS
    //MemtoReg0 = (!S0 * S1 * !S2 * S3 * !S4)
	MemtoReg0 = (!state[0] & !state[1] & state[2] & !state[3] & !state[4]);

	//MemtoReg1 = (S0 * S1 * !S2 * S3 * !S4) + (S0 * !S1 * S2 * !S3 * S4)
	MemtoReg1 = (state[0] & state[1] & !state[2] & state[3] & !state[4]) |
                 (state[0] & !state[1] & state[2] & !state[3] & state[4]);


    // ***** Equacoes de proximo estado: ******
    // ESTADO ANTERIOR + OP_CODE (Caso tenha mais de um caminho)
    // ATIVADO QUANDO: Numero do estado em binario possuir o bit em analise como 1

    // ANALISE A PARTIR DO BIT MENOS SIGNIFICATIVO (NS0)
                        // ESTADO 1 = (!S0 * !S1 * !S2 * !S3 * !S4)
    next_state[0] =     (!state[0] & !state[1] & !state[2] & !state[3] & !state[4] ) |
                        // ESTADO 3 = (!S0 * S1 * !S2 * !S3 * !S4 * OP0 * OP1 * !OP2 * !OP3 * !OP4 * OP5)
                        (!state[0] & state[1] & !state[2] & !state[3] & !state[4] & op_code[0] & op_code[1] & !op_code[2] & !op_code[3] & !op_code[4] & op_code[5]) |
                        // ESTADO 5 = (!S0 * S1 * !S2 * !S3 * !S4 * OP0 * OP1 * !OP2 * OP3 * !OP4 * OP5)
                        (!state[0] & state[1] & !state[2] & !state[3] & !state[4] & op_code[0] & op_code[1] & !op_code[2] & op_code[3] & !op_code[4] & op_code[5]) |
                        // ESTADO 7 = (!S0 * S1 * S2 * !S3 * !S4)
                        (!state[0] & state[1] & state[2] & !state[3] & !state[4] ) |
                        // ESTADO 9 = (S0 * !S1 * !S2 * !S3 * !S4 * !OP0 * OP1 * !OP2 * !OP3 * !OP4 * !OP5)
                        (state[0] & !state[1] & !state[2] & !state[3] & !state[4] & !op_code[0] & op_code[1] & !op_code[2] & !op_code[3] & !op_code[4] & !op_code[5]) |
                        // ESTADO 11 = (S0 * !S1 * !S2 * !S3 * !S4 * OP0 * !OP1 * OP2 * !OP3 * OP4 * !OP5)
                        (state[0] & !state[1] & !state[2] & !state[3] & !state[4] & op_code[0] & !op_code[1] & op_code[2] & !op_code[3] & op_code[4] & !op_code[5]) |
                        // ESTADO 13 = (!S0 * !S1 * S2 * S3 * !S4)
                        (!state[0] & !state[1] & state[2] & state[3] & !state[4] ) |
                        // ESTADO 15 = (S0 * !S1 * !S2 * !S3 * !S4 * !OP0 * !OP1 * OP2 * OP3 * !OP4 * !OP5)
                        (state[0] & !state[1] & !state[2] & !state[3] & !state[4] & !op_code[0] & !op_code[1] & op_code[2] & op_code[3] & !op_code[4] & !op_code[5]) |
                        // ESTADO 17 = (S0 * !S1 * !S2 * !S3 * !S4 * OP0 * OP1 * !OP2 * !OP3 * !OP4 * !OP5)
                        (state[0] & !state[1] & !state[2] & !state[3] & !state[4] & op_code[0] & op_code[1] & !op_code[2] & !op_code[3] & !op_code[4] & !op_code[5]);

					 //ESTADO X = (S0 * !S1 * !S2 * !S3 * !S4 * OP0 * OP1 * !OP2 * !OP4 * OP5)
	next_state[1] =  (state[0] & !state[1] & !state[2] & !state[3] & !state[4] & op_code[0] & op_code[1] & !op_code[2] & !op_code[4] & op_code[5]) |
					 //ESTADO X = (!S0 * S1 * !S2 * !S3 * !S4 * OP0 * OP1 * !OP2 * !OP3 * !OP4 * OP5)
					 (!state[0] & state[1] & !state[2] & !state[3] & !state[4] & op_code[0] & op_code[1] & !op_code[2] & !op_code[3] & !op_code[4] & op_code[5]) |
					 //ESTADO X = (S0 * !S1 * !S2 * !S3 * !S4 * !OP0 * !OP1 * !OP2 * !OP3 * !OP4 * !OP5)
					 (state[0] & !state[1] & !state[2] & !state[3] & !state[4] & !op_code[0] & !op_code[1] & !op_code[2] & !op_code[3] & !op_code[4] & !op_code[5]) |
					 //ESTADO X = (!S0 * S1 * S2 * !S3 * !S4)
					 (!state[0] & state[1] & state[2] & !state[3] & !state[4]) |
					 //ESTADO X = (S0 * !S1 * !S2 * !S3 * !S4 * !OP0 * !OP1 * OP2 * !OP3 * OP4 * !OP5)
					 (state[0] & !state[1] & !state[2] & !state[3] & !state[4] & !op_code[0] & !op_code[1] & op_code[2] & !op_code[3] & op_code[4] & !op_code[5]) |
					 //ESTADO X = (S0 * !S1 * !S2 * !S3 * !S4 * OP0 * !OP1 * OP2 * !OP3 * OP4 * !OP5)
					 (state[0] & !state[1] & !state[2] & !state[3] & !state[4] & op_code[0] & !op_code[1] & op_code[2] & !op_code[3] & op_code[4] & !op_code[5]) |
					 //ESTADO X = (S0 * !S1 * !S2 * !S3 * !S4 * OP0 * !OP1 * OP2 * !OP3 * !OP4 * !OP5)
					 (state[0] & !state[1] & !state[2] & !state[3] & !state[4] & op_code[0] & !op_code[1] & op_code[2] & !op_code[3] & !op_code[4] & !op_code[5]) |
					 //ESTADO X = (S0 * !S1 * !S2 * !S3 * !S4 * !OP0 * !OP1 * OP2 * OP3 * !OP4 * !OP5)
					 (state[0] & !state[1] & !state[2] & !state[3] & !state[4] & !op_code[0] & !op_code[1] & op_code[2] & op_code[3] & !op_code[4] & !op_code[5]);

					 //ESTADO X = (S0 * !S1 * !S2 * !S3 * !S4 * !OP0 * !OP1 * OP2 * !OP3 * !OP4 * !OP5)
	next_state[2] =  (state[0] & state[1] & !state[2] & !state[3] & !state[4]) |
					 //ESTADO X = (!S0 * S1 * !S2 * !S3 * !S4 * OP0 * OP1 * !OP2 * OP3 * !OP4 * OP5)
					 (!state[0] & state[1] & !state[2] & !state[3] & !state[4] & op_code[0] & op_code[1] & !op_code[2] & op_code[3] & !op_code[4] & op_code[5]) |
					 //ESTADO X = (S0 * !S1 * !S2 * !S3 * !S4 * !OP0 * !OP1 * !OP2 * !OP3 * !OP4 * !OP5)
					 (state[0] & !state[1] & !state[2] & !state[3] & !state[4] & !op_code[0] & !op_code[1] & !op_code[2] & !op_code[3] & !op_code[4] & !op_code[5]) |
					 //ESTADO X = (!S0 * S1 * S2 * !S3 * !S4)
					 (!state[0] & state[1] & state[2] & !state[3] & !state[4]) |
					 //ESTADO X = (S0 * !S1 * !S2 * !S3 * !S4 * OP0 * !OP1 * !OP2 * OP3 * !OP4 * !OP5)
					 (state[0] & !state[1] & !state[2] & !state[3] & !state[4] & !op_code[0] & !op_code[1] & !op_code[2] & op_code[3] & !op_code[4] & !op_code[5]) |
					 //ESTADO X = (!S0 * !S1 * S2 * S3 * !S4)
					 (!state[0] & !state[1] & state[2] & state[3] & !state[4]) |
					 //ESTADO X = (S0 * !S1 * !S2 * !S3 * !S4 * OP0 * !OP1 * OP2 * !OP3 * !OP4 * !OP5)
					 (state[0] & !state[1] & !state[2] & !state[3] & !state[4] & op_code[0] & !op_code[1] & op_code[2] & !op_code[3] & !op_code[4] & !op_code[5]) |
					 //ESTADO X = (S0 * !S1 * !S2 * !S3 * !S4 * !OP0 * !OP1 * OP2 * OP3 * !OP4 * !OP5)
					 (state[0] & !state[1] & !state[2] & !state[3] & !state[4] & !op_code[0] & !op_code[1] & op_code[2] & op_code[3] & !op_code[4] & !op_code[5]);

					 //ESTADO X = (S0 * !S1 * !S2 * !S3 * !S4 * !OP0 * !OP1 * OP2 * !OP3 * !OP4 * !OP5)
	next_state[3] =  (state[0] & !state[1] & !state[2] & !state[3] & !state[4] & !op_code[0] & !op_code[1] & op_code[2] & !op_code[3] & !op_code[4] & !op_code[5]) |
					 //ESTADO X = (S0 * !S1 * !S2 * !S3 * !S4 * !OP0 * OP1 * !OP2 * !OP3 * !OP4 * !OP5)
					 (state[0] & !state[1] & !state[2] & !state[3] & !state[4] & !op_code[0] & op_code[1] & !op_code[2] & !op_code[3] & !op_code[4] & !op_code[5]) |
					 //ESTADO X = (S0 * !S1 * !S2 * !S3 * !S4 * !OP0 * !OP1 * OP2 * !OP3 * OP4 * !OP5)
					 (state[0] & !state[1] & !state[2] & !state[3] & !state[4] & !op_code[0] & !op_code[1] & op_code[2] & !op_code[3] & op_code[4] & !op_code[5]) |
					 //ESTADO X = (S0 * !S1 * !S2 * !S3 * !S4 * OP0 * !OP1 * OP2 * !OP3 * OP4 * !OP5)
					 (state[0] & !state[1] & !state[2] & !state[3] & !state[4] & op_code[0] & !op_code[1] & op_code[2] & !op_code[3] & op_code[4] & !op_code[5]) |
					 //ESTADO X = (S0 * !S1 * !S2 * !S3 * !S4 * !OP0 * !OP1 * !OP2 * OP3 * !OP4 * !OP5)
					 (state[0] & !state[1] & !state[2] & !state[3] & !state[4] & !op_code[0] & !op_code[1] & !op_code[2] & op_code[3] & !op_code[4] & !op_code[5]) |
					 //ESTADO X = (!S0 * !S1 * S2 * S3 * !S4)
					 (!state[0] & !state[1] & state[2] & state[3] & !state[4]) |
					 //ESTADO X = (!S0 * !S1 * S2 * S3 * !S4)
					 (state[0] & !state[1] & !state[2] & !state[3] & !state[4] & op_code[0] & !op_code[1] & op_code[2] & !op_code[3] & !op_code[4] & !op_code[5]) |
					 //ESTADO X = (S0 * !S1 * !S2 * !S3 * !S4 * !OP0 * !OP1 * OP2 * OP3 * !OP4 * !OP5)
					 (state[0] & !state[1] & !state[2] & !state[3] & !state[4] & !op_code[0] & !op_code[1] & op_code[2] & op_code[3] & !op_code[4] & !op_code[5]);

					 //ESTADO X = (S0 * !S1 * !S2 * !S3 * !S4 * OP0 * OP1 * !OP2 * !OP3 * !OP4 * !OP5)
	next_state[4] =  (state[0] & !state[1] & !state[2] & !state[3] & !state[4] & op_code[0] & op_code[1] & !op_code[2] & !op_code[3] & !op_code[4] & !op_code[5]) |
					 //ESTADO X = (S0 * S1 * S2 * S3 * !S4)
					 (state[0] & state[1] & state[2] & state[3] & !state[4]);

 }


void CONTROL_NEXT() {
    // atualizando estado para o próximo ciclo
    state[0] = next_state[0];
    state[1] = next_state[1];
    state[2] = next_state[2];
    state[3] = next_state[3];
    state[4] = next_state[4];

}

/*******************************************************/

// +-----------+
// | SIMULAÇÃO |
// +-----------+

/*
 *  INICIALIZAR TODOS OS COMPONENTES DO PROJETO
 * ----------------------------
 *   O que ela faz:
 *         INICIALIZA OS COMPONENTES PARA O VALORES CORRETOS
 *
 */
void initialize(const char* source) {
	int i;
	// instrução a ser lida do arquivo
    unsigned int instruction;
	// conta quantas instruções foram lidas para indexar memória
	int instr_counter;
	reg* current_reg = NULL;

	// abrir arquivo do código fonte
	FILE* bin = NULL;
	bin = fopen(source, "r");

	// checar integridade do código fonte
	if (bin == NULL) {
		printf("ERRO: Código fonte não carregado.\n");
		exit(0);
	}

	// inicializar memória
    // acessar posição inicial (byte)
    memory_pointer = (byte*)(&(MEMORY));
    // escrever 0 em todos os bytes
	for (i = 0; i < MAX_SIZE; i++) {
        memory_pointer = (byte*)(&(MEMORY[i]));
        (*memory_pointer) = 0;
	}

	// ler instruções do código fonte
    // acessar endereço inicial da memória
    instr_counter = 0;
    memory_word_pointer = (word*)(MEMORY);
	while (fscanf(bin, "%d ", &instruction) != EOF) {
        // copiar instrução para dentro da memória (tamanho = 1 word)
        (*memory_word_pointer) = instruction;
        // ir para próxima posição (+ 4 bytes)
        memory_word_pointer += 1;
        instr_counter++;
	}
    // armazenar endereço da última instrução válida
    last_valid = ((instr_counter - 1) * 4);

	// fechar arquivo do código fonte
	fclose(bin);
	bin = NULL;

	// inicializar banco de registradores
	for (i = 0; i < 32; i++) {
		current_reg = get_register(i);
		(*current_reg) = 0;
	}

	// inicializar PC p/ primeira posição válida
	PC = 0;

}


 /*
  * start
  * ----------------------------
  * Inicializa todos os sinais de controle,
  * memória e registradores
  * Estado 0
  *
  */
void start() {
	int i;
	// inicializar sinais de controle
	// inicializa para o ciclo de busca
	RegDst0     = 0;
	RegDst1     = 0;
	RegWrite    = 0;
	ALUSrcA     = 0;
	// ALUSrcB = 01
	ALUSrcB0    = 1;
	ALUSrcB1    = 0;
	ALUOp0      = 0;
	ALUOp1      = 0;
	PCSource0   = 0;
	PCSource1   = 0;
	PCWriteCond = 0;
	PCWrite     = 0;
	IorD        = 0;
	MemRead     = 1;
	MemWrite    = 0;
	BNE         = 0;
	IRWrite     = 1;
	MemtoReg0   = 0;
	MemtoReg1   = 0;

	// inicializa o vetor de estado
	for(i = 0; i < 5; i++) {
		state[i] = 0;
	}
}

/*
 * finalize
 * ----------------------------
 *   Checa o estado da saída para exibir
 *   a mensagem correta e exibe na tela o
 *   conteúdo dos registradores e da memória.
 *
 */
void finalize() {
	int i, j;
	char* regid = NULL; // identificador do registrador (nome)
	reg* current_reg = NULL; // ponteiro para registrador
    int reg_data = 0; // conteúdo do registrador
    char* exit_message =  NULL; // mensagem de status da saída

    // exibir status da saída
    exit_message = status_message();
    printf("Status da saída: %s", exit_message);
    printf("\n");

    // exibir registradores
	printf("PC = %u\t", PC);
	printf("IR = %u\t", IR);
	printf("MDR = %u\t", MDR);
	printf("\n");
	printf("A = %u\t", A);
	printf("B = %u\t", B);
	printf("AluOut = %u\n", ALUOut);
	printf("Controle = [");
    // bits do sinal de controle
    printf("%d", RegDst0);
    printf("%d", RegDst1);
    printf("%d", RegWrite);
    printf("%d", ALUSrcA);
    printf("%d", ALUSrcB0);
    printf("%d", ALUSrcB1);
    printf("%d", ALUOp0);
    printf("%d", ALUOp1);
    printf("%d", PCSource0);
    printf("%d", PCSource1);
    printf("%d", PCWriteCond);
    printf("%d", PCWrite);
    printf("%d", IorD);
    printf("%d", MemRead);
    printf("%d", MemWrite);
    printf("%d", BNE);
    printf("%d", IRWrite);
    printf("%d", MemtoReg0);
    printf("%d", MemtoReg1);
    printf("]\n");
	printf("\n");

	// exibir banco de registradores
	printf("Banco de Registradores\n");
	for (i = 0; i < 8; i++) {
		for (j = i; j < (i + (8 * 4)); j+=8) {
			regid = register_name(j);
			current_reg = get_register(j);
            reg_data = (int)((*current_reg));
			printf("R%02d (%s) = %-4d\t", j, regid, reg_data);
		}
		printf("\n");
	}
	printf("\n");

    // exibir memória (a byte)
	printf("Memória (endereços a byte)\n");
	// imprimir as 32 primeiras posições de memória (em inteiros sem sinal)
    memory_word_pointer = (word*)(MEMORY);
    // para manter formatação em colunas na sáida, acessamos utilizando índices i e j
    // ponteiro de palavra vai acessar endereço de MEMORY[indice]
	for (i = 0; i < 32; i += 4) {
        memory_word_pointer = (word*)(&(MEMORY[i]));
		for (j = i; j < (i + (32 * 4)); j += 32) {
            memory_word_pointer = (word*)(&MEMORY[j]);
            printf("[%02d] = %-12u\t", j, (*memory_word_pointer));
		}
		printf("\n");
	}
}

/*******************************************************/

// +----------------------+
// | FUNCIONAMENTO DA CPU |
// +----------------------+

/*
 * FUNCOES QUE CHAMAM O FUNCIONAMENTO DE TODOS OS COMPONENTES DE ACORDO COM O CLOCK
 * ----------------------------
 */
void set() {
    CONTROL();
    CONTROL_NEXT();

    MUX_MEMORY();
    SIGNAL_EXTEND_16_TO_32();
    MUX_ALU_1();
    MUX_ALU_2();
    ALU_CONTROL();
    ALU();
    MUX_BNE();
    MUX_WRITE_REG();
    MUX_WRITE_DATA();
    MUX_PC();
}


void go() {
    MEMORY_BANK();
    IR_UNIT();
    PROGRAM_COUNTER();
    ALU_OUT();
    REGISTER_BANK();
    clocks++;
}


/*
 * check_status
 * ----------------------------
 * Verifica o estado da saída do programa,
 * procurando por instruções ou acessos inválidos.
 * Os estados estão definidos no cabeçalho, sendo
 * STATUS_VALID 0, ou seja, enquanto a função não
 * alterar esse valor para outro estado inválido
 * (enquanto retorno for falso),
 * a simulação continua rodando.
 */
int check_status() {
    // checar acesso a memória
    if (MAR > MAX_SIZE || MAR < 0) {
        status = STATUS_INVALID_ACCESS;
    }

    // checar se instrução é inválida
    // 4294967295 = 00xffffffff = linha vazia
    if (MDR == 4294967295) {
        status = STATUS_INVALID_INSTR;
    }

    // checar acesso ao banco de registradores
	// (checar após início do programa, pois começa com null)
    if (write_reg == NULL && clocks > 0) {
        status = STATUS_INVALID_REG;
    }

    // checar se programa já rodou mais
    // de MAX_RUNS vezes, o que pode indicar
    // loop infinito, já que estamos lidando
    // com programas reduzidos.
    if (clocks > MAX_RUNS) {
        status = STATUS_INFINITE;
    }

    return status;
}


/*
 * debugger
 * ----------------------------
 * Escreve no arquivo 'log.txt' o conteúdo
 * de todas os sinais e registradores da simulação.
 * É chamado após cada ciclo.
 *
 */
void debugger() {
    reg* current_reg = NULL;
    char* regid = NULL;
    int i, j;

    fprintf(f_debug, "CLOCKS: %d\n\n", clocks);
    fprintf(f_debug, "*** IR ***\n");
    fprintf(f_debug, "op_code: ");
    for (i = 5; i > -1; i--) {
        fprintf(f_debug, "%d", op_code[i]);
    }
    fprintf(f_debug, "\n");
    fprintf(f_debug, "function: ");
    for (i = 5; i > -1; i--) {
        fprintf(f_debug, "%d", function[i]);
    }
    fprintf(f_debug, "\n");
    fprintf(f_debug, "rs: ");
    for (i = 4; i > -1; i--) {
        fprintf(f_debug, "%d", rs[i]);
    }
    fprintf(f_debug, "\n");
    fprintf(f_debug, "rt: ");
    for (i = 4; i > -1; i--) {
        fprintf(f_debug, "%d", rt[i]);
    }
    fprintf(f_debug, "\n");
    fprintf(f_debug, "rd: ");
    for (i = 4; i > -1; i--) {
        fprintf(f_debug, "%d", rd[i]);
    }
    fprintf(f_debug, "\n");
    fprintf(f_debug, "immediate: ");
    for (i = 15; i > -1; i--) {
        fprintf(f_debug, "%d", immediate[i]);
    }
    fprintf(f_debug, "\n");
    fprintf(f_debug, "jump_addr: ");
    for (i = 31; i > -1; i--) {
        fprintf(f_debug, "%d", jump_addr[i]);
    }
    fprintf(f_debug, "\n");

    fprintf(f_debug, "\n");
    fprintf(f_debug, "*** REGISTRADORES ***\n");

    for (i = 0; i < 8; i++) {
		for (j = i; j < (i + (8 * 4)); j+=8) {
			regid = register_name(j);
			current_reg = get_register(j);
			fprintf(f_debug, "R%02d (%s) = %d\t", j, regid, (*current_reg));
		}
		fprintf(f_debug, "\n");
	}
    fprintf(f_debug, "\n");

    fprintf(f_debug, "MAR: %d\n", MAR);
    fprintf(f_debug, "IR: %d\n", IR);
    fprintf(f_debug, "MDR: %d\n", MDR);

    fprintf(f_debug, "reg_write_data: %d\n", reg_write_data);
    fprintf(f_debug, "PC: %d\n", PC);
    fprintf(f_debug, "pc_write_data: %d\n", pc_write_data);

    fprintf(f_debug, "\n");
    fprintf(f_debug, "*** ULA ***\n");

    fprintf(f_debug, "A: %d\n", A);
    fprintf(f_debug, "B: %d\n", B);
    fprintf(f_debug, "ALUInput: ");
    for (i = 2; i > -1; i--) {
        fprintf(f_debug, "%d", ALUInput[i]);
    }
    fprintf(f_debug, "\n");
    fprintf(f_debug, "ALUResult: %d\n", ALUResult);
    fprintf(f_debug, "ALUOut: %d\n", ALUOut);
    fprintf(f_debug, "immediate_extended: %d\n", immediate_extended);
    fprintf(f_debug, "operator_1: %d\n", operator_1);
    fprintf(f_debug, "operator_2: %d\n", operator_2);
    fprintf(f_debug, "ALU_zero: %d\n", ALU_zero);

    fprintf(f_debug, "\n");
    fprintf(f_debug, "*** SINAIS DE CONTROLE ***\n");

    fprintf(f_debug, "RegDst0: %d\n", RegDst0);
    fprintf(f_debug, "RegDst1: %d\n", RegDst1);
    fprintf(f_debug, "RegWrite: %d\n", RegWrite);
    fprintf(f_debug, "ALUSrcA: %d\n", ALUSrcA);
    fprintf(f_debug, "ALUSrcB0: %d\n", ALUSrcB0);
    fprintf(f_debug, "ALUSrcB1: %d\n", ALUSrcB1);
    fprintf(f_debug, "ALUOp0: %d\n", ALUOp0);
    fprintf(f_debug, "ALUOp1: %d\n", ALUOp1);
    fprintf(f_debug, "PCSource0: %d\n", PCSource0);
    fprintf(f_debug, "PCSource1: %d\n", PCSource1);
    fprintf(f_debug, "PCWriteCond: %d\n", PCWriteCond);
    fprintf(f_debug, "PCWrite: %d\n", PCWrite);
    fprintf(f_debug, "IorD: %d\n", IorD);
    fprintf(f_debug, "MemRead: %d\n", MemRead);
    fprintf(f_debug, "MemWrite: %d\n", MemWrite);
    fprintf(f_debug, "BNE: %d\n", BNE);
    fprintf(f_debug, "IRWrite: %d\n", IRWrite);
    fprintf(f_debug, "MemtoReg0: %d\n", MemtoReg0);
    fprintf(f_debug, "MemtoReg1: %d\n", MemtoReg1);

    fprintf(f_debug, "\n");

    fprintf(f_debug, "PCControl: %d\n", PCControl);

    fprintf(f_debug, "\n");
    fprintf(f_debug, "*** MEMÓRIA ***\n");

    memory_word_pointer = (word*)(MEMORY);
	for (i = 0; i < 28; i += 4) {
        memory_word_pointer = (word*)(&(MEMORY[i]));
		for (j = i; j < (i + (32 * 4)); j += 32) {
            memory_word_pointer = (word*)(&MEMORY[j]);
            fprintf(f_debug, "[%02d] = %u\t", j, (*memory_word_pointer));
		}
		fprintf(f_debug, "\n");
	}

    // exibir próximo estado em decimal para conferir com o diagrama de estados
    fprintf(f_debug, "next state: ");
    fprintf(f_debug, "%d\n", bin2dec(next_state, 5));

    fprintf(f_debug, "\n");
    fprintf(f_debug, "************************************************************\n");
    fprintf(f_debug, "************************************************************\n");
    fprintf(f_debug, "\n");
}


/*
 * cycle
 * ----------------------------
 * Loop de execução do programa. Enquanto
 * o status de saída for válido (0), ele
 * executa as funções de simulação e escreve
 * todos os valores em uso em log.txt.
 *
 */
void cycle() {
    while (1) {
        check_status();
        set();
        go();
        debugger();
        if (status) {
            break;
        }
    }
}


/*
 * interactive
 * ----------------------------
 * Fornece um modo interativo de executar
 * o programa, a cada ENTER sendo um novo
 * clock, exibindo na tela a saída para
 * facilitar a depuração.
 *
 */
void interactive() {
    char buffer = '\n';
    // system("clear");
    while (buffer == '\n') {
        check_status();
        printf("CLOCK ATUAL: %d\n", clocks);
        finalize();
        printf("continuar? ");
        scanf("%c", &buffer);
        set();
        go();
        debugger();
        // system("clear");
        printf("\n\n");
        if (status) {
            break;
        }
    }
    // system("clear");
}


/*******************************************************/

// +------+
// | MAIN |
// +------+
int main(int argc, char const *argv[]) {

	const char* source = NULL;

	// verificar se código fonte foi passado como argumento
	if (argc < 2) {
		printf("ERRO: Código fonte não foi passado como argumento.\n");
		exit(1);
	}

    // abrir arquivo de log
    f_debug = fopen("log.txt", "w");
    // checar integridade do arquivo de log
	if (f_debug == NULL) {
		printf("ERRO: Arquivo de log não abriu.\n");
		exit(0);
	}

	// código fonte
	source = argv[1];

	// inicializar memória e registradores
	initialize(source);

	// inicializar sinais de controle
	start();

    debugger(); // colocar estado inicial no log
    cycle();

    // para facilitar a depuração, use o modo interativo
    // interactive();

    // finalizar execução e exibir informações na tela
    finalize();

    // fechar arquivo de log
    fclose(f_debug);

	return 0;
}
