#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

const int MAX_MEM_SIZE  = (1 << 13);

void fetchStage(int *icode, int *ifun, int *rA, int *rB, wordType *valC, wordType *valP) {
    // Get Program Counter // PC
    wordType PC = getPC();

    // Get icode by extracting the 1st 4 nibbles // icode
    *icode = (getByteFromMemory(PC) >> 4);

    // Get ifun by extracting the 2nd 4 nibbles using bitwise mask // ifun
    *ifun = (getByteFromMemory(PC) & 0xf);

    // Refer to comp-table images to understand groupings of common commands
    if ((*icode == RMMOVQ) || (*icode == MRMOVQ) || (*icode == PUSHQ) || (*icode == POPQ) || 
        (*icode == OPQ) || (*icode == RRMOVQ) || (*icode == IRMOVQ)) {
        
        // Get byte of memory at address PC + 1 // M1[PC + 1]
        byteType bytePC1 = getByteFromMemory(PC + 1);

        // First nibble of the PC1 byte
        wordType bytePart1 = bytePC1 >> 4; 

        // Second nibble of the PC1 byte
        wordType bytePart2 = bytePC1 & 0xf;  

        setRegister(*rA, bytePart1);
        setRegister(*rB, bytePart2);        
    }

    // Refer to comp-table images to understand groupings of common commands
    if ((*icode == RMMOVQ) || (*icode == MRMOVQ) || (*icode == IRMOVQ)) {
        // Get word from memory at address PC + 2 // M8[PC + 2]
        wordType wordPC1 = getWordFromMemory(PC + 2);

        // Set valC equal to the above word
        *valC = wordPC1;

        // Set valP equal to the counter + 10
        *valP = (PC + 10);
    }

    // Refer to comp-table images to understand groupings of common commands
    if ((*icode == PUSHQ) || (*icode == POPQ) || (*icode == OPQ) || (*icode == RRMOVQ)) {
        // Set valP equal to the counter + 2
        *valP = (PC + 2);
    }

    // Refer to comp-table images to understand groupings of common commands
    if ((*icode == JXX) || (*icode == CALL)) {
        // Get word from memory at address PC + 1 // M8[PC + 1]
        wordType wordPC1 = getWordFromMemory(PC + 1);

        // Set valC equal to the above word
        *valC = wordPC1;
        
        // Set valP equal to the counter + 9
        *valP = (PC + 9);
    }

    // Refer to comp-table images to understand groupings of common commands
    if ((*icode == NOP) || (*icode == HALT) || (*icode == RET)) {
        *valP = (PC + 1);
    }
}

void decodeStage(int icode, int rA, int rB, wordType *valA, wordType *valB) {
    // Refer to comp-table images to understand groupings of common commands
    if ((icode == RMMOVQ) || (icode == PUSHQ) || (icode == OPQ) || (icode == RRMOVQ)) {
        *valA = getRegister(rA);
    }

    // Refer to comp-table images to understand groupings of common commands
    if ((icode == RMMOVQ) || (icode == MRMOVQ) || (icode == OPQ)) {
        *valB = getRegister(rB);
    }

    // Refer to comp-table images to understand groupings of common commands
    if ((icode == PUSHQ) || (icode == POPQ) || (icode == CALL) || (icode == RET)) {
        *valB = getRegister(RSP);
    }

    // Refer to comp-table images to understand groupings of common commands
    if ((icode == POPQ) || (icode == RET)) {
        *valA = getRegister(RSP);
    }
}

void executeStage(int icode, int ifun, wordType valA, wordType valB, wordType valC, wordType *valE, bool *Cnd) {
    int bitsInByte = 8;

    // Refer to comp-table images to understand groupings of common commands
    if ((icode == RMMOVQ) || (icode == MRMOVQ)) {
        *valE = (valB + valC);
    }

    // Refer to comp-table images to understand groupings of common commands
    if ((icode == PUSHQ) || (icode == CALL)) {
        *valE = (valB + (-bitsInByte));
    }

    // Refer to comp-table images to understand groupings of common commands
    if ((icode == POPQ) || (icode == RET)) {
        *valE = (valB + bitsInByte); 
    }

    // Refer to comp-table images to understand groupings of common commands
    if (icode == OPQ) {
        // Perform Specified Operation
        if (ifun == ADD) { 
            *valE = valB + valA;     
        } else if (ifun == SUB) { 
            valA = -valA;
            *valE = valB + valA; 
        } else if (ifun == AND) { 
            valE = valB & valA; 
        } else if (ifun == XOR) { 
            *valE = valB ^ valA; 
        }         

        if (*valE = 0) {
            zeroFlag = 1;
        } 
    }


    // Refer to comp-table images to understand groupings of common commands
    if (icode == RRMOVQ) { *valE = valA; }

    // Refer to comp-table images to understand groupings of common commands
    if (icode == IRMOVQ) { *valE = valC; }

    // Refer to comp-table images to understand groupings of common commands
    if (icode == JXX) { *Cnd = Cond(ifun); }
}

void memoryStage(int icode, wordType valA, wordType valP, wordType valE, wordType *valM) {
    // Refer to comp-table images to understand groupings of common commands
    if ((icode == RMMOVQ) || (icode == PUSHQ)) {
        setWordInMemory(valE, valA);
    }

    // Refer to comp-table images to understand groupings of common commands
    if (icode == MRMOVQ) { *valM = getWordFromMemory(valE); }
    
    // Refer to comp-table images to understand groupings of common commands
    if ((icode == POPQ) || (icode == RET)) {
        *valM = getWordFromMemory(valA);
    }

    // Refer to comp-table images to understand groupings of common commands
    if (icode == CALL) { setWordInMemory(valE, valP); }
}

void writebackStage(int icode, int rA, int rB, wordType valE, wordType valM) {
    // Refer to comp-table images to understand groupings of common commands
    if ((icode == MRMOVQ) || (icode == POPQ)) {
        setRegister(rA, valM);
    }

    // Refer to comp-table images to understand groupings of common commands
    if ((icode == PUSHQ) || (icode == POPQ) || (icode == CALL) || (icode == RET)) {
        setRegister(RSP, valE);
    }

    // Refer to comp-table images to understand groupings of common commands
    if ((icode == OPQ) || (icode == RRMOVQ) || (icode == IRMOVQ)) {
        setRegister(rB, valE);
    }
}

void pcUpdateStage(int icode, wordType valC, wordType valP, bool Cnd, wordType valM) {
    // Refer to comp-table images to understand groupings of common commands
    if ((icode == NOP) || (icode == RMMOVQ) || (icode == MRMOVQ) || (icode == PUSHQ) || 
        (icode == POPQ) || (icode == OPQ) || (icode == RRMOVQ) || (icode == IRMOVQ)) {
        setPC(valP);
    }

    // Refer to comp-table images to understand groupings of common commands
    if (icode == HALT) {
        setPC(valP);
        setStatus(STAT_HLT);
    }

    // Refer to comp-table images to understand groupings of common commands
    if (icode == JXX) {
        setPC(Cnd ? valC : valP); 
    }

    // Refer to comp-table images to understand groupings of common commands
    if (icode == CALL) {
        setPC(valC); 
    }

    // Refer to comp-table images to understand groupings of common commands
    if (icode == RET) {
        setPC(valM); 
    }
}

void stepMachine(int stepMode) {
  /* FETCH STAGE */
  int icode = 0, ifun = 0;
  int rA = 0, rB = 0;
  wordType valC = 0;
  wordType valP = 0;
 
  /* DECODE STAGE */
  wordType valA = 0;
  wordType valB = 0;

  /* EXECUTE STAGE */
  wordType valE = 0;
  bool Cnd = 0;

  /* MEMORY STAGE */
  wordType valM = 0;

  fetchStage(&icode, &ifun, &rA, &rB, &valC, &valP);
  applyStageStepMode(stepMode, "Fetch", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);

  decodeStage(icode, rA, rB, &valA, &valB);
  applyStageStepMode(stepMode, "Decode", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);
  
  executeStage(icode, ifun, valA, valB, valC, &valE, &Cnd);
  applyStageStepMode(stepMode, "Execute", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);
  
  memoryStage(icode, valA, valP, valE, &valM);
  applyStageStepMode(stepMode, "Memory", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);
  
  writebackStage(icode, rA, rB, valE, valM);
  applyStageStepMode(stepMode, "Writeback", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);
  
  pcUpdateStage(icode, valC, valP, Cnd, valM);
  applyStageStepMode(stepMode, "PC update", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);

  incrementCycleCounter();
}

/** 
 * main
 * */
int main(int argc, char **argv) {
    int stepMode = 0;
    FILE *input = parseCommandLine(argc, argv, &stepMode);

    initializeMemory(MAX_MEM_SIZE);
    initializeRegisters();
    loadMemory(input);

    applyStepMode(stepMode);
    while (getStatus() != STAT_HLT) {
        stepMachine(stepMode);
        applyStepMode(stepMode);
    #ifdef DEBUG
        printMachineState();
        printf("\n");
    #endif
    }

    printMachineState();
    return 0;
}