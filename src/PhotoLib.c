#include <stdio.h>         
#include <LPC17xx.H>
#include <stdint.h>/* NXP LPC17xx definitions            */
#include "string.h"
#include "GLCD.h"
#include "LED.h"
#include "KBD.h"
#include "Menu.h"
//#include "iraq.c"


extern unsigned char IRAQ_pixel_data[];
//extern unsigned char CUTSOM_pixel_data[];
extern unsigned char userPicture[6][48];
extern unsigned char STAR_pixel_data[];
extern unsigned char HEADER_pixel_data[];


extern void refreshCanvasStart();


void initPhotoGal(){

GLCD_Clear (White);

	//gonna have to put in bitmaps for labels and whatnot

}

int switchImage(int imgNum){

switch(imgNum){

	case 0:
		
	case 1:
		
	case 2:

 return 0;
}


}

void startPhotoGallery(){

	
	
initPhotoGal();
GLCD_Clear(Blue);

	
	GLCD_Bitmap (  0,   60, 320,  69, HEADER_pixel_data); //top header 320x69
	//GLCD_Bitmap (  0,   0, 319,  240, IRAQ_pixel_data);
	//GLCD_Bitmap (  0,   0, 260,  240, CUTSOM_pixel_data);
	//GLCD_Clear(Blue);

	for(;;){
	if (get_button() == KBD_SELECT){
		break;}
	
	if (get_button() == KBD_RIGHT){
	
			GLCD_Clear(Blue);
			refreshCanvasStart();

	
	}
	
	if (get_button() == KBD_LEFT){
	GLCD_Clear(Blue);
GLCD_Clear(Blue);

	
	GLCD_Bitmap (  0,   60, 320,  69, HEADER_pixel_data);	
	}
	
	
	
	
}

}


