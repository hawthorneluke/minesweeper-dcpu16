

// dcpu-16-Win32.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "dcpu-16-Win32.h"
#include "dcpu-16.h"
#include <stdio.h>

#include "SDL.h"
#include "SDL_thread.h"


int main(int argc, char *argv[]) {
    
    printf("Initializing SDL.\n");
    
     SDL_Surface *screen;

    /* Initialize the SDL library */
    if( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
        fprintf(stderr,
                "Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    /* Clean up on exit */
    atexit(SDL_Quit);
    
    /*
     * Initialize the display in a 640x480 8-bit palettized mode,
     * requesting a software surface
     */
    screen = SDL_SetVideoMode(640, 480, 8, SDL_SWSURFACE);
    if ( screen == NULL ) {
        fprintf(stderr, "Couldn't set 640x480x8 video mode: %s\n",
                        SDL_GetError());
        exit(1);
    }


	SDL_Thread *SDL_CreateThread(int (*fn)(void *), void *data);


    printf("SDL initialized.\n");



	//CPU *cpu = new CPU();

	while(true){}




    printf("Quiting SDL.\n");
    
    /* Shutdown all subsystems */
    SDL_Quit();
    
    printf("Quiting....\n");

    exit(0);
}