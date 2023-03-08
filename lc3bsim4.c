/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {                                                  
    IRD,
    ResetEXC,
    COND2, COND1, COND0,
    J5, J4, J3, J2, J1, J0,
    LD_MAR,
    LD_MDR,
    LD_IR,
    LD_BEN,
    LD_REG,
    LD_CC,
    LD_PC,
    LD_VECTOR,
    LD_PRIV,
    LD_EXC,
    LD_SAVEDUSP,
    LD_SAVEDSSP,
    GATE_PC,
    GATE_MDR,
    GATE_ALU,
    GATE_MARMUX,
    GATE_SHF,
    GATE_SP,
    GATE_PSR,
    GATE_PC1,
    GATE_VECTOR,
    PCMUX1, PCMUX0,
    DRMUX1, DRMUX2,
    SR1MUX1, SR2MUX2,
    ADDR1MUX,
    ADDR2MUX1, ADDR2MUX0,
    MARMUX,
    SPMUX1, SPMUX2,
    PSRMUX,
    VECTORMUX1, VECTORMUX2,
    ALUK1, ALUK0,
    MIO_EN,
    R_W,
    DATA_SIZE,
    LSHF1,
    SET_PRIV,
/* MODIFY: you have to add all your new control signals */
    CONTROL_STORE_BITS,
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD]); }
int GetResetEXC(int *x)        { return(x[ResetEXC]); }
int GetCOND(int *x)          { return((x[COND2] << 2) + (x[COND1] << 1) + x[COND0]); }
int GetJ(int *x)             { return((x[J5] << 5) + (x[J4] << 4) +
				      (x[J3] << 3) + (x[J2] << 2) +
				      (x[J1] << 1) + x[J0]); }
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }
int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }
int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x)         { return((x[DRMUX1]<<1)+(x[DRMUX2])); }
int GetSR1MUX(int *x)        { return((x[SR1MUX1]<<1)+(x[SR2MUX2])); }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); } 
int GetLSHF1(int *x)         { return(x[LSHF1]); }
/* MODIFY: you can add more Get functions for your new control signals */
int GetSavedUSP(int *x)      { return (x[LD_SAVEDUSP]); }
int GetSavedSSP(int *x)      { return (x[LD_SAVEDSSP]); }
int GetSPMUX(int *x)         { return ((x[SPMUX1] << 1) + (x[SPMUX2])); }
int GetGATE_SP(int *x)       { return (x[GATE_SP]); }
int GetGATE_PSR(int *x)      { return (x[GATE_PSR]); }
int GetGATE_PC1(int *x)      { return (x[GATE_PC1]); }
int GetPSRMUX(int *x)        { return (x[PSRMUX]); }
int GetLD_VECTOR(int *x)     { return (x[LD_VECTOR]); }
int GetGATE_VECTOR(int *x)   { return (x[GATE_VECTOR]); }
int GetVECTORMUX(int *x)     { return ((x[VECTORMUX1] << 1) + (x[VECTORMUX2])); }
int GetLD_PRIV(int *x)       { return (x[LD_PRIV]); }
int GetLD_EXC(int *x)        { return (x[LD_EXC]); }
int GetSET_PRIV(int *x)      { return (x[SET_PRIV]); }


/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
   There are two write enable signals, one for each byte. WE0 is used for 
   the least significant byte of a word. WE1 is used for the most significant 
   byte of a word. */

#define WORDS_IN_MEM    0x08000 
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */
int BUS;	/* value of the bus */

typedef struct System_Latches_Struct{

int PC,		/* program counter */
    MDR,	/* memory data register */
    MAR,	/* memory address register */
    IR,		/* instruction register */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P,		/* p condition bit */
    BEN;        /* ben register */

int READY;	/* ready bit */
  /* The ready bit is also latched as you dont want the memory system to assert it 
     at a bad point in the cycle*/

int REGS[LC_3b_REGS]; /* register file. */

int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

int STATE_NUMBER; /* Current State Number - Provided for debugging */ 

/* For lab 4 */
int INTV; /* Interrupt vector register */
int EXCV; /* Exception vector register */
int SSP; /* Initial value of system stack pointer */
/* MODIFY: You may add system latches that are required by your implementation */
int PSR; //priv bit
int INT;
int EXC;
int USP; //For user stack
int VECTOR; 
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {                                                    
    printf("----------------LC-3bSIM Help-------------------------\n");
    printf("go               -  run program to completion       \n");
    printf("run n            -  execute program for n cycles    \n");
    printf("mdump low high   -  dump memory from low to high    \n");
    printf("rdump            -  dump the register & bus values  \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  eval_micro_sequencer();   
  cycle_memory();
  eval_bus_drivers();
  drive_bus();
  latch_datapath_values();

  CURRENT_LATCHES = NEXT_LATCHES;

  CYCLE_COUNT++;

  if(CYCLE_COUNT == 300){
    NEXT_LATCHES.INT = 1;
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {                                      
    int i;

    if (RUN_BIT == FALSE) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }

    printf("Simulating for %d cycles...\n\n", num_cycles);
    for (i = 0; i < num_cycles; i++) {
	if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
	}
	cycle();
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed.                 */
/*                                                             */
/***************************************************************/
void go() {                                                     
    if (RUN_BIT == FALSE) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }

    printf("Simulating...\n\n");
    while (CURRENT_LATCHES.PC != 0x0000)
	cycle();
    RUN_BIT = FALSE;
    printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {          
    int address; /* this is a byte address */

    printf("\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	printf("  0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	fprintf(dumpsim_file, " 0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */   
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {                               
    int k; 

    printf("\nCurrent register/bus values :\n");
    printf("-------------------------------------\n");
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%0.4x\n", BUS);
    printf("MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);

    printf("INTERRUPT    : %d\n", CURRENT_LATCHES.INT);
    printf("EXC          : %d\n", CURRENT_LATCHES.EXC);

    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%0.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */  
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {                         
    char buffer[20];
    int start, stop, cycles;

    printf("LC-3b-SIM> ");

    scanf("%s", buffer);
    printf("\n");

    switch(buffer[0]) {
    case 'G':
    case 'g':
	go();
	break;

    case 'M':
    case 'm':
	scanf("%i %i", &start, &stop);
	mdump(dumpsim_file, start, stop);
	break;

    case '?':
	help();
	break;
    case 'Q':
    case 'q':
	printf("Bye.\n");
	exit(0);

    case 'R':
    case 'r':
	if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
	else {
	    scanf("%d", &cycles);
	    run(cycles);
	}
	break;

    default:
	printf("Invalid Command\n");
	break;
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */ 
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {                 
    FILE *ucode;
    int i, j, index;
    char line[200];

    printf("Loading Control Store from file: %s\n", ucode_filename);

    /* Open the micro-code file. */
    if ((ucode = fopen(ucode_filename, "r")) == NULL) {
	printf("Error: Can't open micro-code file %s\n", ucode_filename);
	exit(-1);
    }

    /* Read a line for each row in the control store. */
    for(i = 0; i < CONTROL_STORE_ROWS; i++) {
	if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
	    printf("Error: Too few lines (%d) in micro-code file: %s\n",
		   i, ucode_filename);
	    exit(-1);
	}

	/* Put in bits one at a time. */
	index = 0;

	for (j = 0; j < CONTROL_STORE_BITS; j++) {
	    /* Needs to find enough bits in line. */
	    if (line[index] == '\0') {
		printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
		       ucode_filename, i);
		exit(-1);
	    }
	    if (line[index] != '0' && line[index] != '1') {
		printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
		       ucode_filename, i, j);
		exit(-1);
	    }

	    /* Set the bit in the Control Store. */
	    CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
	    index++;
	}

	/* Warn about extra bits in line. */
	if (line[index] != '\0')
	    printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
		   ucode_filename, i);
    }
    printf("\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {                                           
    int i;

    for (i=0; i < WORDS_IN_MEM; i++) {
	MEMORY[i][0] = 0;
	MEMORY[i][1] = 0;
    }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {                   
    FILE * prog;
    int ii, word, program_base;

    /* Open program file. */
    prog = fopen(program_filename, "r");
    if (prog == NULL) {
	printf("Error: Can't open program file %s\n", program_filename);
	exit(-1);
    }

    /* Read in the program. */
    if (fscanf(prog, "%x\n", &word) != EOF)
	program_base = word >> 1;
    else {
	printf("Error: Program file is empty\n");
	exit(-1);
    }

    ii = 0;
    while (fscanf(prog, "%x\n", &word) != EOF) {
	/* Make sure it fits. */
	if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
		   program_filename, ii);
	    exit(-1);
	}

	/* Write the word to memory array. */
	MEMORY[program_base + ii][0] = word & 0x00FF;
	MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
	ii++;
    }

    if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

    printf("Read %d words from program into memory.\n\n", ii);
}

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */ 
/*             and set up initial state of the machine.        */
/*                                                             */
/***************************************************************/
void initialize(char *argv[], int num_prog_files) { 
    int i;
    init_control_store(argv[1]);

    init_memory();
    for ( i = 0; i < num_prog_files; i++ ) {
	load_program(argv[i + 2]);
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);
    CURRENT_LATCHES.SSP = 0x3000; /* Initial value of system stack pointer */
    CURRENT_LATCHES.USP = 0xFDFF;
    CURRENT_LATCHES.PSR = 1;
    CURRENT_LATCHES.EXC = 0;
    CURRENT_LATCHES.INT = 0;
    CURRENT_LATCHES.VECTOR = 0;
    NEXT_LATCHES = CURRENT_LATCHES;

    RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {                              
    FILE * dumpsim_file;

    /* Error Checking */
    if (argc < 3) {
	printf("Error: usage: %s <micro_code_file> <program_file_1> <program_file_2> ...\n",
	       argv[0]);
	exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv, argc - 2);

    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
	printf("Error: Can't open dumpsim file\n");
	exit(-1);
    }

    while (1)
	get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code, except for the places indicated 
   with a "MODIFY:" comment.

   Do not modify the rdump and mdump functions.

   You are allowed to use the following global variables in your
   code. These are defined above.

   CONTROL_STORE
   MEMORY
   BUS

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */
/***************************************************************/


void eval_micro_sequencer() {

  /* 
   * Evaluate the address of the next state according to the 
   * micro sequencer logic. Latch the next microinstruction.
   */
  int* curInst = CURRENT_LATCHES.MICROINSTRUCTION;
  int nextStateNum = GetJ(curInst);
    if(GetResetEXC(curInst)){
        NEXT_LATCHES.EXC = 0;
    }
    if(((CURRENT_LATCHES.IR >> 13) & 0xF) == 5){
        nextStateNum = 50;
    }
    else{ // evaluate microsequencer gates
    
        if(CURRENT_LATCHES.MICROINSTRUCTION[IRD]){
            nextStateNum = ((CURRENT_LATCHES.IR >> 12)) & 0xF; // Get IR[15:12] to find which state to branch to
        }
        else{
            if(CURRENT_LATCHES.EXC){
                nextStateNum = 50; 
            }
            else{
                if(GetCOND(curInst) == 1){ // Cond0 on 
                    if(CURRENT_LATCHES.READY){
                        nextStateNum = GetJ(curInst) | 0x02; //J1
                    }
                }else if(GetCOND(curInst) == 2){ // Cond1 on
                    if(CURRENT_LATCHES.BEN){
                        nextStateNum = GetJ(curInst) | 0x04; //J2
                    }
                }else if(GetCOND(curInst) == 3){ //Both Cond0 and Cond1 on
                    if(CURRENT_LATCHES.IR & 0x0800){ //IR[11]
                        nextStateNum = GetJ(curInst) | 0x01; //J0
                    }
                }else if(GetCOND(curInst) == 4){ // Cond2 on
                    if(CURRENT_LATCHES.PSR & 0x8000){ //PSR[15]
                        nextStateNum = GetJ(curInst) | 0x08; //J3
                    }
                }else if(GetCOND(curInst) == 5){ //Cond 2 and Cond 0
                    if(CURRENT_LATCHES.INT){
                        NEXT_LATCHES.INT = 0;
                        nextStateNum = GetJ(curInst) | 0x10; //J4
                    }
                }


        }

    }
  }
  NEXT_LATCHES.STATE_NUMBER = nextStateNum;
  printf("%d\n", nextStateNum);
  for(int i = 0; i < CONTROL_STORE_BITS; i++){ //35 bits in a microinstruction
    NEXT_LATCHES.MICROINSTRUCTION[i] = CONTROL_STORE[NEXT_LATCHES.STATE_NUMBER][i]; //populate the microinstruction for the next cycle
  }  
}

int numCycle = 0;
void cycle_memory() {
 
  /* 
   * This function emulates memory and the WE logic. 
   * Keep track of which cycle of MEMEN we are dealing with.  
   * If fourth, we need to latch Ready bit at the end of 
   * cycle to prepare microsequencer for the fifth cycle.  
   */
  int MIOEN = GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION);
  int READY = CURRENT_LATCHES.READY;
  int RW = GetR_W(CURRENT_LATCHES.MICROINSTRUCTION);
  int DATASIZE = GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION);

  if(MIOEN){ 
    numCycle++;
    if(READY && RW){ // R enables the memory to execute correctly and RW = 1 indicates that memory needs to be written to 
        if(DATASIZE){ //If WORD
            int byte0 = CURRENT_LATCHES.MDR & 0x00FF;
            int byte1 = (CURRENT_LATCHES.MDR >> 8) & 0x00FF; 
            MEMORY[((CURRENT_LATCHES.MAR)) >> 1][1] = byte1; 
            MEMORY[((CURRENT_LATCHES.MAR)) >> 1][0] = byte0;
        }
        else{ //BYTE
            if(CURRENT_LATCHES.MAR & 0x1){
                MEMORY[(CURRENT_LATCHES.MAR) >> 1][CURRENT_LATCHES.MAR & 0x1] = ((CURRENT_LATCHES.MDR & 0xFF00) >> 8);
            }else{
                MEMORY[(CURRENT_LATCHES.MAR) >> 1][CURRENT_LATCHES.MAR & 0x1] = (CURRENT_LATCHES.MDR & 0xFF);
            }
            
        }
    }
    if(numCycle == 4){
        NEXT_LATCHES.READY = 1;
    }
    if(numCycle == 5){
        NEXT_LATCHES.READY = 0;
        numCycle = 0;
    }
  }
  
}

int MARMUXGATE = 0;
int GATEPC = 0;
int GATEALU = 0;
int GATESHF = 0;
int GATEMDR = 0;
int GATESP = 0;
int GATEPSR = 0; 
int GATEPC1 = 0;
int GATEVECTOR = 0;

int setALUGate(){
  int SR2OUT;
  int IR = CURRENT_LATCHES.IR;
  int* curInst = CURRENT_LATCHES.MICROINSTRUCTION;
  int SR1 = 0;
  if(GetSR1MUX(curInst) == 1){
    SR1 = CURRENT_LATCHES.REGS[((CURRENT_LATCHES.IR)>>6) & 0x07];
  }else if(GetSR1MUX(curInst) == 0){
    SR1 = CURRENT_LATCHES.REGS[((CURRENT_LATCHES.IR)>>9) & 0x07];
  }else if(GetSR1MUX(curInst) == 2){
    SR1 = CURRENT_LATCHES.REGS[6];
  }
  if((IR >> 5) & 1){ //imm5
    int sign = IR & 0x10;
    if(sign){
        SR2OUT = Low16bits(0xFFF0 | (IR & 0x001F)); 
    }else{
        SR2OUT = Low16bits(0x0000 | (IR & 0x001F));
    }
     
  }else{
    SR2OUT = CURRENT_LATCHES.REGS[(IR) & 0x7];
  }

  if(!GetALUK(curInst)){ //ADD
    GATEALU = Low16bits(SR1 + SR2OUT);
  }else if(GetALUK(curInst) == 1){ //AND
    GATEALU = Low16bits(SR1 & SR2OUT);
  }else if(GetALUK(curInst) == 2){ //XOR
    GATEALU = Low16bits(SR1 ^ SR2OUT);
  }else if(GetALUK(curInst) == 3){ //PASSA
    GATEALU = Low16bits(SR1); 
  }
}

int setMARMUXGate(){
    int* curInst = CURRENT_LATCHES.MICROINSTRUCTION;
    int IR = CURRENT_LATCHES.IR;
    if(!GetMARMUX(curInst)){ //ZEXT & LSHF1
        MARMUXGATE = (IR & 0xFF) << 1; // ZEXT(LSHF(IR[7:0]),1)
    }else{  //ADDER
        int ADR1 = GetADDR1MUX(curInst);
        int ADR1OUT = 0;
        int ADR2OUT = 0;
        if(!ADR1){ //PC
            ADR1OUT = CURRENT_LATCHES.PC;
        }
        else{
            if(GetSR1MUX(curInst) == 2){
                ADR1OUT = CURRENT_LATCHES.REGS[6];
            }
            else if(GetSR1MUX(curInst) == 1){
                ADR1OUT = CURRENT_LATCHES.REGS[((CURRENT_LATCHES.IR)>>6) & 0x7];
            }else if(GetSR1MUX(curInst) == 0){
                ADR1OUT = CURRENT_LATCHES.REGS[((CURRENT_LATCHES.IR)>>9) & 0x7];
            }
        }
        if(GetADDR2MUX(curInst) == 1){ //offset6, IR[5:0]
            int sign = CURRENT_LATCHES.IR & 0x20;
            if(sign){
                int mask = 65472;
                ADR2OUT = Low16bits((CURRENT_LATCHES.IR & 0x003F) | mask);
                ADR2OUT *= -1;
            }
            else{
                ADR2OUT = Low16bits(CURRENT_LATCHES.IR & 0x003F);
            }
            
        }
        else if(!GetADDR2MUX(curInst)){ //0
            ADR2OUT = 0;
        }else if(GetADDR2MUX(curInst) == 2){ //PCOffset9, IR[8:0]
            int sign = CURRENT_LATCHES.IR & 0x100; 
            if(sign){
                int mask = 65024;
                ADR2OUT = Low16bits(((CURRENT_LATCHES.IR) & 0x01FF) | mask);
                ADR2OUT *= -1;
            }
            else{
                ADR2OUT = Low16bits((CURRENT_LATCHES.IR) & 0x01FF);
            }
        }else if(GetADDR2MUX(curInst) == 3){ //PCOffset11, IR[10:0]
            int sign = CURRENT_LATCHES.IR & 0x400; 
            if(sign){
                int mask = 63488;
                ADR2OUT = Low16bits(((CURRENT_LATCHES.IR) & 0x07FF) | mask);
                ADR2OUT *= -1;
            }
            else{
                ADR2OUT = Low16bits((CURRENT_LATCHES.IR) & 0x07FF);
            }
        } 
        if(GetLSHF1(curInst)){
            ADR2OUT = ADR2OUT << 1;
        }

        MARMUXGATE = ADR1OUT + ADR2OUT; 
    }
}

int setPCGate(){
    GATEPC = CURRENT_LATCHES.PC;
}

int setSHFGate(){
      
    int* curInst = CURRENT_LATCHES.MICROINSTRUCTION;
    int SR;
    if(GetSR1MUX(curInst)){
        SR = ((CURRENT_LATCHES.IR >> 6) & 0x07);
    }else{
        SR = ((CURRENT_LATCHES.IR >> 9) & 0x07);
    }
     
    int IR = CURRENT_LATCHES.IR;
    int sign = (CURRENT_LATCHES.REGS[SR] & 0x8000);
    int amt4 = (IR & 0x0F);
    if(IR & 0x0010){ //Right shift
        if(IR & 0x0020){ //RSHFA
            if(sign){
                int mask = (-1*sign) >> amt4;
                GATESHF = Low16bits(CURRENT_LATCHES.REGS[SR] >> amt4) | mask;
            }
            else{ //ZEXT
                GATESHF = Low16bits((CURRENT_LATCHES.REGS[SR] >> amt4));
            }        
        }
        else{ //RSHFL
            GATESHF = Low16bits((CURRENT_LATCHES.REGS[SR] >> amt4));
        }
    }
    else{ //LSHF
        GATESHF = Low16bits((CURRENT_LATCHES.REGS[SR] << amt4));
    }
}

int setMDRGate(){ 
    int* curInst = CURRENT_LATCHES.MICROINSTRUCTION;
    int mar0 = CURRENT_LATCHES.MAR & 0x1; //MAR[0]
    if(GetDATA_SIZE(curInst)){ //WORD
        GATEMDR = Low16bits(CURRENT_LATCHES.MDR);
    }else{//BYTE
        if(CURRENT_LATCHES.MAR & 0x1){
            GATEMDR = ((CURRENT_LATCHES.MDR & 0xFF00) >> 8);
            int sign = (GATEMDR & 0x80);
            if(sign){
                GATEMDR |= 0xFF00;
            }
        }else{
            GATEMDR = (CURRENT_LATCHES.MDR & 0x00FF);
            int sign = GATEMDR & 0x80;
            if(sign){
                GATEMDR |= 0xFF00;
            }
        }
    }
}

int setGATESP(){
    int* curInst = CURRENT_LATCHES.MICROINSTRUCTION;
    if(GetSPMUX(curInst) == 0){
        GATESP = CURRENT_LATCHES.REGS[6] + 2;
    }else if(GetSPMUX(curInst) == 1){
        GATESP = CURRENT_LATCHES.REGS[6] - 2;
    }else if(GetSPMUX(curInst) == 2){
        GATESP = CURRENT_LATCHES.SSP;
    }else if(GetSPMUX(curInst) == 3){
        GATESP = CURRENT_LATCHES.USP;
    }
}

int setGATEPSR(){
    GATEPSR = (CURRENT_LATCHES.PSR << 15) | (CURRENT_LATCHES.N << 2) | (CURRENT_LATCHES.Z << 1) | (CURRENT_LATCHES.P);
    
}

int setGATEPC1(){
    GATEPC1 = CURRENT_LATCHES.PC - 2;
}

int setGATEVECTOR(){
    GATEVECTOR = (0x02 << 8) + ((CURRENT_LATCHES.VECTOR & 0xFF) << 1);
}

void eval_bus_drivers() {

  /* 
   * Datapath routine emulating operations before driving the bus.
   * Evaluate the input of tristate drivers 
   *             Gate_MARMUX,
   *		 Gate_PC,
   *		 Gate_ALU,
   *		 Gate_SHF,
   *		 Gate_MDR.
   */    
  setALUGate();
  setMARMUXGate();
  setPCGate();
  setSHFGate();
  setMDRGate();
  setGATESP();
  setGATEPSR();
  setGATEPC1();
  setGATEVECTOR();
}


void drive_bus() {

  /* 
   * Datapath routine for driving the bus from one of the 5 possible 
   * tristate drivers. 
   */ 

  int* curInst = CURRENT_LATCHES.MICROINSTRUCTION;

  if(GetGATE_ALU(curInst)){
    BUS = Low16bits(GATEALU);
  }else if(GetGATE_MARMUX(curInst)){
    BUS =  Low16bits(MARMUXGATE);
  }else if(GetGATE_MDR(curInst)){
    BUS = Low16bits(GATEMDR);
  }else if(GetGATE_SHF(curInst)){
    BUS =  Low16bits(GATESHF);
  }else if(GetGATE_PC(curInst)){
    BUS = Low16bits(GATEPC);
  }else if(GetGATE_SP(curInst)){
    BUS = Low16bits(GATESP);
  }else if(GetGATE_PSR(curInst)){
    BUS = Low16bits(GATEPSR);
  }else if(GetGATE_PC1(curInst)){
    BUS = Low16bits(GATEPC1);
  }else if(GetGATE_VECTOR(curInst)){
    BUS = Low16bits(GATEVECTOR);
  }
  else{
    BUS = 0;
  }
}


void latch_datapath_values() {

  /* 
   * Datapath routine for computing all functions that need to latch
   * values in the data path at the end of this cycle.  Some values
   * require sourcing the bus; therefore, this routine has to come 
   * after drive_bus.
   */       
  int* curInst = CURRENT_LATCHES.MICROINSTRUCTION;     
  if(GetLD_BEN(curInst)){
    NEXT_LATCHES.BEN = (((CURRENT_LATCHES.IR) & 0x0800) && CURRENT_LATCHES.N) || (((CURRENT_LATCHES.IR) & 0x0400) && CURRENT_LATCHES.Z) || (((CURRENT_LATCHES.IR) & 0x0200) && CURRENT_LATCHES.P);

  }if(GetLD_CC(curInst)){
    
    int cc = Low16bits(BUS);
    int size = 8*sizeof(int);
    if(cc<<15){
        cc = cc << (size-16);
        cc = cc >> (size-16);
    }
    if(cc == 0){
        NEXT_LATCHES.N = 0;
        NEXT_LATCHES.Z = 1;
        NEXT_LATCHES.P = 0;
    }else if(cc > 0){
        NEXT_LATCHES.N = 0;
        NEXT_LATCHES.Z = 0;
        NEXT_LATCHES.P = 1;
    }else if(cc < 0){
        NEXT_LATCHES.N = 1;
        NEXT_LATCHES.Z = 0;
        NEXT_LATCHES.P = 0;
    }

  }if(GetLD_IR(curInst)){
    NEXT_LATCHES.IR = Low16bits(BUS);

  }if(GetLD_MAR(curInst)){
    NEXT_LATCHES.MAR = Low16bits(BUS);

  }if(GetLD_MDR(curInst)){
    int mar = CURRENT_LATCHES.MAR;
    int mar0 = CURRENT_LATCHES.MAR & 0x1; //MAR[0]
    int READY = CURRENT_LATCHES.READY;
    int RW = GetR_W(curInst);
    int DATASIZE = GetDATA_SIZE(curInst);

    if(GetMIO_EN(curInst)){ 
         if(READY && !RW){ // IF we're reading
            NEXT_LATCHES.MDR = Low16bits((((MEMORY[((CURRENT_LATCHES.MAR)) >> 1][1])) << 8) + MEMORY[((CURRENT_LATCHES.MAR)) >> 1 ][0]);
        }

    }else{
        if(GetDATA_SIZE(curInst)){
            NEXT_LATCHES.MDR = Low16bits(BUS);
        }else{ 
            if(CURRENT_LATCHES.MAR & 0x1){
                NEXT_LATCHES.MDR = (BUS & 0x00FF) << 8;
            }else{
                NEXT_LATCHES.MDR = (BUS & 0x00FF);
            }
        }
    }

  }
  if(GetLD_PC(curInst)){
    if(GetPCMUX(curInst) == 1){
        NEXT_LATCHES.PC = Low16bits(BUS);
    }
    else if(GetPCMUX(curInst) == 0){
        NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 2;
    }
    else if(GetPCMUX(curInst) == 2){
        int SR1 = 0;
        int ADR2OUT = 0;
        if(GetADDR1MUX(curInst)){
            if(GetSR1MUX(curInst) == 1){
                SR1 = CURRENT_LATCHES.REGS[((CURRENT_LATCHES.IR)>>6) & 0x07];
            }else if(GetSR1MUX(curInst) == 0){
                SR1 = CURRENT_LATCHES.REGS[((CURRENT_LATCHES.IR)>>9) & 0x07];
            }else if(GetSR1MUX(curInst) == 2){
                SR1 = CURRENT_LATCHES.REGS[6];
            }
        }else{
            SR1 = CURRENT_LATCHES.PC;
        }
        if(GetADDR2MUX(curInst) == 1){ //offset6, IR[5:0]
            int sign = CURRENT_LATCHES.IR & 0x20;
            if(sign){
                int mask = 65472;
                ADR2OUT = Low16bits((CURRENT_LATCHES.IR & 0x003F) | mask);
                ADR2OUT *= -1;
            }
            else{
                ADR2OUT = Low16bits(CURRENT_LATCHES.IR & 0x003F);
            }
            
        }
        else if(GetADDR2MUX(curInst) == 0){ //0
            ADR2OUT = 0;
        }else if(GetADDR2MUX(curInst) == 2){ //PCOffset9, IR[8:0]
            int sign = CURRENT_LATCHES.IR & 0x100; 
            if(sign){
                int mask = 65024;
                ADR2OUT = Low16bits(((CURRENT_LATCHES.IR) & 0x01FF) | mask);
                ADR2OUT *= -1;
            }
            else{
                ADR2OUT = Low16bits((CURRENT_LATCHES.IR) & 0x01FF);
            }
        }else if(GetADDR2MUX(curInst) == 3){ //PCOffset11, IR[10:0]
            int sign = CURRENT_LATCHES.IR & 0x400; 
            if(sign){
                int mask = 63488;
                ADR2OUT = Low16bits(((CURRENT_LATCHES.IR) & 0x07FF) | mask);
                ADR2OUT *= -1;
            }
            else{
                ADR2OUT = Low16bits((CURRENT_LATCHES.IR) & 0x07FF);
            }
            
        } 
        
        if(GetLSHF1(curInst)){
            ADR2OUT = (ADR2OUT << 1) & 0xFFFF;
        }      
        printf("ADR2OUT = %x\n", ADR2OUT);
        NEXT_LATCHES.PC = SR1 + ADR2OUT;
        printf("NEXT_LATCHES.PC = %x\n", NEXT_LATCHES.PC);
        
    }

  }
  if(GetLD_REG(curInst)){
    if(GetDRMUX(curInst) == 2){ // 2
        NEXT_LATCHES.REGS[6] = Low16bits(BUS);
    }
    else if(GetDRMUX(curInst) == 1){ // 1
        NEXT_LATCHES.REGS[7] = Low16bits(BUS);
    }else{ // 0
        NEXT_LATCHES.REGS[((CURRENT_LATCHES.IR) >> 9) & 0x7] = Low16bits(BUS); 
    }
  }
  if(GetLD_EXC(curInst)){
    if((CURRENT_LATCHES.PSR == 1) && (BUS < 0x2FFF && BUS >= 0x0000) && CURRENT_LATCHES.STATE_NUMBER != 18 && CURRENT_LATCHES.STATE_NUMBER != 19){//Protection
        NEXT_LATCHES.EXC = 1;
        printf("PROT\n");
    }else if(GetJ(curInst) == 10 || GetJ(curInst) == 11){ //Unknownop
        NEXT_LATCHES.EXC = 1;
        printf("UNKNOWNOP\n");
    }else if(((BUS & 0x1) == 1) && (CURRENT_LATCHES.STATE_NUMBER == 6 || CURRENT_LATCHES.STATE_NUMBER == 7)){//Unaligned
        NEXT_LATCHES.EXC = 1;
        printf("UNALIGNED\n");
    }

    if(GetResetEXC(curInst)){
        NEXT_LATCHES.EXC = 0;
        printf("RESET\n");
    }

  }
  if(GetLD_PRIV(curInst)){
    if(GetPSRMUX(curInst) == 1){
        NEXT_LATCHES.PSR = (Low16bits(BUS)) >> 15;
    }else{
        NEXT_LATCHES.PSR = GetSET_PRIV(curInst);
    }
  }if(GetLD_VECTOR(curInst)){
    if((CURRENT_LATCHES.PSR == 1) && (BUS < 0x2FFF && BUS >= 0x0000) && CURRENT_LATCHES.STATE_NUMBER != 18 && CURRENT_LATCHES.STATE_NUMBER != 19){//Prot
        NEXT_LATCHES.VECTOR = 0x02;
    }else{
        if(((CURRENT_LATCHES.MAR & 0x1) == 1) && (CURRENT_LATCHES.STATE_NUMBER == 6 || CURRENT_LATCHES.STATE_NUMBER == 7)){//unaligned
            NEXT_LATCHES.VECTOR = 0x03;
        }else{
            if(GetVECTORMUX(curInst) == 0){//INTV
                NEXT_LATCHES.VECTOR = 0x01;
            }else if(GetVECTORMUX(curInst) == 1){ //Protec
                NEXT_LATCHES.VECTOR = 0x02;
            }else if(GetVECTORMUX(curInst) == 2){ //Unaligned
                NEXT_LATCHES.VECTOR = 0x03;
            }else if(GetVECTORMUX(curInst) == 3){// Unkownop
                NEXT_LATCHES.VECTOR = 0x04;
            }
        }
    }
  }if(GetSavedSSP(curInst)){
    NEXT_LATCHES.SSP = CURRENT_LATCHES.REGS[6];
  }if(GetSavedUSP(curInst)){
    NEXT_LATCHES.USP = CURRENT_LATCHES.REGS[6]; 
  }

}
