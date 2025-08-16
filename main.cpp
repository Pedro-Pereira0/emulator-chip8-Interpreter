//#include
//#include OpenGL graphics and input
#include "chip8.h"

chip8 myChip8;

int main(int argc, char **argv){

    // Set up render system and register input callbacks
    myChip8.initialize();
    myChip8.loadGame("pong");

    // Emulation loop
    for(;;)
    {     // Emulate one cycle
        myChip8.emulateCycle();

    // If the draw flag is set, update the screen
        if(myChip8.drawFlag)
            drawGraphics();

    // Store key press state (Press and Release)
        myChip8.setKeys();	
    }

    return 0;
}

    void drawGraphics(){}