#include "math.h"
#include <stdio.h>        
#include <stdlib.h> 
#include <LPC17xx.H>
#include <stdint.h>/* NXP LPC17xx definitions */
#include "string.h"
#include "GLCD.h"
#include "LED.h"
#include "KBD.h"
#include "Menu.h"

// Define screen dimensions
#define SCREEN_WIDTH  160
#define SCREEN_HEIGHT 120

// Define map properties
#define MAP_WIDTH 8
#define MAP_HEIGHT 8
#define TILE_SIZE 64

// Define field of view
#define FOV 60
#define NUM_RAYS SCREEN_WIDTH

#define PI 3.14159
#define P2 PI/2
#define P3 3*PI/2
#define DR 0.0174533 // one degree

#define menuWidth 30
#define menuHieght 120 * 2

// Mode definitions
#define PIXEL_DRAW 2
#define LINE_DRAW 1
#define CIRCLE_DRAW 0
#define MENU_MODE 3

// Menu item definitions
#define MENU_CIRCLE 0 
#define MENU_LINE  1 
#define MENU_PIXEL 2 
#define MENU_SAVE 3 

#define MENU_WIDTH 58
#define MENU_ICON_HEIGHT 52

// Global variables for menu and mode
int MENU = MENU_CIRCLE;
int MODE = PIXEL_DRAW; 

// External variables
extern unsigned char DRAWMEN_pixel_data[];
extern unsigned char userPicture[6][48];
extern unsigned char ClockLEDOn;

// Player position and angle
float playerX, playerY, playerAngle;

// Player coordinates
int pX = 0, pY = 0;

// Map dimensions
int mapX = 8; 
int mapY = 8;

// History array for storing previous positions
int lastPressX;
int lastPressY;

int lastPressX1;
int lastPressY1;

int lastPressX2;
int lastPressY2;

int hist[6];  // Format X0 Y0 X1 Y1 X2 Y2

// Initialize the game
void init(){
    GLCD_Clear(LightGrey);       // Clear the screen with LightGrey color
    pX = 1;                       // Set initial player X position
    pY = 1;                       // Set initial player Y position
    GLCD_SetBackColor(Black);     // Set background color to Black
    GLCD_SetTextColor(Black);     // Set text color to Black
    // playerAngle = 0;           // Initialize player angle if needed
}

// Set the drawing color
void setColor(int color){
    GLCD_SetBackColor(color);     // Set background color
    GLCD_SetTextColor(color);     // Set text color
}

// Set the current mode
void setVarMode(int mode){
    MODE = mode;
}

// Get the next drawing mode
int nextMode(int mode){
    int temp = mode;
    temp++;
    if(temp > 3){
        return 0;
    }
    return temp;
}

// Get the previous drawing mode
int prevMode(int mode){
    int temp = mode;
    temp--;
    if(temp < 0){
        return 3;
    }
    return temp;
}

// Convert degrees to radians
float degToRad(float angle) {
    return angle * PI / 180.0f;
}

// Draw a single pixel
void draw1Pixel(unsigned int x, unsigned int y){
    GLCD_PutPixel(x, y);
}

// Draw a pixel with scaling (160x120)
void drawPixel(unsigned int x, unsigned int y){
    unsigned int newX = x * 2;
    unsigned int newY = y * 2;
    
    GLCD_PutPixel(newX, newY);
    GLCD_PutPixel(newX + 1, newY);
    GLCD_PutPixel(newX, newY + 1);
    GLCD_PutPixel(newX + 1, newY + 1);
}

// Draw a line using Bresenham's algorithm
void drawLine(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1){
    unsigned int x = x0;
    unsigned int y = y0;

    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1); // Absolute difference in x
    int dy = (y1 > y0) ? (y1 - y0) : (y0 - y1); // Absolute difference in y
    int sx = (x0 < x1) ? 1 : -1;                // Step direction for x
    int sy = (y0 < y1) ? 1 : -1;                // Step direction for y
    int err = dx - dy;                          // Error term

    while (1) {
        draw1Pixel(x, y);                      // Plot the current pixel
        if (x == x1 && y == y1) break;          // Check if endpoint is reached

        int e2 = 2 * err;                       // Scaled error
        if (e2 > -dy) {
            err -= dy;
            x += sx;                            // Move in x direction
        }
        if (e2 < dx) {
            err += dx;
            y += sy;                            // Move in y direction
        }
    }
}

// Draw a rectangle by drawing its edges
void drawRect(unsigned int x, unsigned int y, unsigned int width, unsigned int height) {
    drawLine(x, y, x + width - 1, y);                     // Top edge
    drawLine(x + width - 1, y, x + width - 1, y + height - 1); // Right edge
    drawLine(x + width - 1, y + height - 1, x, y + height - 1); // Bottom edge
    drawLine(x, y + height - 1, x, y);                    // Left edge
}

// Draw a filled rectangle by filling it line by line
void drawFilledRect(unsigned int x, unsigned int y, unsigned int width, unsigned int height) {
    for (int i = 0; i < height; i++){
        drawLine(x, y + i, x + width - 1, y + i);         // Draw each horizontal line
    }
}

// Draw a circle using Bresenham's algorithm
void drawCircle(int xc, int yc, int radius) {
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius; // Initial decision parameter

    while (x <= y) {
        // Draw the 8 symmetric points of the circle
        drawPixel(xc + x, yc + y);
        drawPixel(xc - x, yc + y);
        drawPixel(xc + x, yc - y);
        drawPixel(xc - x, yc - y);
        drawPixel(xc + y, yc + x);
        drawPixel(xc - y, yc + x);
        drawPixel(xc + y, yc - x);
        drawPixel(xc - y, yc - x);

        // Update decision parameter and coordinates
        if (d <= 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

// Draw the pointer on the canvas
void drawPointer(unsigned int x, unsigned int y){
    setColor(Green); // Set color to Green
    drawRect((x) * 5 - 1 + 80, (y) * 5 - 1, 7, 7); // Draw pointer rectangle
}

// Remove the pointer from the canvas
void removePointer(unsigned int x, unsigned int y){
    setColor(White); // Set color to White (background)
    drawRect((x) * 5 - 1 + 80, (y) * 5 - 1, 7, 7); // Erase pointer rectangle
}

// Draw a 5x5 pixel block with specified color
void draw5Pixel(unsigned int x, unsigned int y, int colour){
    setColor(colour); // Set drawing color
    drawFilledRect(x * 5 + 80, y * 5, 5, 5); // Draw filled 5x5 block
}

// Move the pointer to a new location
void movePointer(unsigned int x, unsigned int y){
    removePointer(pX, pY); // Remove current pointer
    drawPointer(x, y);      // Draw new pointer
}

// Update a single pixel on the canvas based on user input
void canvasUpdatePixel(unsigned int x, unsigned int y){
    unsigned int byteIndex = x / 8;             // Determine byte index
    int bitIndex = x % 8;                        // Determine bit index
    unsigned char byte = userPicture[byteIndex][y];
    int bitValue = (byte >> (7 - bitIndex)) & 1; // Get bit value

    if (bitValue == 1){
        draw5Pixel(x, y, Black);                // Draw black pixel
    }
    else {
        draw5Pixel(x, y, White);                // Draw white pixel
    }
} 

// Refresh the menu display
void refreshMenu(){
    GLCD_Bitmap(0, 0, 60, 240, DRAWMEN_pixel_data); // Display menu bitmap
}

// Refresh the entire canvas display
void refreshCanvas(){
    for (int col = 0; col < 6; col++) {
        for (int row = 0; row < 48; row++) {
            char byte = userPicture[col][row]; // Get the current byte

            // Process each bit in the byte
            for (int bit = 0; bit < 8; bit++) {
                if (byte & (1 << bit)) {
                    int x = (col * 8) + bit;      // Calculate X position
                    int y = row;                   // Y position
                    draw5Pixel(x, y, Black);      // Draw black pixel
                }
                else {
                    int x = (col * 8) + bit;      // Calculate X position
                    int y = row;                   // Y position
                    draw5Pixel(x, y, White);      // Draw white pixel
                }
            }
        }
    }
}

// Refresh the canvas at the start of the game
void refreshCanvasStart(){
    for (int col = 0; col < 48; col++) {
        for (int row = 0; row < 48; row++) {
            canvasUpdatePixel(col, row); // Update each pixel
        }
    }
}

// Draw a pixel on the canvas with a specified color
void canvasDrawPixel(unsigned int x, unsigned int y, int colour){
    int byteIndex = x / 8;          // Determine byte index
    int bitIndex = x % 8;           // Determine bit index

    if (colour == Black){
        userPicture[byteIndex][y] |= (1 << (7 - bitIndex)); // Set bit to draw black
    }
    else {
        userPicture[byteIndex][y] &= ~(1 << (7 - bitIndex)); // Clear bit to draw white
    }

    draw5Pixel(x, y, colour);       // Update the pixel on display
}

// Move the menu selection up
int menuUp(){
    int temp = MENU - 1;
    if (temp < 0) temp = 3;          // Wrap around to last menu item
    return temp;
}

// Move the menu selection down
int menuDown(){
    int temp = MENU + 1;
    if (temp > 3) temp = 0;          // Wrap around to first menu item
    return temp;
}

// Draw the menu pointer at the specified menu item
void drawMenuPointer(int menuItem){
    setColor(Blue);                                  // Set color to Blue
    drawRect(1, (menuItem) * 60, MENU_WIDTH, MENU_ICON_HEIGHT); // Draw pointer rectangle
    setColor(Black);                                 // Reset color to Black
}

// Draw the load menu pointer at the specified menu item
void drawLoadMenuPointer(int menuItem){
    setColor(Red);                                   // Set color to Red
    drawRect(1, (menuItem) * 60, MENU_WIDTH, MENU_ICON_HEIGHT); // Draw pointer rectangle
    setColor(Black);                                 // Reset color to Black
}

// Remove the menu pointer from the specified menu item
void removeMenuPointer(int menuItem){
    setColor(White);                                 // Set color to White (background)
    drawRect(1, (menuItem) * 60, MENU_WIDTH, MENU_ICON_HEIGHT); // Erase pointer rectangle
    setColor(Black);                                 // Reset color to Black
}

// Update the menu selection to the specified menu item
void updateMenu(int menuItem){
    removeMenuPointer(MENU);                         // Remove current menu pointer
    drawMenuPointer(menuItem);                       // Draw new menu pointer
    MENU = menuItem;                                 // Update current menu selection
}

// Set the current mode based on input
void setMode(int mode){
    switch (mode)
    {
        case MENU_MODE:
            MODE = MENU_MODE;
            removePointer(pX, pY);                  // Remove current pointer
            drawMenuPointer(MENU);                   // Draw menu pointer
            break;
        
        case PIXEL_DRAW:
            MODE = PIXEL_DRAW;
            removeMenuPointer(MENU);                 // Remove menu pointer
            break;
        
        case CIRCLE_DRAW:
            MODE = CIRCLE_DRAW;
            removeMenuPointer(MENU);                 // Remove menu pointer
            break;
        
        case LINE_DRAW:
            MODE = LINE_DRAW;
            removeMenuPointer(MENU);                 // Remove menu pointer
            break;
    }
}

// Refresh the gap between pointers to clear any stuck pointers
void refreshGap(int y){
    setColor(LightGrey);                             // Set color to LightGrey
    drawFilledRect(60, (y * 5) - 1, 20, 13);        // Draw filled rectangle to refresh gap
    setColor(Black);                                 // Reset color to Black
}

// Update the pointer position based on user input
void updatePointer() {
    int input = get_button();                        // Get user input
    
    // Handle UP button press
    if (input == KBD_UP && MODE != MENU_MODE) {
        removePointer(pX, pY);                       // Remove current pointer
        canvasUpdatePixel(pX, pY);                   // Update canvas at current position
        pY--;                                        // Move pointer up
        if (pY > 48 || pY < 1) pY = 1;               // Boundary check
        drawPointer(pX, pY);                         // Draw new pointer position
    } 
    // Handle UP button press in menu mode
    else if (input == KBD_UP && MODE == MENU_MODE){
        updateMenu(menuUp());                         // Move menu selection up
    }

    // Handle DOWN button press
    else if (input == KBD_DOWN && MODE != MENU_MODE) {
        removePointer(pX, pY);                       // Remove current pointer
        canvasUpdatePixel(pX, pY);                   // Update canvas at current position
        pY++;                                        // Move pointer down
        if (pY > 48 || pY < 1) pY = 1;               // Boundary check
        drawPointer(pX, pY);                         // Draw new pointer position
    } 
    // Handle DOWN button press in menu mode
    else if (input == KBD_DOWN && MODE == MENU_MODE){
        updateMenu(menuDown());                       // Move menu selection down
    }
    
    // Handle LEFT button press
    else if (input == KBD_LEFT && MODE != MENU_MODE) {
        removePointer(pX, pY);                       // Remove current pointer
        canvasUpdatePixel(pX, pY);                   // Update canvas at current position
        if (pX < 1) {
            setMode(MENU_MODE);                       // Switch to menu mode
            refreshGap(pY);                           // Refresh gap to clear pointer
        } 
        else {
            pX--;                                      // Move pointer left
            drawPointer(pX, pY);                      // Draw new pointer position
        }
    } 
    // Handle RIGHT button press
    else if (input == KBD_RIGHT) {
        if(MODE != MENU_MODE) {
            removePointer(pX, pY);                   // Remove current pointer
            canvasUpdatePixel(pX, pY);               // Update canvas at current position
            if (pX > 48 || pX < 1) pX = 1;           // Boundary check
            pX++;                                    // Move pointer right
            drawPointer(pX, y);                      // Draw new pointer position
        }
        else{
            pX = 1;                                  // Reset X position
            refreshGap(pY);                           // Refresh gap to clear pointer
            setMode(PIXEL_DRAW);                      // Switch to pixel draw mode
            drawPointer(pX, pY);                      // Draw pointer in new mode
        }
    }

    // Ensure pointer stays within bounds
    if (pX > 48 || pX < 0) pX = 1;                   
    if (pY > 48 || pY < 0) pY = 1;
}

// Draw a line with 5x5 scaling
void draw5Line(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, int colour ){
    unsigned int x = x0;
    unsigned int y = y0;

    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int dy = (y1 > y0) ? (y1 - y0) : (y0 - y1);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        draw5Pixel(x, y, colour);                   // Plot the current scaled pixel
        if (x == x1 && y == y1) break;              // Check if endpoint is reached

        int e2 = 2 * err;                            // Scaled error
        if (e2 > -dy) {
            err -= dy;
            x += sx;                                  // Move in x direction
        }
        if (e2 < dx) {
            err += dx;
            y += sy;                                  // Move in y direction
        }
    }
}

// Draw a line on the canvas with specified color
void canvasDrawLine(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, int colour){
    unsigned int x = x0;
    unsigned int y = y0;

    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int dy = (y1 > y0) ? (y1 - y0) : (y0 - y1);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        canvasDrawPixel(x, y, colour);             // Plot the current pixel on canvas
        if (x == x1 && y == y1) break;             // Check if endpoint is reached

        int e2 = 2 * err;                            // Scaled error
        if (e2 > -dy) {
            err -= dy;
            x += sx;                                  // Move in x direction
        }
        if (e2 < dx) {
            err += dx;
            y += sy;                                  // Move in y direction
        }
    }
}

// Update a line on the canvas by redrawing affected pixels
void canvasUpdateLine(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1){
    unsigned int x = x0;
    unsigned int y = y0;

    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int dy = (y1 > y0) ? (y1 - y0) : (y0 - y1);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        canvasUpdatePixel(x, y);                   // Update the current pixel on canvas
        if (x == x1 && y == y1) break;             // Check if endpoint is reached

        int e2 = 2 * err;                            // Scaled error
        if (e2 > -dy) {
            err -= dy;
            x += sx;                                  // Move in x direction
        }
        if (e2 < dx) {
            err += dx;
            y += sy;                                  // Move in y direction
        }
    }
}

// Draw a circle on the canvas with specified radius
void canvasDrawCircle(int xc, int yc, int radius){
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius; // Initial decision parameter

    while (x <= y) {
        // Draw the 8 symmetric points of the circle on the canvas
        canvasDrawPixel(xc + x, yc + y, Black);
        canvasDrawPixel(xc - x, yc + y, Black);
        canvasDrawPixel(xc + x, yc - y, Black);
        canvasDrawPixel(xc - x, yc - y, Black);
        canvasDrawPixel(xc + y, yc + x, Black);
        canvasDrawPixel(xc - y, yc + x, Black);
        canvasDrawPixel(xc + y, yc - x, Black);
        canvasDrawPixel(xc - y, yc - x, Black);

        // Update decision parameter and coordinates
        if (d <= 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

// Main game loop
void game(){
    int histNum = 0;          // Stores number of saved points for line drawing
    int cHistNum = 0;         // Stores number of saved points for circle drawing
    unsigned int lineframeCounter = 0;
    unsigned int circleframeCounter = 0;
    unsigned int menuframeCounter = 0;
    
    init();                   // Initialize game settings
    refreshMenu();            // Display the menu
    
    draw5Pixel(4, 4, White);  // Draw an initial white pixel
    
    int rad = 0;              // Initialize radius for circle drawing
    refreshCanvasStart();     // Refresh the entire canvas at start
    
    drawPointer(pX, pY);      // Draw the initial pointer position
    
    for(;;){                  // Infinite game loop
        // Default update based on clock
        if (ClockLEDOn) { 
            ClockLEDOn = 0; 
            updatePointer(); 
        }
        
        // Handle drawing a single pixel
        if(get_button() == KBD_SELECT && MODE == PIXEL_DRAW){
            canvasDrawPixel(pX, pY, Black); // Draw black pixel at pointer
        }
        
        // Handle line drawing
        lineframeCounter++;
        if(get_button() == KBD_SELECT && MODE == LINE_DRAW){
            drawLoadMenuPointer(MENU); // Indicate line drawing mode
            if (histNum == 0 && lineframeCounter > 30) {
                lastPressX1 = pX;
                lastPressY1 = pY;
                histNum = 1;
                lineframeCounter = 0;		
            }   
            else if (histNum == 1 && lineframeCounter > 30) {
                lastPressX2 = pX;
                lastPressY2 = pY;
                canvasDrawLine(lastPressX1, lastPressY1, lastPressX2, lastPressY2, Black); // Draw the line
                histNum = 0;	
                lineframeCounter = 0;
            }
        }
        
        // Handle menu selection and mode change
        if(get_button() == KBD_SELECT && MODE == MENU_MODE){
            if (MENU == MENU_SAVE){
                drawLoadMenuPointer(MENU_SAVE);       // Indicate save action
                refreshCanvasStart();                 // Refresh canvas
                removeMenuPointer(MENU_SAVE);         // Remove save pointer
            }
            else {
                MODE = MENU;                          // Set mode to current menu selection
            }
        }
        
        // Handle circle drawing
        circleframeCounter++;
        if(get_button() == KBD_SELECT && MODE == CIRCLE_DRAW){
            drawLoadMenuPointer(MENU); // Indicate circle drawing mode
            if (cHistNum == 0 && circleframeCounter > 30) {
                lastPressX1 = pX;
                lastPressY1 = pY;
                cHistNum = 1;
                circleframeCounter = 0;		
            }   
            else if (cHistNum == 1 && circleframeCounter > 30) {
                lastPressX2 = pX;
                lastPressY2 = pY;
                rad = fmax(abs(lastPressX1 - pX)/2, abs(lastPressY1 - pY)/2); // Calculate radius
                canvasDrawCircle(lastPressX1, lastPressY1, rad); // Draw the circle
                cHistNum = 0;	
                circleframeCounter = 0;
            }
        }
        
        // Handle exiting menu mode
        if (get_button() == KBD_LEFT && MODE == MENU_MODE){
            return; // Exit the game loop
        }
    };	
}
