#ifndef __TILES_H__
#define __TILES_H__

#include <SDL.h>

// Declare the external variables
extern int windowWidth;   // the SDL window's width
extern int windowHeight;  // the SDL window's height
extern boolean fullScreen; // true if the window should be full-screen, else false

// Function declarations
void initTiles(void);
void resizeWindow(int width, int height);
void updateTile(int row, int column, short charIndex,
    short foreRed, short foreGreen, short foreBlue,
    short backRed, short backGreen, short backBlue);
void updateScreen(void);
SDL_Surface *captureScreen(void);

#endif
