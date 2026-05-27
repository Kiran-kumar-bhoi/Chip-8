//author @Kiran-kumar-bhoi
#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#define FONTSET_ADDRESS 0x00
#define FONTSET_BYTES_PER_CHAR 5

const unsigned int START_ADDRESS = 0x200;
const unsigned int FONTSET_START_ADDRESS = 0x050;
const unsigned int FONTSET_SIZE = 80;
bool SOUND = false;

uint8_t fontset[FONTSET_SIZE] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};
uint8_t randByte(){

    int random_num = rand() % 256;
    return random_num ;
}

bool drawFlag;
uint8_t reg[16];
uint8_t ram[4096];
uint8_t sp;
uint8_t delayTimer;
uint8_t soundTimer;
uint8_t input[16];
uint16_t idx;
uint16_t pc;
uint16_t stack[16];
uint16_t opCode;
uint32_t vRam[32*64];

void unknownOpCode(uint16_t opCode){
    printf("Unknown Opcode : 0x%x",opCode);
    exit(1);
}

void execute(){
    uint8_t x,y,n,nn;
    uint16_t nnn;

    //fetching the data
    opCode = ram[pc]<<8 | ram[pc+1];
    x      = opCode>>8  & 0x000F;
    y      = opCode>>4  & 0x000F;
    n      = opCode     & 0x000F;
    nn     = opCode     & 0x00FF;
    nnn    = opCode     & 0x0FFF;

    switch (opCode & 0xF000){
        case 0x0000://clr and ret
            switch (nn) {
                case 0xE0://clear the screen
                    memset(vRam, 0, sizeof(vRam));
                    pc+=2;
                    break;
                case 0xEE:
                    pc = stack[--sp];
                    break;
                default:
                    unknownOpCode(opCode);
            }
            break;

        case 0x1000://jump to nnn
            pc = nnn;
            break;

        case 0x2000://call to nnn
            stack[sp++] = pc+2;
            pc = nnn;
            break;

        case 0x3000://3xnn skip if reg[x] == nn
            pc += (reg[x] == nn) ? 4 : 2;
            break;

        case 0x4000://4xnn skip if reg[x] != nn
            pc += (reg[x] != nn) ? 4 : 2;
            break;

        case 0x5000://5xy0 skip if reg[x] == reg [y]
            pc += (reg[x]==reg[y]) ? 4: 2;
            break;

        case 0x6000://6xnn set reg[x] = nn
            reg[x] = nn;
            pc += 2;
            break;

        case 0x7000://7xnn reg[x]+= nn
            reg[x] += nn;
            pc += 2;

        case 0x8000://8xyn arithmatic operations
            switch(n){
                case 0x0:// reg[x] = reg[y]
                    reg[x] = reg[y];
                    break;

                case 0x1:// or
                    reg[x] = reg[x] | reg[y];
                    break;

                case 0x2:// and
                    reg[x] = reg[x] & reg[y];
                    break;

                case 0x3:// xor
                    reg[x] = reg[x] ^ reg[y];
                    break;

                case 0x4:// add
                    reg[0xF] = ((int)reg[x]+(int)reg[y]) > 255 ? 1 : 0;
                    reg[x] = reg[x] + reg[y];
                    break;

                case 0x5:// subtract x-y
                    reg[0xF] = (reg[x] > reg[y]) ? 1 : 0;
                    reg[x] = reg[x] - reg[y];
                    break;

                case 0x6:// rightshift
                    reg[0xF] = reg[x] & 0x01;
                    reg[x] = reg[x]>>1;
                    break;

                case 0x7:// subtract y-x
                    reg[0xF] = (reg[x] < reg[y]) ? 1 : 0;
                    reg[x] = reg[y] - reg[x];
                    break;

                case 0xE:// leftshift
                    reg[0xF] = (reg[x]>>7) & 0x01;
                    reg[x] = reg[x]<<1;
                    break;

                default:
                    unknownOpCode(opCode);

            }
            pc += 2;
            break;

        case 0x9000://9xy0 skip is reg[x] != reg[y]
            pc += (reg[x]!=reg[y]) ? 4: 2;
            break;

        case 0xA000://Annn sets Idx to nnn
            idx = nnn;
            pc += 2;
            break;

        case 0xB000://jump to nnn + reg[0]
            pc = nnn + reg[0];
            break;

        case 0xC000://Cxnn reg[x] = nn & random
            reg[x] = nn & randByte();
            pc += 2;
            break;

        case 0xD000://Dxyn display nByte string from idx at (reg[x],reg[y])

        case 0xE000://Exnn check for input
            switch(nn){
                case 0x9E://skip if reg[x] is pressed
                    pc += (key(reg[x])) ? 4 : 2;
                    break;

                case 0xA1://skip if reg[x] is not pressed
                    pc += (!key(reg[x])) ? 4 : 2;
                    break;

                default:
                    unknownOpCode(opCode);
            }
            break;

        case 0xF000://Fxnn timer and stuff
            switch (nn) {
                case 0x07://set delaytimer as reg[x]
                    delayTimer = reg[x];
                    pc += 2;
                    break;

                case 0x15://set soundtimer as reg[x]
                    soundTimer = reg[x];
                    pc += 2;
                    break;

                case 0x1E://adds reg[x] to idx
                    reg[0xF] = (idx + reg[x] > 0xFFF) ? 1 : 0;
                    idx += reg[x];
                    pc += 2;
                    break;

                case 0x29://loads fontsize
                    idx = FONTSET_BYTES_PER_CHAR * reg[x];
                    pc += 2;
                    break;

                case 0x33://loads BCD of reg[x] into idx
                    ram[idx]   = (reg[x] % 1000) / 100;
                    ram[idx+1] = (reg[x] % 100) / 10;
                    ram[idx+2] = (reg[x] % 10);
                    pc += 2;
                    break;

                case 0x55://copy sprite from reg into ram
                    for(int i = 0; i<=x; i++){
                        ram[i+idx] = reg[i];
                    }
                    idx += x + 1;
                    pc += 2;
                    break;

                case 0x65://copy sprite form ram into reg
                    for(int i = 0; i<=x; i++){
                         reg[i] = ram[i+idx];
                    }
                    idx += x + 1;
                    pc += 2;
                    break;

                default:
                    unknownOpCode(opCode);
            }
            break;

        default:
            unknownOpCode(opCode);

    }
}

void generateSound(bool flag){
    if(flag){SOUND = true;}
    else{SOUND = false;}
}

void chipTick(){
    if (delayTimer>0){
        --delayTimer;
    }
    if (soundTimer>0){
        --soundTimer;
        generateSound(true);
    }else{
        generateSound(false);
    }
}



void init(){

    //sets the pc
    pc = START_ADDRESS;

    //loads the fonts
	for (int i = 0; i < FONTSET_SIZE; ++i)
	{
		ram[FONTSET_START_ADDRESS + i] = fontset[i];
	}

	//links the clock as random seed
	srand((unsigned int)time(NULL));

}

int main(){

}
