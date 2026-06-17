CHIP-8 Emulator

A lightweight and functional CHIP-8 emulator written in C. This project uses the SDL2 library to handle window creation, graphics rendering, and keyboard input.
📂 Directory Structure

The project is organized as follows:
Plaintext

📦 chip-8-emulator
 ┣ 📂 headers
 ┃ ┗ 📜 chip8.h        # Constants, fontset, and configurations
 ┣ 📂 src
 ┃ ┗ 📜 chip8.c        # Main emulator logic, CPU instructions, and SDL2 integration
 ┣ 📂 testRoms         # Directory for test ROMs
 ┃ ┗ 📜 <romname>.ch8  
 ┗ 📂 roms             # Directory for playable game ROMs
   ┗ 📜 roms.ch8       

🛠️ Prerequisites


To compile and run this emulator, you will need:

    A C compiler (like gcc or clang)

    SDL2 Development Libraries installed on your system.

Installing SDL2:

    Debian/Ubuntu: sudo apt-get install libsdl2-dev

    macOS (Homebrew): brew install sdl2

    Windows: Download the development libraries from the SDL website or use MSYS2/MinGW.

🚀 Building and Running

1. Compile the Source Code
Navigate to the root directory of your project and compile the source file. Ensure you link the SDL2 library and include the headers folder:
Bash

gcc src/chip8.c -o chip8 -I./headers -lSDL2

2. Run the Emulator
Execute the compiled binary and pass the path to a .ch8 ROM file as an argument:
Bash

./chip8 roms/roms.ch8

⌨️ Keyboard Mapping

The original CHIP-8 used a 16-key hexadecimal keypad (0-F). This emulator maps those keys to a standard QWERTY keyboard as follows:
Original CHIP-8 Keypad	Mapped QWERTY Keys
        1 2 3 C	            1 2 3 4
        4 5 6 D	            Q W E R
        7 8 9 E	            A S D F
        A 0 B F	            Z X C V
⚙️ Features

    Complete Instruction Set: Implements all standard CHIP-8 opcodes.

    Display Rendering: Accurately emulates the 64x32 monochrome display, scaled up by a factor of 15 (configurable in chip8.h) for modern screens.

    Timers: Supports both the 60Hz Delay Timer and Sound Timer.

    Clock Speed: Executes 10 instructions per frame to provide a smooth emulation speed.

👤 Author

    Kiran-kumar-bhoi
