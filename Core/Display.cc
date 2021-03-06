#include "Display.h"
#include "Gui.h"
#include <SDL2/SDL_opengl.h>
#include <stdio.h>

Display::Display(){
    WINDOW_WIDTH = WIDTH * (int)SCALE;
    WINDOW_HEIGHT = HEIGHT * (int)SCALE;
    back_buffer = NULL;
    window = NULL;
    fullscreen_flag = 0;
    vsync_flag = 0;
    limit_fps_flag = 1;
    lost_window_focus = 0;

    /* set rendering colors */
    background_color[0] = (float) DEFAULT_BACKGROUND_R / (float) 0xFF;
    background_color[1] = (float) DEFAULT_BACKGROUND_G / (float) 0xFF;
    background_color[2] = (float) DEFAULT_BACKGROUND_B / (float) 0xFF;

    foreground_color[0] = (float) DEFAULT_FOREGROUND_R / (float) 0xFF;
    foreground_color[1] = (float) DEFAULT_FOREGROUND_G / (float) 0xFF;
    foreground_color[2] = (float) DEFAULT_FOREGROUND_B / (float) 0xFF;
}

Display::~Display(){
    /* clean-up */
    if (back_buffer) {
        for (int i = 0; i < WIDTH; i++) {
            free(back_buffer[i]);
        }
        free(back_buffer);
    }
    gui.~Gui();
    SDL_GL_DeleteContext(glcontext);
    SDL_DestroyWindow(window);
}
    

int Display::Initialize( 
    bool fullscreen,
    int *steps,
    bool *paused,
    bool *load_store_quirk,
    bool *shift_quirk, 
    bool *vwrap,
    bool *muted
) {

    int window_mode = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

    /* init the backbuffer */
    back_buffer = (unsigned char **) malloc(WIDTH * sizeof(unsigned char *));
    const char *err_str = "Unable to allocate memory on the heap.\n";

    if(!back_buffer) {
        fprintf(stderr, "%s", err_str);
        return 1;
    }
    memset(back_buffer, 0, WIDTH * sizeof(unsigned char *));

    for (int i = 0; i < WIDTH; i++) {
        back_buffer[i] = (unsigned char *) malloc(HEIGHT * sizeof(unsigned char));
        if(!back_buffer[i]) {
            fprintf(stderr, "%s", err_str);
            return 1;
        }
        memset(back_buffer[i], 0, HEIGHT * sizeof(unsigned char));
    }

    /* setup window with openGL context */
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    window = SDL_CreateWindow(
        "Kiwi8", 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        WINDOW_WIDTH, 
        WINDOW_HEIGHT, 
        window_mode
    );

    if (window == NULL) {
        fprintf(stderr, "Error: %s\n", SDL_GetError());
        return 1;
    }

    glcontext = SDL_GL_CreateContext(window);

    /* disable V-Sync */
    SDL_GL_SetSwapInterval(0);

    /* specify the texture */
    glTexImage2D(
        GL_TEXTURE_2D, 
        0, 
        GL_RGB, 
        WIDTH, 
        HEIGHT, 
        0,
        GL_RGB, 
        GL_UNSIGNED_BYTE, 
        (GLvoid *) texture
    );

    /* configure the texture */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    /* enable textures */
    glEnable(GL_TEXTURE_2D);

    /* setup ImGui binding */
    gui.Initialize(
        this, 
        steps,
        paused, 
        load_store_quirk, 
        shift_quirk, 
        vwrap, 
        muted 
    );

    /* set to fullscreen mode if flag present */
    if (fullscreen) ToggleFullscreen();

    return 0;
}

void Display::Resize(int x, int y) {
    /* get the current window size */
    WINDOW_WIDTH = x;
    WINDOW_HEIGHT = y;
}

void Display::ToggleFullscreen() {
    /* check if already fullscreen */
    if (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP) {
        /* set windowed */
        SDL_SetWindowFullscreen(window, 0);
        SDL_ShowCursor(SDL_ENABLE);
        fullscreen_flag = 0;

    } else {
        /* set fullscreen */
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        /* currently, a new ImGui Frame will draw the mouse cursor 
           regardless of SDL2's cursor visibility function */
        SDL_ShowCursor(SDL_DISABLE);
        fullscreen_flag = 1;
        
    }
}

void Display::ToggleVsync() {
    if (SDL_GL_GetSwapInterval()) {
        SDL_GL_SetSwapInterval(0);
        vsync_flag = 0;
    } else {
        SDL_GL_SetSwapInterval(1);
        vsync_flag = 1;
    }
}

void Display::RaiseWindow() {
    SDL_RaiseWindow(window);
    lost_window_focus = 0;
}

void Display::RenderFrame(unsigned char **frame){
    gui.NewFrame();

    /* copy the frame to back_buffer */
    if (frame != NULL) {
        for (int i = 0; i < WIDTH; i++) {
            memcpy(back_buffer[i], frame[i], HEIGHT * sizeof(unsigned char));
        }
    }

    /* set Viewport & Clear the screen (sets the background color) */
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    for (int i = 0; i < WIDTH; i++){
        for (int j = 0; j < HEIGHT; j++){
            if (back_buffer[i][HEIGHT-j-1]) {
                /* Fill the foreground pixel */
                texture[j][i][0] = (unsigned char)(foreground_color[0] * (float) 0xFF); //R
                texture[j][i][1] = (unsigned char)(foreground_color[1] * (float) 0xFF); //G
                texture[j][i][2] = (unsigned char)(foreground_color[2] * (float) 0xFF); //B

            } else {
                /* Fill the background pixel */
                texture[j][i][0] = (unsigned char)(background_color[0] * (float) 0xFF); //R
                texture[j][i][1] = (unsigned char)(background_color[1] * (float) 0xFF); //G
                texture[j][i][2] = (unsigned char)(background_color[2] * (float) 0xFF); //B
            }
        }
    }
    
    /* send texture to GPU */
    glTexSubImage2D(
        GL_TEXTURE_2D, 
        0, 
        0, 
        0, 
        WIDTH, 
        HEIGHT,
        GL_RGB, 
        GL_UNSIGNED_BYTE, 
        (GLvoid *) texture
    );

    /* create room at the top for menu bar */
    float top_edge = gui.show_menu_flag ? 
        (float)(WINDOW_HEIGHT - MENU_HEIGHT) / WINDOW_HEIGHT : (float) 1.0;

    /* render the texture */
    glBegin(GL_QUADS);

        /* bottom left */
        glTexCoord2f(0.0, 0.0); 
        glVertex2f(-1.0, -1.0); 

        /* bottom right */
        glTexCoord2f(1.0, 0.0);
        glVertex2f(1.0, -1.0); 

        /* top right */
        glTexCoord2f(1.0, 1.0); 
        glVertex2f(1.0, top_edge); 

        /* top left */
        glTexCoord2f(0.0, 1.0); 
        glVertex2f(-1.0, top_edge); 
        
    glEnd();

    gui.Render();
    SDL_GL_SwapWindow(window);
}
