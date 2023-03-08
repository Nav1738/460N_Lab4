#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

int isOpcode(char* linePointer);
void findAddress(char* arg, int address, char* output);
void convertRegister(char* reg, char* bin);
void convertNumber(char* num, int bits, char* bin);
void decToBinary(int dec, int bits, char* bin);
int toNum(char* pStr);


#define ADD_OPCODE "0001"
#define AND_OPCODE "0101"
#define BR_OPCODE "0000"
#define JMP_OPCODE "1100"
#define RET_OPCODE "1100"
#define JSR_OPCODE "0100"
#define JSRR_OPCODE "0100"
#define LDB_OPCODE "0010"
#define LDW_OPCODE "0110"
#define LEA_OPCODE "1110"
#define RTI_OPCODE "1000"
#define LSHF_OPCODE "1101"
#define RSHFL_OPCODE "1101"
#define RSHFA_OPCODE "1101"
#define STB_OPCODE "0011"
#define STW_OPCODE "0111"
#define TRAP_OPCODE "1111"
#define XOR_OPCODE "1001"
#define NOT_OPCODE "1001"

#define REGISTER_BIT_SIZE 3
#define MACHINE_CODE_SIZE 16

#define MAX_LINE_LENGTH 255
#define MAX_LABEL_LENGTH 20
#define MAX_SYMBOLS 255

typedef struct {
    int address;
    char label[MAX_LABEL_LENGTH+1];
} TableEntry;
TableEntry symbolTable[MAX_SYMBOLS];
int numSymbols = 0; //counter variable for symbols

char opCodeList[28][6] = {"add", "and", "br", "brn", "brz", "brp", "brnz", "brnp", "brzp", "brnzp", 
                        "jmp", "ret", "jsr", "jsrr", "ldb", "ldw", "lea", "rti", "lshf", "rshfl",
                        "rshfa", "stb", "stw", "trap", "xor", "not", "nop", "halt"};

enum {DONE, OK, EMPTY_LINE};


void convertRegister(char* reg, char* bin) { //R12
    int dec = atoi(reg+1);
    decToBinary(dec, REGISTER_BIT_SIZE, bin);
}
void convertNumber(char* num, int bits, char* bin) { //#42 or x40
    int dec = toNum(num);
    decToBinary(dec, bits, bin);
}
void decToBinary(int dec, int bits, char* bin) { // dec can be negative or positive
    int b = 0; // index of bin
    int negative = 0;
    if (dec < 0) {
        dec = -dec - 1;
        negative = 1;
    }    
    for (int b = bits-1; b >= 0; b--) {
        if (dec%2 == 0) {
            if (!negative) {
                bin[b] = '0';
            } else {
                bin[b] = '1';
            }
        } else {
            if (!negative) {
                bin[b] = '1';
            } else {
                bin[b] = '0';
            }
        }
        dec /= 2;
    }
    bin[bits] = 0;
}

int toNum(char* pStr) { // convert dec or hex to dec
    char* t_ptr;
    char* orig_pStr;
    int t_length,k;
    int lNum, lNeg = 0;
    long int lNumLong;

    orig_pStr = pStr;

    if (*pStr == '#') { // decimal 
        pStr++;
        if(*pStr == '-') { // dec is negative
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for(k = 0; k < t_length; k++) {
            //if (!isdigit(*t_ptr)) {
	        //    printf("Error: invalid decimal operand, %s\n",orig_pStr);
	        //    exit(4);
            //}
            t_ptr++;
        }
        lNum = atoi(pStr);
        if (lNeg) lNum = -lNum;
 
        return lNum;
    }
    else if (*pStr == 'x') { // hex
        pStr++;
        if (*pStr == '-') { // hex is negative
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for(k = 0; k < t_length; k++) {
            //if (!isxdigit(*t_ptr)) {
	        //    printf("Error: invalid hex operand, %s\n",orig_pStr);
	        //    exit(4);
            //}
            t_ptr++;
        }
        lNumLong = strtol(pStr, NULL, 16); // convert hex string into integer
        lNum = (lNumLong > INT_MAX)? INT_MAX : lNumLong;
        if(lNeg) lNum = -lNum;
        return lNum;
    }
    else {
	    // printf("Error: invalid , %s\n", orig_pStr);
	    exit(4);  /* This has been changed from error code 3 to error code 4, see clarification 12 */
    }
}

void ADD1(char* DR, char* SR1, char* SR2, char* binary) {
    char DR_bin[REGISTER_BIT_SIZE+1], SR1_bin[REGISTER_BIT_SIZE+1], SR2_bin[REGISTER_BIT_SIZE+1];
    convertRegister(DR, DR_bin);
    convertRegister(SR1, SR1_bin);
    convertRegister(SR2, SR2_bin);

    snprintf(binary, MACHINE_CODE_SIZE+1, "%s%s%s%s%s", ADD_OPCODE, DR_bin, SR1_bin, "000", SR2_bin);
}

void ADD2(char* DR, char* SR1, char* imm5, char* binary) {
    char DR_bin[REGISTER_BIT_SIZE+1], SR1_bin[REGISTER_BIT_SIZE+1], imm5_bin[5+1];
    convertRegister(DR, DR_bin);
    convertRegister(SR1, SR1_bin);
    convertNumber(imm5, 5, imm5_bin);

    snprintf(binary, MACHINE_CODE_SIZE+1, "%s%s%s%s%s", ADD_OPCODE, DR_bin, SR1_bin, "1", imm5_bin);
}

void AND1(char* DR, char* SR1, char* SR2, char* binary) {
    char DR_bin[REGISTER_BIT_SIZE+1], SR1_bin[REGISTER_BIT_SIZE+1], SR2_bin[REGISTER_BIT_SIZE+1];
    convertRegister(DR, DR_bin);
    convertRegister(SR1, SR1_bin);
    convertRegister(SR2, SR2_bin);

    snprintf(binary, MACHINE_CODE_SIZE+1, "%s%s%s%s%s", AND_OPCODE, DR_bin, SR1_bin, "000", SR2_bin);
}

void AND2(char* DR, char* SR1, char* imm5, char* binary) {
    char DR_bin[REGISTER_BIT_SIZE+1], SR1_bin[REGISTER_BIT_SIZE+1], imm5_bin[5+1];
    convertRegister(DR, DR_bin);
    convertRegister(SR1, SR1_bin);
    convertNumber(imm5, 5, imm5_bin);

    snprintf(binary, MACHINE_CODE_SIZE+1, "%s%s%s%s%s", AND_OPCODE, DR_bin, SR1_bin, "1", imm5_bin);
}

void BR(int n, int z, int p, char* pcoffset9, char* binary) { // offset would be a hex or dec
    char pcoffset9_bin[9+1], nbit[2], zbit[2], pbit[2];

    sprintf(nbit, "%d", n);
    sprintf(zbit, "%d", z);
    sprintf(pbit, "%d", p);
    convertNumber(pcoffset9, 9, pcoffset9_bin);
    
    snprintf(binary, MACHINE_CODE_SIZE+1, "%s%s%s%s%s", BR_OPCODE, nbit, zbit, pbit, pcoffset9_bin);
}

void JMP(char* baseR, char* binary) {
    char baseR_bin[REGISTER_BIT_SIZE+1];
    convertRegister(baseR, baseR_bin);

    snprintf(binary, MACHINE_CODE_SIZE+1, "%s%s%s%s", JMP_OPCODE, "000", baseR_bin, "000000");
}

void RET(char* binary) {
    snprintf(binary, MACHINE_CODE_SIZE+1, "%s%s", JMP_OPCODE, "000111000000");
}

void JSR(char* pcoffset11, char* binary) {
    char pcoffset11_bin[11+1];
    convertNumber(pcoffset11, 11, pcoffset11_bin);

    snprintf(binary, MACHINE_CODE_SIZE+1, "%s%s%s", JSR_OPCODE, "1", pcoffset11_bin);
}

void JSRR(char* baseR, char* binary) {
    char baseR_bin[REGISTER_BIT_SIZE+1];
    convertRegister(baseR, baseR_bin);

    snprintf(binary, MACHINE_CODE_SIZE+1, "%s%s%s%s", JSRR_OPCODE, "000", baseR_bin, "000000");
}

void LDB(char* DR, char* baseR, char* boffset6, char* binary) {
    char DR_bin[REGISTER_BIT_SIZE+1], baseR_bin[REGISTER_BIT_SIZE+1], boffset6_bin[6+1];
    convertRegister(DR, DR_bin);
    convertRegister(baseR, baseR_bin);
    convertNumber(boffset6, 6, boffset6_bin);

    snprintf(binary, MACHINE_CODE_SIZE+1, "%s%s%s%s", LDB_OPCODE, DR_bin, baseR_bin, boffset6_bin);
}

void LDW(char* DR, char* baseR, char* offset6, char* binary) {
    char DR_bin[REGISTER_BIT_SIZE+1], baseR_bin[REGISTER_BIT_SIZE+1], offset6_bin[6+1];
    convertRegister(DR, DR_bin);
    convertRegister(baseR, baseR_bin);
    convertNumber(offset6, 6, offset6_bin);

    snprintf(binary, MACHINE_CODE_SIZE+1, "%s%s%s%s", LDW_OPCODE, DR_bin, baseR_bin, offset6_bin);
}

void LEA(char* DR, char* pcoffset9, char* binary) {
    char DR_bin[REGISTER_BIT_SIZE+1], pcoffset9_bin[9+1];
    convertRegister(DR, DR_bin);
    convertNumber(pcoffset9, 9, pcoffset9_bin);

    snprintf(binary, MACHINE_CODE_SIZE+1, "%s%s%s", LEA_OPCODE, DR_bin, pcoffset9_bin);
}

void RTI(char* binary) {
    snprintf(binary, MACHINE_CODE_SIZE+1, "%s%s", RTI_OPCODE, "000000000000");
}

void LSHF(char* DR, char* SR, char* ammount4, char* binary) {
    char DR_bin[REGISTER_BIT_SIZE+1], SR_bin[REGISTER_BIT_SIZE+1], ammount4_bin[4+1];
    convertRegister(DR, DR_bin);
    convertRegister(SR, SR_bin);
    convertNumber(ammount4, 4, ammount4_bin);

    snprintf(binary, MACHINE_CODE_SIZE+1, "%s%s%s%s%s", LSHF_OPCODE, DR_bin, SR_bin, "00", ammount4_bin);
}

void RSHFL(char* DR, char* SR, char* ammount4, char* binary) {
    char DR_bin[REGISTER_BIT_SIZE+1], SR_bin[REGISTER_BIT_SIZE+1], ammount4_bin[4+1];
    convertRegister(DR, DR_bin);
    convertRegister(SR, SR_bin);
    convertNumber(ammount4, 4, ammount4_bin);

    snprintf(binary, MACHINE_CODE_SIZE+1, "%s%s%s%s%s", RSHFL_OPCODE, DR_bin, SR_bin, "01", ammount4_bin);
}

void RSHFA(char* DR, char* SR, char* ammount4, char* binary) {
    char DR_bin[REGISTER_BIT_SIZE+1], SR_bin[REGISTER_BIT_SIZE+1], ammount4_bin[4+1];
    convertRegister(DR, DR_bin);
    convertRegister(SR, SR_bin);
    convertNumber(ammount4, 4, ammount4_bin);

    snprintf(binary, MACHINE_CODE_SIZE+1, "%s%s%s%s%s", RSHFA_OPCODE, DR_bin, SR_bin, "11", ammount4_bin);
}

void STB(char* SR, char* baseR, char* offset6, char* binary) {
    char SR_bin[REGISTER_BIT_SIZE+1], baseR_bin[REGISTER_BIT_SIZE+1], boffset6_bin[6+1];
    convertRegister(SR, SR_bin);
    convertRegister(baseR, baseR_bin);
    convertNumber(offset6, 6, boffset6_bin);

    snprintf(binary, MACHINE_CODE_SIZE+1, "%s%s%s%s", STB_OPCODE, SR_bin, baseR_bin, boffset6_bin);
}

void STW(char* SR, char* baseR, char* offset6, char* binary) {
    char SR_bin[REGISTER_BIT_SIZE+1], baseR_bin[REGISTER_BIT_SIZE+1], boffset6_bin[6+1];
    convertRegister(SR, SR_bin);
    convertRegister(baseR, baseR_bin);
    convertNumber(offset6, 6, boffset6_bin);

    snprintf(binary, MACHINE_CODE_SIZE+1, "%s%s%s%s", STW_OPCODE, SR_bin, baseR_bin, boffset6_bin);
}

void TRAP(char* trapvect8, char* binary) {
    char trapvect8_bin[8+1];
    convertNumber(trapvect8, 8, trapvect8_bin);

    snprintf(binary, MACHINE_CODE_SIZE+1, "%s%s%s", TRAP_OPCODE, "0000", trapvect8_bin);
}

void XOR1(char* DR, char* SR1, char* SR2, char* binary) {
    char DR_bin[REGISTER_BIT_SIZE+1], SR1_bin[REGISTER_BIT_SIZE+1], SR2_bin[REGISTER_BIT_SIZE+1];
    convertRegister(DR, DR_bin);
    convertRegister(SR1, SR1_bin);
    convertRegister(SR2, SR2_bin);

    snprintf(binary, MACHINE_CODE_SIZE+1, "%s%s%s%s%s", XOR_OPCODE, DR_bin, SR1_bin, "000", SR2_bin);
}

void XOR2(char* DR, char* SR1, char* imm5, char* binary) {
    char DR_bin[REGISTER_BIT_SIZE+1], SR1_bin[REGISTER_BIT_SIZE+1], imm5_bin[5+1];
    convertRegister(DR, DR_bin);
    convertRegister(SR1, SR1_bin);
    convertNumber(imm5, 5, imm5_bin);

    snprintf(binary, MACHINE_CODE_SIZE+1, "%s%s%s%s%s", XOR_OPCODE, DR_bin, SR1_bin, "1", imm5_bin);
}

void NOT(char* DR, char* SR, char* binary) {
    char DR_bin[REGISTER_BIT_SIZE+1], SR_bin[REGISTER_BIT_SIZE+1];
    convertRegister(DR, DR_bin);
    convertRegister(SR, SR_bin);

    snprintf(binary, MACHINE_CODE_SIZE+1, "%s%s%s%s", NOT_OPCODE, DR_bin, SR_bin, "111111");
}

int readAndParse (FILE* pInfile, char* pLine, char** pLabel, char** pOpcode, 
    char** pArg1, char** pArg2, char** pArg3, char** pArg4) {
    // This will read a line and set the neccessary arguments based on the opCode or pseudoOp
	
    char* lRet,* lPtr;
	int i;
	
    if (!fgets(pLine, MAX_LINE_LENGTH, pInfile)) return (DONE);
	for (i = 0; i < strlen(pLine); i++) {
        pLine[i] = tolower(pLine[i]);
    }
    // convert entire line to lowercase
	*pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = pLine + strlen(pLine);
	   
    // ignore the comments
	lPtr = pLine;

	while (*lPtr != ';' && *lPtr != '\0' && *lPtr != '\n') {
        lPtr++;
    }

	*lPtr = '\0';
	if (!(lPtr = strtok(pLine, "\t\n ,"))) return (EMPTY_LINE);

	if (isOpcode(lPtr) == -1 && lPtr[0] != '.') { // found a label
	    *pLabel = lPtr;
		if (!(lPtr = strtok(NULL, "\t\n ,"))) return (OK);
	}
	   
    *pOpcode = lPtr;
	if (!(lPtr = strtok(NULL, "\t\n ,"))) return (OK);  
    *pArg1 = lPtr;

    if (!(lPtr = strtok(NULL, "\t\n ,"))) return(OK);
	*pArg2 = lPtr;
	   
    if (!(lPtr = strtok(NULL, "\t\n ,"))) return(OK);
    *pArg3 = lPtr;

	if (!(lPtr = strtok(NULL, "\t\n ,"))) return(OK);
    *pArg4 = lPtr;

	return (OK);
}

int isOpcode(char* linePointer) {
    for (int i = 0; i < (sizeof(opCodeList)/sizeof(*opCodeList)); i++) {
        if (strcmp(linePointer, opCodeList[i]) == 0) {
            return 1; // if opcode
        }
    }
    return -1; // if label
}

int assembler(char* inputFile, char* outputFile) {
    char lLine[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1, *lArg2, *lArg3, *lArg4;
	int lRet;
    FILE* lInfile = fopen(inputFile, "r"); // open the input file
    FILE * pOutfile = fopen(outputFile, "w" );
    // printf(inputFile); printf("\n");
    // printf(outputFile); printf("\n");

    // FIRST PASS THAT MAKES SYMBOL TABLE
    int address_number = 0; // set equal to .orig
    char lLine_pass1[MAX_LINE_LENGTH + 1];

    while (fgets(lLine_pass1, MAX_LINE_LENGTH, lInfile)) {
        for (int i = 0; i < strlen(lLine_pass1); i++) {
            lLine_pass1[i] = tolower(lLine_pass1[i]);
        }
        char* lptr_pass1 = lLine_pass1;
        while (isspace((unsigned char)*lptr_pass1)) lptr_pass1 ++;

        if (*lptr_pass1 == ';' || *lptr_pass1 == '\0' || *lptr_pass1 == '\n') {
            continue;
        }
         
        // look for .orig and set linenumber equal to the value and populate symbol table
        char *lSymbol = '\0';
        if (lSymbol = strtok(lptr_pass1, "\t\n ,")) {
            if (lSymbol[0] == '.' && !strcmp(lSymbol, ".orig")) {
                char* addr = strtok(NULL, "\t\n ,");
                address_number = toNum(addr)-2;
            }
            else if (isOpcode(lSymbol) == -1 && lSymbol[0] != '.') { // found a label
                symbolTable[numSymbols].address = address_number; // add to symbol table
                strcpy(symbolTable[numSymbols].label, lSymbol);
                numSymbols++;
            }
        }
        address_number+=2;
    }
    // printf("Symbol Table: \n");
    // for (int i = 0; i < numSymbols; i++) {
    //     printf("%x  |  %s\n", symbolTable[i].address, symbolTable[i].label);
    // }
    // printf("\n");
    rewind(lInfile);
    int address = 0;
    // SECOND PASS THAT MAKES MACHINE TCODE
    do {
	    lRet = readAndParse(lInfile, lLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4);
		if (lRet != DONE && lRet != EMPTY_LINE) {
            char binaryLine[MACHINE_CODE_SIZE+1] = "";
            // printf("Label: %s | Opcode: %s | Arg1: %s | Arg2: %s | Arg3:%s | Arg4:%s\n", lLabel, lOpcode, lArg1, lArg2, lArg3, lArg4);
            
            char pcoffset[7] = "";
            if (!strcmp(lOpcode, ".orig")) {
                address = toNum(lArg1)-2;
                char orig_binary[MACHINE_CODE_SIZE+1];
                convertNumber(lArg1, MACHINE_CODE_SIZE, orig_binary);
                strcat(binaryLine, orig_binary);
            } else if (!strcmp(lOpcode, ".end")) {
                break;
            } else if (!strcmp(lOpcode, ".fill")) {
                char fill_binary[MACHINE_CODE_SIZE+1];
                convertNumber(lArg1, MACHINE_CODE_SIZE, fill_binary);
                strcat(binaryLine, fill_binary);
            } else if (!strcmp(lOpcode, "nop")) {
                strcat(binaryLine, "0000000000000000");
            } else if (!strcmp(lOpcode, "halt")) {
                TRAP("x25", binaryLine);
            } else if (!strcmp(lOpcode, "add")) {
                if (lArg3[0] == 'r') {
                    ADD1(lArg1, lArg2, lArg3, binaryLine);
                } else {
                    ADD2(lArg1, lArg2, lArg3, binaryLine);
                }
            } else if (!strcmp(lOpcode, "and")) {
                if (lArg3[0] == 'r') {
                    AND1(lArg1, lArg2, lArg3, binaryLine);
                } else {
                    AND2(lArg1, lArg2, lArg3, binaryLine);
                }
            } else if (!strcmp(lOpcode, "br") || !strcmp(lOpcode, "brnzp")) {
                findAddress(lArg1, address, pcoffset);
                BR(1, 1, 1, pcoffset, binaryLine);
            } else if (!strcmp(lOpcode, "brn")) {
                findAddress(lArg1, address, pcoffset);
                BR(1, 0, 0, pcoffset, binaryLine);
            } else if (!strcmp(lOpcode, "brz")) {
                findAddress(lArg1, address, pcoffset);
                BR(0, 1, 0, pcoffset, binaryLine);
            } else if (!strcmp(lOpcode, "brp")) {
                findAddress(lArg1, address, pcoffset);
                BR(0, 0, 1, pcoffset, binaryLine);
            } else if (!strcmp(lOpcode, "brnz")) {
                findAddress(lArg1, address, pcoffset);
                BR(1, 1, 0, pcoffset, binaryLine);
            } else if (!strcmp(lOpcode, "brzp")) {
                findAddress(lArg1, address, pcoffset);
                BR(0, 1, 1, pcoffset, binaryLine);
            } else if (!strcmp(lOpcode, "brnp")) {
                findAddress(lArg1, address, pcoffset);
                BR(1, 0, 1, pcoffset, binaryLine);
            } else if (!strcmp(lOpcode, "jmp")) {
                JMP(lArg1, binaryLine);
            } else if (!strcmp(lOpcode, "ret")) {
                RET(binaryLine);
            } else if (!strcmp(lOpcode, "jsr")) {
                findAddress(lArg1, address, pcoffset);
                JSR(pcoffset, binaryLine);
            } else if (!strcmp(lOpcode, "jsrr")) {
                JSRR(lArg1, binaryLine);
            } else if (!strcmp(lOpcode, "ldb")) {
                LDB(lArg1, lArg2, lArg3, binaryLine);
            } else if (!strcmp(lOpcode, "ldw")) {
                LDW(lArg1, lArg2, lArg3, binaryLine);
            } else if (!strcmp(lOpcode, "lea")) {
                findAddress(lArg2, address, pcoffset);
                LEA(lArg1, pcoffset, binaryLine);
            } else if (!strcmp(lOpcode, "rti")) {
                RTI(binaryLine);
            } else if (!strcmp(lOpcode, "lshf")) {
                LSHF(lArg1, lArg2, lArg3, binaryLine);
            } else if (!strcmp(lOpcode, "rshfl")) {
                RSHFL(lArg1, lArg2, lArg3, binaryLine);
            } else if (!strcmp(lOpcode, "rshfa")) {
                RSHFA(lArg1, lArg2, lArg3, binaryLine);
            } else if (!strcmp(lOpcode, "stb")) {
                STB(lArg1, lArg2, lArg3, binaryLine);
            } else if (!strcmp(lOpcode, "stw")) {
                STW(lArg1, lArg2, lArg3, binaryLine);
            } else if (!strcmp(lOpcode, "trap")) {
                TRAP(lArg1, binaryLine);
            } else if (!strcmp(lOpcode, "xor")) {
                if (lArg3[0] == 'r') {
                    XOR1(lArg1, lArg2, lArg3, binaryLine);
                } else {
                    XOR2(lArg1, lArg2, lArg3, binaryLine);
                }
            } else if (!strcmp(lOpcode, "not")) {
                NOT(lArg1, lArg2, binaryLine);
            } else {
                // printf("\tnot recognizable - %s\n", lLine);
            }
            // printf();
            // printf("%x: %s | %s\n", address, binaryLine, pcoffset);
            fprintf(pOutfile, "0x%.4X\n", strtol(binaryLine, NULL, 2));
            address+=2;
		}
	} while (lRet != DONE);
}

void findAddress(char* arg, int address, char* output) { // BR #2, x16, DONE -> outputs pc offset
    if ((arg[0] == '#') || (arg[0] == 'x')) { // if symbol is a number
        strcpy(output, arg);
    } else {
        for (int i = 0; i < numSymbols; i++) {
            TableEntry iStr = symbolTable[i];
            if (!strcmp(iStr.label, arg)) { // if symbol iis found in table
                int offset = (iStr.address - address)/2 - 1;
                char buffer[20];
                output[0] = '#';
                sprintf(buffer, "%d", offset);
                strcat(output, buffer);
            }
        }
    }
}

int main(int argc, char** argv) {
    assembler(argv[1], argv[2]);
    return 0;
}