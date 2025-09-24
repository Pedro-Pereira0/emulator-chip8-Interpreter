#include <string.h>
#include <string>

class chip8{

    public:
        //The graphics of the Chip 8 are black and white and the screen has a total of 2048 pixels (64 * 32)
        //Pixel state 1 or 0
        //Drawing is done with XOR operations
        unsigned char gfx[64 * 32];

        //keypad
        unsigned char key[16];

        chip8();
		~chip8();

        bool drawFlag;
        bool loadGame(const char* gameName);
        void emulateCycle();
        void debugRender();
    
    private:
        //chip-8 system
        //35 operation codes 
        unsigned short opcode;
        //4gb of memory
        unsigned char memory[4096];
        //15 8-bit registers - 16 used has carry-flag
        unsigned char V[16];
        //Index Register
        unsigned short I;
        //PC - program counter
        unsigned short pc;


        //Two timers that count at 60Hz, when the counter is bigger than 0, the counter, counts down to 0.
        //When the sound timer goes to 0, the system buzzes
        unsigned char delay_timer;
        unsigned char sound_timer;

        //The stack is used to remember the current location before a jump, or a subroutine call is performed
        unsigned short stack[16];
        //Pointer sp is used to remember the current level of the stack
        unsigned short sp;

  
        void initialize();
};