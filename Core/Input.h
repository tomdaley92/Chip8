#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>

#define NUM_KEYS 16

/* There is a 4 ( and no 3), so each value 
can have its own bit slot */
#define CONTINUE 0
#define USER_QUIT 1
#define SOFT_RESET 2
#define LOAD_ROM 4 

/* Forward declaration */
class Display;

class Input {
    private:
        /* For processing window/keyboard events */
        SDL_Event event;
        const unsigned char *state;

        Display *display;

        int *cycles;
        bool *cpu_halt;
        bool *paused;

        int ProcessEvents();
        void ProcessKeys();

    public:
        Input();
        ~Input();

        /* Chip-8 HEX based keypad (0x0-0xF) */
        unsigned char keys[NUM_KEYS];

        /* For opcode 0xFX0A */
        bool awaiting_key_press;

        void Initialize(Display *display, 
                        int *cycles, 
                        bool *cpu_halt, 
                        bool *paused);
        void Reset();
        int Poll();
        
};

#endif
