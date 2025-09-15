#include "chip8.h"
#include <iostream>

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
    
    chip8::chip8(){}

    chip8::~chip8(){}
    

    void chip8::initialize(){
        
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

    bool chip8::loadGame(const char* gameName){

        initialize();

        std::cout<<"Loading %s\n", gameName;

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
        std::cout<<"Filesize: %d\n", (int)lSize;

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
        //The CHIP-8 interpreter itself occupies the first 512 bytes of the memory space on these machines.
        if((4096-512) > lSize)
        {
            for(int i = 0; i < lSize; ++i)
                memory[i + 512] = buffer[i];
        }
        else
            std::cout << "Error: ROM too big for memory";
        
        // Close file, free buffer
        fclose(fp);
        free(buffer);

        return true;
    }

    void chip8::setKeys(){

    }

    //Emulates one emulation cycle, where an opcode is fetch, decoded and executed
    void chip8::emulateCycle(){
        
        // Fetch Opcode
        //Each opcode is 2 bytes, which means the opcode in memory is in two positions pc and pc+1.
        //We bit shift the first opcode 8 bits, and complete it by doing an OR operation with the 2nd memory position and the 0s
        //of the bitshift.
        opcode = memory[pc] << 8 | memory[pc + 1];

        // Decode Opcode
        //we need to decode the opcode and check the opcode table to see what it means.
        //First we get the first 4bits of the code
        switch(opcode & 0xF000)
        {    
            case 0xA000: // ANNN: Sets I to the address NNN
                // Execute opcode
                //For the ANNN, we have to store in the I(Index Register), 
                //the value of NNN so we remove the first 4bits, with an AND operation with 0x0FFF (0000 1111 1111 1111)
                I = opcode & 0x0FFF;

                //Because each code is 2 bytes the next code will be two positions forward
                pc += 2;
            break;
        
            case 0xB000: //BNNN: Jumps to the address NNN plus V0
                pc = V[0] + (opcode & 0x0FFF);
            break;

            case 0xC000: //Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
                V[(opcode & 0x0F00) >> 8] = (rand() % 256) & (opcode & 0x00FF);
                pc += 2;
            break;
            
            case 0xD000: //DXYN
                /*Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. 
                Each row of 8 pixels is read as bit-coded starting from memory location I; 
                I value does not change after the execution of this instruction. 
                As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that does not happen.*/

                unsigned short x = V[(opcode & 0x0F00) >> 8];
                unsigned short y = V[(opcode & 0x00F0) >> 4];
                unsigned short n = V[(opcode & 0x000F)];

                unsigned short width = 8;

                unsigned short pixel;

                V[0xF] = 0;

                for(int yLine = 0; yLine < n; yLine++){
                    //Gets the pixel value at each Y axis position
                    pixel = memory[I + yLine];

                    for(int xLine = 0; xLine < width; xLine++){

                        //Checks if the pixel value, along the X axis, is activated or not, 0x80 is 10000000, for each iteration the 1 will bitshift to the right.
                        //This way, it will go through each bit value of the pixel. If its activated, there wont be any need to activate it, otherwise, it will toggle the pixel.
                        //If the pixel goes from set to unset (1 to 0), then VF is set to 1, meaning there was a collision.
                        //The pixel value is set with a xor operation on 1.

                        if((pixel & (0x80 >> xLine)) != 0){
                            
                            //If the pixel is flipped from 1 to 0
                            if(gfx[(x + xLine + ((y + yLine) * 64))] == 1)
                                V[0xF] = 1;       

                            //Set the pixel value
                            gfx[x + xLine + ((y + yLine) * 64)] ^= 1;
                        }
                    }
                }

                drawFlag = true;
                pc += 2;
            
            break;

            case 0xE000:
                switch(opcode & 0x00FF){
                    case 0x009E: //Skips the next instruction if the key stored in VX
                    //(only consider the lowest nibble) is pressed (usually the next instruction is a jump to skip a code block).
                        if(key[V[(opcode & 0x0F00) >> 8]] != 0){
                            pc += 4;
                        }else{
                            pc += 2;
                        }
                    break;

                    case 0x00A1: //Skips the next instruction if the key stored in VX(only consider the lowest nibble) 
                                //is not pressed (usually the next instruction is a jump to skip a code block).
                        if(key[V[(opcode & 0x0F00) >> 8]] == 0){
                            pc += 4;
                        }else{
                            pc += 2;
                        }

                    break;
                }

            break;

            case 0xF000:
                switch(opcode & 0x00FF){
                    case 0x0007: //Sets VX to the value of the delay timer.
                        V[(opcode & 0x0F00) >> 8] = delay_timer;
                        pc += 2;
                    break;

                    case 0x000A: //A key press is awaited, and then stored in VX.
                        
                        //Checks each key space
                        bool keyPress = false;
                        for(int i = 0; i<16; i++){
                            if(key[i] != 0){
                                V[(opcode & 0x0F00) >> 8] = i;
                                keyPress = true;
                            }
                        }
                        //If no key is pressed will return and retry, the pc doesnt advance, so it doesnt skip to the next instruction
                        if(!keyPress){
                            return;
                        }

                        pc += 2;
                    break;

                    case 0x0015: //Sets the delay timer to VX.
                        delay_timer = V[(opcode & 0x0F00) >> 8];
                        pc += 2;
                    break;

                    case 0x0018: //Sets the sound timer to VX.

                        sound_timer = V[(opcode & 0x0F00) >> 8];
                        pc += 2;
                    break;

                    case 0x001E: //Adds VX to I. VF is not affected.
                        I+=V[(opcode & 0x0F00) >> 8];
                        pc += 2;
                    break;

                    case 0x0029: //Sets I to the location of the sprite for the character in VX(only consider the lowest nibble). 
                                //Characters 0-F (in hexadecimal) are represented by a 4x5 font.
                                //0x5??
                        I = V[(opcode & 0x0F00) >> 8] * 0x5;
                        pc += 2;
                    break;

                    case 0x0033: //Stores the binary-coded decimal representation of VX, 
                                //with the hundreds digit in memory at location in I, the tens digit at location I+1, 
                                //and the ones digit at location I+2.

                        memory[I] = V[(opcode & 0x0F00) >> 8] / 100; //567: 567 / 100 = 5
                        memory[I+1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10; //567: 56 -> mod 10 = 6
                        memory[I+2] = V[(opcode & 0x0F00) >> 8] % 10;

                        pc += 2;

                    break;

                    case 0x0055: //Stores from V0 to VX (including VX) in memory, starting at address I. 
                                //The offset from I is increased by 1 for each value written, but I itself is left unmodified.
                        int vx = (opcode & 0x0F00) >> 8;
                        for(int i = 0; i <= vx; i++){
                            memory[I + i] = V[i];
                        }

                        pc += 2;
                    break;

                    case 0x0065://Fills from V0 to VX (including VX) with values from memory, starting at address I. 
                                //The offset from I is increased by 1 for each value read, but I itself is left unmodified.
                        
                        int vx = (opcode & 0x0F00) >> 8;
                        for(int i = 0; i <= vx; i++){
                            V[i] = memory[I + i];
                        }

                        pc += 2; 
                    break;

                }
            break;

            case 0x0000:
                switch(opcode & 0x00FF){
                    case 0x0000: //Calls machine code routine (RCA 1802 for COSMAC VIP) at address NNN. 
                                //Not necessary for most ROMs.

                        pc = opcode & 0x0FFF;

                    break;

                    case 0x00E0: //Clears the screen.
                        for(int i = 0; i < 2048; ++i)
                            gfx[i] = 0;

                        drawFlag = true;

                        pc += 2;

                    break;

                    case 0x00EE: //Returns from a subroutine.
                        --sp;			// 16 levels of stack, decrease stack pointer to prevent overwrite
                        pc = stack[sp];	// Put the stored return address from the stack back into the program counter					
                        pc += 2;
                    break;
                }
            break;

            case 0x1000: //Jumps to address NNN.
                pc = opcode & 0x0FFF;
            break;

            case 0x2000: //Calls subroutine at NNN.
                //Temporary call, save the program counter in the stack
                stack[sp] = pc;
                //increment the stack pointer, so the program counter isn't overwritten
                ++sp;
                //Set the program counter to the address NNN
                pc = opcode & 0x0FFF;
                //Because we are calling at a specific address, we don't increment pc
            break;

            case 0x3000: //Skips the next instruction if VX equals NN (usually the next instruction is a jump to skip a code block).
                if(V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
                    pc += 4;
                else
                    pc += 2;
            break;

            case 0x4000: //Skips the next instruction if VX does not equal NN (usually the next instruction is a jump to skip a code block).
                if(V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
                    pc += 4;
                else
                    pc += 2;
            break;

            case 0x5000: //Skips the next instruction if VX equals VY (usually the next instruction is a jump to skip a code block).
                if(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
                    pc += 4;
                else
                    pc += 2;
            break;

            case 0x6000: //Sets VX to NN.
                V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
                pc += 2;
            break;

            case 0x7000: //Adds NN to VX (carry flag is not changed).
                V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
                pc += 2;
            break;

            case 0x8000:
                switch(opcode & 0x000F){
                    case 0x0000: //Sets VX to the value of VY.
                        V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
                        pc += 2;
                    break;

                    case 0x0001: //Sets VX to VX or VY. (bitwise OR operation).
                        V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] | V[(opcode & 0x00F0) >> 4];
                        pc += 2;
                    break;

                    case 0x0002: //Sets VX to VX and VY. (bitwise AND operation).
                         V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] & V[(opcode & 0x00F0) >> 4];
                        pc += 2;
                    break;

                    case 0x0003: //	Sets VX to VX xor VY.
                        V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] ^ V[(opcode & 0x00F0) >> 4];
                        pc += 2;
                    break;

                    case 0x0004: //Adds VY to VX
                        //Checks if there is an overflow. For the code 0x8XY4:
                        //We see if V[Y] is bigger, than the capacity left to reach 255. if V[Y] is bigger than there is an overflow. 
                        if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
                            //Position 15, used has carry-flag
                            V[0xF] = 1; //carry
                        else
                            V[0xF] = 0;

                        //Adds VY to VX
                        V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
                        pc += 2;
                    break;

                    case 0x0005: //VY is subtracted from VX. 
                                //VF is set to 0 when there's an underflow, and 1 when there is not.
                        //Underflow is when a subtraction results in a negative number. So we check if VY is bigger than VX
                        if(V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
                            V[0xF] = 0;
                        else
                            V[0xF] = 1;

                        V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] - V[(opcode & 0x00F0) >> 4];

                        pc += 2;

                    break;

                    case 0x0006: //Shifts VX to the right by 1, then stores the least significant bit of VX prior to the shift into VF.
                        //Last bit least significant. Bitwise AND with 1
                        V[0xF] = V[(opcode & 0x0F00) >> 8] & 1;
                        V[(opcode & 0x0F00) >> 8] >>= 1;

                        pc += 2;
                    break;

                    case 0x0007: //Sets VX to VY minus VX. 
                                //VF is set to 0 when there's an underflow, and 1 when there is not. (i.e. VF set to 1 if VY >= VX).
                        if(V[(opcode & 0x0F00) >> 8] < V[(opcode & 0x00F0) >> 4])
                            V[0xF] = 0; //underflow
                        else
                            V[0xF] = 1;

                        V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
                    break;
                }
            break;

            case 0x9000: //Skips the next instruction if VX does not equal VY.
                if(V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
                    pc += 4; //Skips the next instruction
                else
                    pc += 2; //Goes to the next instruction
            break;
        
            default:
            printf ("Unknown opcode: 0x%X\n", opcode);
        }  

        // Update timers
        //Emulation cycles has to execute 60 codes per second (60Hz) for the timers to work correctly.
        // Update timers
        if(delay_timer > 0)
            --delay_timer;
        
        if(sound_timer > 0)
        {
            if(sound_timer == 1)
            printf("BEEP!\n");
            --sound_timer;
        } 
    }