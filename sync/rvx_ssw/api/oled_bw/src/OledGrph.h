/************************************************************************/
/*																		*/
/*	OledGrph.h	--	Declarations for OLED Graphics Routines				*/
/*																		*/
/************************************************************************/
/*	Author:		Gene Apperson											*/
/*	Copyright 2011, Digilent Inc.										*/
/************************************************************************/
/*  File Description:													*/
/*																		*/
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/*																		*/
/*	06/03/2011(GeneA): created											*/
/*																		*/
/************************************************************************/


/* ------------------------------------------------------------ */
/*					Miscellaneous Declarations					*/
/* ------------------------------------------------------------ */



/* ------------------------------------------------------------ */
/*					General Type Declarations					*/
/* ------------------------------------------------------------ */

/* ------------------------------------------------------------ */
/*					Object Class Declarations					*/
/* ------------------------------------------------------------ */



/* ------------------------------------------------------------ */
/*					Variable Declarations						*/
/* ------------------------------------------------------------ */



/* ------------------------------------------------------------ */
/*					Procedure Declarations						*/
/* ------------------------------------------------------------ */

void	OledSetDrawColor(unsigned char clr);
void	OledSetDrawMode(int mod);
int		OledGetDrawMode();
unsigned char *	OledGetStdPattern(int ipat);
void	OledSetFillPattern(unsigned char * pbPat);

void	OledMoveTo(int xco, int yco);
void	OledGetPos(int * pxco, int * pyco);
void	Oledoled_rgb_draw_pixel();
unsigned char	OledGetPixel();
void	OledLineTo(int xco, int yco);
void	OledDrawRect(int xco, int yco);
void	OledFillRect(int xco, int yco);
void	OledGetBmp(int dxco, int dyco, unsigned char * pbBmp);
void	OledPutBmp(int dxco, int dyco, unsigned char * pbBmp);
void	OledDrawChar(char ch);
void	OledDrawString(char * sz);

/* ------------------------------------------------------------ */

/************************************************************************/
