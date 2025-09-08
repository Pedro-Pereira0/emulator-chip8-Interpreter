#include "chip8.h"

class chip8{
    //chip-8 system
    //35 operation codes, 2bytes long 
    unsigned short opcode;
    //4gb of memory
    unsigned char memory[4096];
    //15 8-bit registers - 16 usado como carry-flag
    unsigned char V[16];
    //Index Register
    unsigned short I;
    //PC - program counter
    unsigned short pc;

    //The graphics of the Chip 8 are black and white and the screen has a total of 2048 pixels (64 * 32)
    //Pixel state 1 or 0
    //Drawing is done with XOR operations
    unsigned char gfx[64 * 32];

    //Two timers that count at 60Hz, when the counter is bigger than 0, the counter, counts down to 0.
    //When the sound timer goes to 0, the system buzzes
    unsigned char delay_timer;
    unsigned char sound_timer;

    //The stack is used to remember the current location before a jump, or a subroutine call is performed
    unsigned short stack[16];
    //Pointer sp is used to remember the current level of the stack
    unsigned short sp;

    //keypad
    unsigned char key[16];

    unsigned char chip8_fontset[80] =
    { 
        0xF0, 0x90, 0x90, 0x90, 0xF0, //0
        0x20, 0x60, 0x20, 0x20, 0x70, //1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
        0x90, 0x90, 0xF0, 0x10, 0x10, //4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
        0xF0, 0x10, 0x20, 0x40, 0x40, //7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
        0xF0, 0x90, 0xF0, 0x90, 0x90, //A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
        0xF0, 0x80, 0x80, 0x80, 0xF0, //C
        0xE0, 0x90, 0x90, 0x90, 0xE0, //D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
        0xF0, 0x80, 0xF0, 0x80, 0x80  //F
    };


    
    public:
        bool drawFlag;

        void initialize(){
            // Initialize registers and memory once
            pc     = 0x200;  // Program counter starts at 0x200
            opcode = 0;      // Reset current opcode	
            I      = 0;      // Reset index register
            sp     = 0;      // Reset stack pointer
            
            // Clear display
            for(int i = 0; i < 2048; ++i)
                gfx[i] = 0;

            // Clear stack
            for(int i = 0; i < 16; ++i)
                stack[i] = 0;

            for(int i = 0; i < 16; ++i)
                key[i] = V[i] = 0;

            // Clear memory
            for(int i = 0; i < 4096; ++i)
                memory[i] = 0;

            // Load fontset
            for(int i = 0; i < 80; ++i){
                memory[i] = chip8_fontset[i];		
            }
            // Reset timers
            delay_timer = 0;
            sound_timer = 0;
        }

        bool loadGame(char* gameName){

            printf("Loading %s\n", gameName);
            //Opening the application as binary
            FILE* fp = fopen(gameName, "rb");
            if(fp == NULL)
            {
                fputs("File error", stderr);
                return false;
            }

            //Check file size
            fseek(fp , 0 , SEEK_END);
            long lSize = ftell(fp);
            rewind(fp);
            printf("Filesize: %d\n", (int)lSize);

            //Allocate the memory to contain the whole file
            char* buffer = (char*)malloc(sizeof(char) * lSize);
            if (buffer == NULL){
                fputs("Memory error", stderr);
                return false;
            }
            
            //Copy the file into the buffer
            size_t result = fread(buffer, 1, lSize, fp);
            if (result != lSize) 
            {
                fputs("Reading error",stderr); 
                return false;
            }

            //Copy buffer to Chip8 memory
            if((4096-512) > lSize)
            {
                for(int i = 0; i < lSize; ++i)
                    memory[i + 512] = buffer[i];
            }
            else
                printf("Error: ROM too big for memory");
            
            // Close file, free buffer
            fclose(fp);
            free(buffer);

            return true;
        }
        void setKeys(){

        }
        //Emulates one emulation cycle, where an opcode is fetch, decoded and executed
        void emulateCycle(){
            // Fetch Opcode
            //Each opcode is 2 bytes, which means the code in memory is in two positions pc and pc+1.
            //We bit shift the first code 8 bits, and complete it by doing an OR operation with the 2nd memory position and the 0s
            //of the bitshift.
            opcode = memory[pc] << 8 | memory[pc + 1];opcode = memory[pc] << 8 | memory[pc + 1];
            // Decode Opcode
            //we need to decode the opcode and check the opcode table to see what it means.

            // Execute Opcode
            
            //For the ANNN, we have to store in the I(Index Register), the value of NNN so we remove the first 4bits, with an AND operation with 0x0FFF (0000 1111 1111 1111)
            I = opcode & 0x0FFF;
            //Because each code is 2 bytes the next code will be two positions forward
            pc += 2;

            // Update timers
            //Emulation cycles has to execute 60 codes per second (60Hz) for the timers to work correctly.
        }
};