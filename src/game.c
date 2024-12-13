#include "math.h"
#include <stdio.h>        
#include <stdlib.h> 
#include <LPC17xx.H>
#include <stdint.h>/* NXP LPC17xx definitions            */
#include "string.h"
#include "GLCD.h"
#include "LED.h"
#include "KBD.h"
#include "Menu.h"

//#include "game.h"

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
#define DR 0.0174533 //one degree

#define menuWidth 30
#define menuHieght 120 * 2

//mode defs
#define PIXEL_DRAW 2
#define LINE_DRAW 1
#define CIRCLE_DRAW 0
#define MENU_MODE 3

//menu defs
#define MENU_CIRCLE 0 
#define MENU_LINE  1 
#define MENU_PIXEL 2 
#define MENU_SAVE 3 

#define MENU_WIDTH 58
#define MENU_ICON_HEIGHT 52


int MENU = MENU_CIRCLE;
int MODE = PIXEL_DRAW; 



extern unsigned char DRAWMEN_pixel_data[];
extern unsigned char userPicture[6][48];

extern unsigned char ClockLEDOn;

//SysTick_Config(SystemCoreClock/50);  /* Generate interrupt every 20 ms     */

float playerX, playerY, playerAngle; //player pos

//#define mapX 8
//#define mapY 8
//#define mapS 64

int pX = 0, pY = 0;

//int menuSpacing = 79;


int mapX=8; int mapY=8;

//--Histrory Array Def 
int lastPressX;
int lastPressY;

int lastPressX1;
int lastPressY1;

int lastPressX2;
int lastPressY2;

int hist[6];  // Format X0 Y0 X1 Y1 X2 Y2

int map[]=
{

1,1,1,1,1,1,1,1,
1,0,0,1,0,0,0,1,
1,0,0,1,0,0,0,1,
1,0,0,0,0,0,0,1,
1,0,0,0,0,1,1,1,
1,0,0,0,0,0,0,1,
1,0,0,0,0,0,0,1,
1,1,1,1,1,1,1,1

};

void init(){
	
GLCD_Clear(LightGrey);
pX= 1;// set player pos
pY= 1;
GLCD_SetBackColor(Black);
GLCD_SetTextColor(Black);

//playerAngle= 0;
}

void setColor(int color){
GLCD_SetBackColor(color);
GLCD_SetTextColor(color);

}

//set mode
void setVarMode(int mode){

	MODE = mode;
}
//return what next mode is
int nextMode(int mode){

int temp = mode;
temp++;
	
if(temp > 3){
return 0;
}
return temp;
}


int prevMode(int mode){
	
	int temp = mode;
temp--;
	
if(temp < 0){
return 3;
}
return temp;
}

float degToRad(float angle) {
    return angle * PI / 180.0f;
}



//draw 1 pixel
void draw1Pixel(unsigned int x, unsigned int y){

GLCD_PutPixel(x, y);

}



void drawPixel(unsigned int x, unsigned int y ){ //convert320x240 -> 160x120
	
//GLCD_SetBackColor(Black);

	unsigned int newX = x*2;
	unsigned int newY = y*2;
	
GLCD_PutPixel(newX, newY);
GLCD_PutPixel(newX+1, newY);
GLCD_PutPixel(newX, newY+1);
GLCD_PutPixel(newX+1, newY+1);

//GLCD_SetBackColor(LightGrey);

	
}





//draws line from x1y1 to x2y2 in 320x240
void drawLine(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1){
	 
		unsigned int x = x0; // Create a copy of x0
    unsigned int y = y0; // Create a copy of y0

    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1); // Absolute difference in x
    int dy = (y1 > y0) ? (y1 - y0) : (y0 - y1); // Absolute difference in y
    int sx = (x0 < x1) ? 1 : -1;                // Step direction for x
    int sy = (y0 < y1) ? 1 : -1;                // Step direction for y
    int err = dx - dy;                          // Error term

    while (1) {
        draw1Pixel(x, y); // Plot the current pixel
        if (x == x1 && y == y1) break;          // Check if we reached the endpoint

        int e2 = 2 * err; // Scaled error term
        if (e2 > -dy) {
            err -= dy;
            x += sx;  // Move in the x direction
        }
        if (e2 < dx) {
            err += dx;
            y += sy;  // Move in the y direction
        }
    }
}






//draws rect from x,y with width and height
void drawRect(unsigned int x, unsigned int y, unsigned int width, unsigned int height) {
   
	//setColor(Blue);
	// Draw the top edge
    drawLine(x, y, x + width-1, y);
    // Draw the right edge
    drawLine(x + width-1, y, x + width-1, y + height-1);
    // Draw the bottom edge
    drawLine(x + width - 1, y + height-1, x, y + height-1);
    // Draw the left edge
    drawLine(x, y + height - 1, x, y);
	
	//setColor(Black);
}


void drawFilledRect(unsigned int x, unsigned int y, unsigned int width, unsigned int height) {
   
	//setColor(Blue);
	// Draw the top edge
  
		for (int i = 0; i<height;i++){
		
		drawLine(x,y+i, x+width-1,y+i);
		
		}
	
	//setColor(Black);
}

//draw circle
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




void drawPointer(unsigned int x, unsigned int y){

		setColor(Green);

		drawRect((x)*5 - 1 + 80,(y)*5 - 1,7,7);
}


void removePointer(unsigned int x, unsigned int y){

		setColor(White);
	
		drawRect((x)*5 - 1 + 80,(y)*5 - 1,7,7);

		                                                                   
}



void draw5Pixel(unsigned int x, unsigned int y, int colour){


setColor(colour);
	
drawFilledRect(x*5 + 80, y*5, 5, 5);

	
}




void movePointer(unsigned int x, unsigned int y){

removePointer(pX, pY);
drawPointer(x,y);

}

// Update player position based on input

//updates single pixel on canvas
void canvasUpdatePixel(unsigned int x, unsigned int y){

	unsigned int byteIndex = x / 8;
	
	int bitIndex = x % 8; //(0,7)
	
	unsigned char byte = userPicture[byteIndex][y];
	
	int bitValue = (byte >> (7 - bitIndex) & 1);
	
	if (bitValue == 1){
	
		draw5Pixel(x,y, Black);
	}
	
	else draw5Pixel(x,y, White);
	
	

} 

void refreshMenu(){

	GLCD_Bitmap(0,0, 60,240, DRAWMEN_pixel_data);


}


void refreshCanvas(){

	int bitIndex = 0;
	
	 for (int col = 0; col < 6; col++) {
        for (int row = 0; row < 48; row++) {
            char byte = userPicture[col][row]; // Get the current byte

            // Process each bit in the byte
            for (int bit = 0; bit < 8; bit++) {
                // Check if the bit is set
							

							
                if (byte & (1 << bit)) {
                    // Calculate the pixel's (x, y) position on the 48x48 screen
                    int x = (col*8) + bit;      // Column position
                    int y = row ;             // Row position
                   
                    // Draw the pixel (or a block for scaling)
                    draw5Pixel(x, y, Black); // Use the put5Pixel function to draw the pixel
                }
								
								else {
									int x = (col*8) + bit;      // Column position
                  int y = row;  
									draw5Pixel(x, y, White);}
            }
        }
    }


}





void refreshCanvasStart(){

for (int col = 0; col < 48; col++) {
        for (int row = 0; row < 48; row++) {
					
					
					canvasUpdatePixel(col,row);
					
				}
				
			}

}

void canvasDrawPixel(unsigned int x, unsigned int y, int colour){
	
//input x is in range (0,47), but maps to specific bit in 6 bytes
	
	
// array[6][48]
//y stays y (0,47)
//x depends on specfic bit (0,6) -> (0,47)
	
	
	
	//unsigned int newX = x;
	
	int byteIndex = x / 8;
	int bitIndex = x % 8;

if (colour == Black){
	
	//stes black
	userPicture[byteIndex][y] |= (1 << (7-bitIndex));

}

else{

	//sets white
	userPicture[byteIndex][y] &= (1 << (7-bitIndex));
	
}

draw5Pixel(x, y, colour);

}



//menu logic functions, returns menu VAL after function
int menuUp(){

int temp = MENU-1 ;

if (temp < 0) temp = 3;	

return temp;
	
	
}

int menuDown(){

int temp = MENU+1 ;	
	
	if (temp > 3) temp = 0;

	return temp;
	
}


//
void drawMenuPointer(int menuItem){

setColor(Blue);

	
drawRect(1,(menuItem) *60 ,MENU_WIDTH, MENU_ICON_HEIGHT);

	

setColor(Black);

}


void drawLoadMenuPointer(int menuItem){

setColor(Red);

	
drawRect(1,(menuItem) *60 ,MENU_WIDTH, MENU_ICON_HEIGHT);

	

setColor(Black);

}

void removeMenuPointer(int menuItem){


	setColor(White);	
	drawRect(1,(menuItem) *60 ,MENU_WIDTH, MENU_ICON_HEIGHT);
	setColor(Black);
	
}

void updateMenu(int menuItem){

	removeMenuPointer(MENU);
	
	drawMenuPointer(menuItem);
	
	removeMenuPointer(MENU);
	MENU = menuItem;
	//removeMenuPointer(MENU);
}

//set mode
void setMode(int mode){

switch (mode)
{
	case MENU_MODE :
		MODE = MENU_MODE;
		removePointer(pX,pY);
		drawMenuPointer(MENU);
		break;
	
	case PIXEL_DRAW :
		MODE = PIXEL_DRAW;
		removeMenuPointer(MENU);
		break;
	
	case CIRCLE_DRAW :
		MODE = CIRCLE_DRAW;
		removeMenuPointer(MENU);
		break;
	
	case LINE_DRAW :
		MODE = LINE_DRAW;
		removeMenuPointer(MENU);
		break;

}

}


//refreshes the gap inbetween to clear stuck pointers
void refreshGap(int y){

	setColor(LightGrey);
	drawFilledRect(60, (y * 5) -1, 20, 13);
	setColor(Black);
	
}

//update pointer func
void updatePointer() {
	
		int input = get_button();
	
//----PRESSES UP -----
	
		//not in menu
    if (input == KBD_UP && MODE != MENU_MODE) {
   
			removePointer(pX, pY);
			canvasUpdatePixel(pX,pY);

			pY--;
			
			//if (pX > 48 || pX < 1)pX = 1; 
			if (pY > 48 || pY < 1) pY = 1;
			
			drawPointer(pX,pY);
			
    } 
		
		//--in menu
		else if (input == KBD_UP && MODE == MENU_MODE){
			
			updateMenu(menuUp());
		
		
		}
//------ PRESSES DOWN --------
		
		//not in menu
		else if (input == KBD_DOWN && MODE != MENU_MODE) {
			
			
		removePointer(pX, pY);
		canvasUpdatePixel(pX,pY);

			
			
			pY++;
			
			//if (pX > 48 || pX < 1)pX = 1; 
			if (pY > 48 || pY < 1) pY = 1;
			
			drawPointer(pX,pY);
    } 
		
		//in menu
		else if (input == KBD_DOWN && MODE == MENU_MODE){
				
				updateMenu(menuDown());
				
		}
		
		else if (input == KBD_LEFT && MODE != MENU_MODE) {
			
			//--
			
			

			//--
			
			removePointer(pX, pY);
			canvasUpdatePixel(pX,pY);

		
			
				if (pX > 48)pX = 1; //dont need this
			
				if (pX < 1) {
					setMode(MENU_MODE); refreshGap(pY);
						} //set to menu mode
			
				else {
					pX--;
					drawPointer(pX,pY);
					
				}
			
    } else if (input == KBD_RIGHT) {
        
				if(MODE != MENU_MODE) {
			
					removePointer(pX, pY);
					canvasUpdatePixel(pX,pY);

			
					if (pX > 48 || pX < 1)pX = 1; //removePointer(pX + 1, pY); 
			
					pX++;
					drawPointer(pX,pY);
				}
				
				else{
				
				pX = 1;// pY =1;
				refreshGap(pY);
				setMode(PIXEL_DRAW);
				drawPointer(pX,pY);
				}
    }
		
		
		
		
		if (pX > 48 || pX < 0)pX = 1; 
		if (pY > 48 || pY < 0) pY = 1;
}


void draw5Line(unsigned int x0, unsigned int y0,unsigned int x1, unsigned int y1, int colour ){

unsigned int x = x0; // Create a copy of x0
    unsigned int y = y0; // Create a copy of y0

    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1); // Absolute difference in x
    int dy = (y1 > y0) ? (y1 - y0) : (y0 - y1); // Absolute difference in y
    int sx = (x0 < x1) ? 1 : -1;                // Step direction for x
    int sy = (y0 < y1) ? 1 : -1;                // Step direction for y
    int err = dx - dy;                          // Error term

    while (1) {
        draw5Pixel(x, y, colour); // Plot the current pixel
        if (x == x1 && y == y1) break;          // Check if we reached the endpoint

        int e2 = 2 * err; // Scaled error term
        if (e2 > -dy) {
            err -= dy;
            x += sx;  // Move in the x direction
        }
        if (e2 < dx) {
            err += dx;
            y += sy;  // Move in the y direction
        }
    }
}



//draw line on canvas
void canvasDrawLine(unsigned int x0, unsigned int y0,unsigned int x1, unsigned int y1, int colour){


unsigned int x = x0; // Create a copy of x0
    unsigned int y = y0; // Create a copy of y0

    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1); // Absolute difference in x
    int dy = (y1 > y0) ? (y1 - y0) : (y0 - y1); // Absolute difference in y
    int sx = (x0 < x1) ? 1 : -1;                // Step direction for x
    int sy = (y0 < y1) ? 1 : -1;                // Step direction for y
    int err = dx - dy;                          // Error term

    while (1) {
        canvasDrawPixel(x, y, colour); // Plot the current pixel
        if (x == x1 && y == y1) break;          // Check if we reached the endpoint

        int e2 = 2 * err; // Scaled error term
        if (e2 > -dy) {
            err -= dy;
            x += sx;  // Move in the x direction
        }
        if (e2 < dx) {
            err += dx;
            y += sy;  // Move in the y direction
        }
    }

}






void canvasUpdateLine(unsigned int x0, unsigned int y0,unsigned int x1, unsigned int y1){


unsigned int x = x0; // Create a copy of x0
    unsigned int y = y0; // Create a copy of y0

    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1); // Absolute difference in x
    int dy = (y1 > y0) ? (y1 - y0) : (y0 - y1); // Absolute difference in y
    int sx = (x0 < x1) ? 1 : -1;                // Step direction for x
    int sy = (y0 < y1) ? 1 : -1;                // Step direction for y
    int err = dx - dy;                          // Error term

    while (1) {
        canvasUpdatePixel(x, y); // Plot the current pixel
        if (x == x1 && y == y1) break;          // Check if we reached the endpoint

        int e2 = 2 * err; // Scaled error term
        if (e2 > -dy) {
            err -= dy;
            x += sx;  // Move in the x direction
        }
        if (e2 < dx) {
            err += dx;
            y += sy;  // Move in the y direction
        }
    }

}


void canvasDrawCircle(int xc, int yc, int radius){
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius; // Initial decision parameter

    while (x <= y) {
        // Draw the 8 symmetric points of the circle
        canvasDrawPixel(xc + x, yc + y, Black);  // 1st quadrant
        canvasDrawPixel(xc - x, yc + y, Black);  // 2nd quadrant
        canvasDrawPixel(xc + x, yc - y, Black);  // 3rd quadrant
        canvasDrawPixel(xc - x, yc - y, Black);  // 4th quadrant
        canvasDrawPixel(xc + y, yc + x, Black);  // 5th quadrant
        canvasDrawPixel(xc - y, yc + x, Black);  // 6th quadrant
        canvasDrawPixel(xc + y, yc - x, Black);  // 7th quadrant
        canvasDrawPixel(xc - y, yc - x, Black);  // 8th quadrant

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


void game(){
int histNum = 0; //stores number of saved points
int cHistNum = 0; //for circle
unsigned int lineframeCounter = 0;
unsigned int circleframeCounter = 0;
unsigned int menuframeCounter = 0;

	
	
init();
refreshMenu();

draw5Pixel(4,4, White);

	int rad = 0;
	refreshCanvasStart();

	drawPointer(pX,pY);
	
for(;;){

	//------DEFAULT UPDATE-------
	if (ClockLEDOn) { ClockLEDOn = 0; updatePointer(); }
	
	

	
	//------ Draw Pixel ---------
	if(get_button() == KBD_SELECT && MODE == PIXEL_DRAW){
	
		canvasDrawPixel(pX,pY, Black);
		
	}
	
	
	
		
		//---- Line draw------	
		lineframeCounter++;
		if(get_button() == KBD_SELECT && MODE == LINE_DRAW){
			drawLoadMenuPointer(MENU);
			if (histNum == 0 && lineframeCounter > 30) {
		
				lastPressX1 = pX;
				lastPressY1 = pY;
		
				histNum = 1;
				lineframeCounter = 0;		
			}
		
			else if (histNum == 1 && lineframeCounter > 30) {
		
				lastPressX2 = pX;
				lastPressY2 = pY;
		
				canvasDrawLine(lastPressX1,lastPressY1, lastPressX2, lastPressY2, Black);

				histNum=0;	
				lineframeCounter = 0;
			}
		}
	
	//------- Change Mode & Refresh --------
	if(get_button() == KBD_SELECT && MODE == MENU_MODE){
	
		if (MENU == MENU_SAVE){
		
		drawLoadMenuPointer(MENU_SAVE);
		refreshCanvasStart();
		removeMenuPointer(MENU_SAVE);
		
		}
		else MODE = MENU; //set mode to current Menu Selection
	}
	
	
	
	
	
	

	//------ Circle Draw--------
	circleframeCounter++;
	if(get_button() == KBD_SELECT && MODE == CIRCLE_DRAW){
				drawLoadMenuPointer(MENU);

		if (cHistNum == 0 && circleframeCounter > 30) {
		
				lastPressX1 = pX;
				lastPressY1 = pY;
		
				cHistNum = 1;
				circleframeCounter = 0;		
			}
		
			else if (cHistNum == 1 && circleframeCounter > 30) {
		
				lastPressX2 = pX;
				lastPressY2 = pY;
				
				rad = fmax( abs(lastPressX1 - pX)/2 , abs(lastPressX2 - pY)/2);
		
				canvasDrawCircle(lastPressX1,lastPressY1, rad);

				cHistNum=0;	
				circleframeCounter = 0;
			}
		
		//canvasDrawCircle(20,20, 7);
		
	}
	
	
	if (get_button() == KBD_LEFT && MODE == MENU_MODE){
	return;
	}

};	


}