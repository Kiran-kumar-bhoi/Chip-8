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
#define FONTSET_SIZE 80

#define WIDTH 64
#define HEIGHT 32
#define SCALE 15

const unsigned int START_ADDRESS = 0x200;
const unsigned int FONTSET_START_ADDRESS = 0x050;
bool SOUND = false;

const uint8_t fontset[80] =
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
uint8_t key[16];
uint16_t idx;
uint16_t pc;
uint16_t stack[16];
uint16_t opCode;
uint8_t vRam[32][64];

void draw_sprite(uint8_t x, uint8_t y, uint8_t n) {
    unsigned row = y, col = x;
    unsigned byte_index;
    unsigned bit_index;

    reg[0xF] = 0;
    for (byte_index = 0; byte_index < n; byte_index++) {
        uint8_t byte = ram[idx + byte_index];

        for (bit_index = 0; bit_index < 8; bit_index++) {
            uint8_t bit = (byte >> bit_index) & 0x1;

            uint8_t *pixelp = &vRam[(row + byte_index) % 32] [(col + (7 - bit_index)) % 64];

            if (bit == 1 && *pixelp ==1) reg[0xF] = 1;


            *pixelp = *pixelp ^ bit;
        }
    }
}
void chip8_loadgame(char *game) {
    FILE *fgame;

    fgame = fopen(game, "rb");

    if (NULL == fgame) {
        fprintf(stderr, "Unable to open game: %s\n", game);
        exit(42);
    }
    fseek(fgame, 0, SEEK_END);
    long size = ftell(fgame);
    rewind(fgame);
    if(size > (4096 - 0x200)){
        printf("ROM too large\n");
        exit(1);
    }

    fread(&ram[0x200], 1, size, fgame);

    fclose(fgame);
}

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
                    drawFlag = true;
                    pc+=2;
                    break;
                case 0xEE:
                    if(sp > 0 ){
                        pc = stack[--sp];
                    }else {
                        printf("stack underflow while executing %d",opCode);
                        exit(1);
                    }
                    break;
                default:
                    unknownOpCode(opCode);
            }
            break;

        case 0x1000://jump to nnn
            pc = nnn;
            break;

        case 0x2000://call to nnn
            if(sp < 16){
                stack[sp++] = pc + 2;
                pc = nnn;
            }else{
                printf("Stack overflow\n");
                exit(1);
            }
            break;

        case 0x3000://3xnn skip if reg[x] == nn
            pc += (reg[x] == nn) ? 4 : 2;
            break;

        case 0x4000://4xnn skip if reg[x] != nn
            pc += (reg[x] != nn) ? 4 : 2;
            break;

        case 0x5000://5xy0 skip if reg[x] == reg [y]
            if (n==0){
                pc += (reg[x]==reg[y]) ? 4: 2;
            }else{
                unknownOpCode(opCode);
            }
            break;

        case 0x6000://6xnn set reg[x] = nn
            reg[x] = nn;
            pc += 2;
            break;

        case 0x7000://7xnn reg[x]+= nn
            reg[x] += nn;
            pc += 2;
            break;

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
            if (n==0){
                pc += (reg[x]!=reg[y]) ? 4: 2;
            }else {
                unknownOpCode(opCode);
            }
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
            draw_sprite(reg[x],reg[y],n);
            drawFlag = true;
            pc+=2;
            break;

        case 0xE000://Exnn check for input
            switch(nn){
                case 0x9E://skip if reg[x] is pressed
                    pc += (key[reg[x]]) ? 4 : 2;
                    break;

                case 0xA1://skip if reg[x] is not pressed
                    pc += (!key[reg[x]]) ? 4 : 2;
                    break;

                default:
                    unknownOpCode(opCode);
            }
            break;

        case 0xF000://Fxnn timer and stuff
            switch (nn) {
                case 0x07://set reg[x] as delaytimer
                    reg[x] = delayTimer;
                    pc += 2;
                    break;

                case 0x0A://wait for keypress
                    {
                    bool keyPressed = false;
                    for (int i = 0; i < 16; i++) {
                        if (key[i]) {
                            reg[x] = i;
                            keyPressed = true;
                            break;
                        }
                    }
                    // only advance if a key was pressed
                    if (keyPressed) {
                        pc += 2;
                    }
                    break;
                    }

                case 0x15://set delaytimer as reg[x]
                    delayTimer = reg[x];
                    pc += 2;
                    break;

                case 0x1E://adds reg[x] to idx
                    reg[0xF] = (idx + reg[x] > 0xFFF) ? 1 : 0;
                    idx += reg[x];
                    pc += 2;
                    break;

                case 0x18://set soundtimer as reg[x]
                    soundTimer = reg[x];
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
    memset(reg, 0, sizeof(reg));
    memset(ram, 0, sizeof(ram));
    memset(stack, 0, sizeof(stack));
    memset(vRam, 0, sizeof(vRam));
    memset(key, 0, sizeof(key));

    //loads the fonts
	for (int i = 0; i < FONTSET_SIZE; ++i)
	{
		ram[FONTSET_START_ADDRESS + i] = fontset[i];
	}

	//links the clock as random seed
	srand((unsigned int)time(NULL));
}


int main(int argc, char* argv[]) {

    // initialize chip8
    init();

    if (argc < 2) {
        printf("Usage: %s romfile\n", argv[0]);
        return 1;
    }

    // load ROM
    chip8_loadgame(argv[1]);

    // initialize SDL
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    SDL_Window* window = SDL_CreateWindow(
        "CHIP-8 Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIDTH * SCALE,
        HEIGHT * SCALE,
        SDL_WINDOW_SHOWN
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );

    SDL_RenderSetLogicalSize(renderer, WIDTH, HEIGHT);

    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH,
        HEIGHT
    );
    uint32_t pixels[WIDTH * HEIGHT];

    bool quit = false;
    SDL_Event e;

    while (!quit) {

        while (SDL_PollEvent(&e)) {

            if (e.type == SDL_QUIT) {
                quit = true;
            }

            if (e.type == SDL_KEYDOWN) {

                switch (e.key.keysym.sym) {

                    case SDLK_1: key[0x1] = 1; break;
                    case SDLK_2: key[0x2] = 1; break;
                    case SDLK_3: key[0x3] = 1; break;
                    case SDLK_4: key[0xC] = 1; break;

                    case SDLK_q: key[0x4] = 1; break;
                    case SDLK_w: key[0x5] = 1; break;
                    case SDLK_e: key[0x6] = 1; break;
                    case SDLK_r: key[0xD] = 1; break;

                    case SDLK_a: key[0x7] = 1; break;
                    case SDLK_s: key[0x8] = 1; break;
                    case SDLK_d: key[0x9] = 1; break;
                    case SDLK_f: key[0xE] = 1; break;

                    case SDLK_z: key[0xA] = 1; break;
                    case SDLK_x: key[0x0] = 1; break;
                    case SDLK_c: key[0xB] = 1; break;
                    case SDLK_v: key[0xF] = 1; break;
                }
            }

            if (e.type == SDL_KEYUP) {

                switch (e.key.keysym.sym) {

                    case SDLK_1: key[0x1] = 0; break;
                    case SDLK_2: key[0x2] = 0; break;
                    case SDLK_3: key[0x3] = 0; break;
                    case SDLK_4: key[0xC] = 0; break;

                    case SDLK_q: key[0x4] = 0; break;
                    case SDLK_w: key[0x5] = 0; break;
                    case SDLK_e: key[0x6] = 0; break;
                    case SDLK_r: key[0xD] = 0; break;

                    case SDLK_a: key[0x7] = 0; break;
                    case SDLK_s: key[0x8] = 0; break;
                    case SDLK_d: key[0x9] = 0; break;
                    case SDLK_f: key[0xE] = 0; break;

                    case SDLK_z: key[0xA] = 0; break;
                    case SDLK_x: key[0x0] = 0; break;
                    case SDLK_c: key[0xB] = 0; break;
                    case SDLK_v: key[0xF] = 0; break;
                }
            }
        }


        for (int i = 0; i < 10; i++) {
            execute();
        }

        chipTick();


        if (drawFlag) {

            for (int y = 0; y < HEIGHT; y++) {

                for (int x = 0; x < WIDTH; x++) {

                    if (vRam[y][x]) {
                        pixels[x + y * WIDTH] = 0xFFFFFFFF;
                    } else {
                        pixels[x + y * WIDTH] = 0xFF000000;
                    }
                }
            }

            SDL_UpdateTexture(
                texture,
                NULL,
                pixels,
                WIDTH * sizeof(uint32_t)
            );

            SDL_RenderClear(renderer);

            SDL_RenderCopy(renderer, texture, NULL, NULL);

            SDL_RenderPresent(renderer);

            drawFlag = false;
        }

        SDL_Delay(16);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
