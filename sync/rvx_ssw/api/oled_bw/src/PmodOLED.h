/************************************************************************/
/*																		*/
/*	oled.h	--	Interface Declarations for OLED Display Driver Module	*/
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
/*	04/29/2011(GeneA): created											*/
/*																		*/
/************************************************************************/

#if !defined(PMODOLED_INC)
#define	PMODOLED_INC

/* ------------------------------------------------------------ */
/*					Miscellaneous Declarations					*/
/* ------------------------------------------------------------ */

#define	cbOledDispMax	512		//max number of bytes in display buffer

#define	ccolOledMax		128		//number of display columns
#define	crowOledMax		32		//number of display rows
#define	cpagOledMax		4		//number of display memory pages

#define	cbOledChar		8		//font glyph definitions is 8 bytes long
#define	chOledUserMax	0x20	//number of character defs in user font table
#define	cbOledFontUser	(chOledUserMax*cbOledChar)

/* Graphics drawing modes.
*/
#define	modOledSet		0
#define	modOledOr		1
#define	modOledAnd		2
#define	modOledXor		3

/* ------------------------------------------------------------ */
/*					General Type Declarations					*/
/* ------------------------------------------------------------ */

/* Pin definitions for access to OLED control signals on Cerebot 32MX4
   If one wishes to convert to using the Cerebo 32MX7 then these definitions
   need to be defined to new values
*/
#define prtSelect	IOPORT_G
#define	prtDataCmd	IOPORT_B
#define	prtReset	IOPORT_D
#define	prtVbatCtrl IOPORT_D
#define	prtVddCtrl	IOPORT_B

#define bitSelect	BIT_9
#define bitDataCmd	BIT_15
#define	bitReset	BIT_5
#define	bitVbatCtrl	BIT_4
#define	bitVddCtrl	BIT_14

/* ------------------------------------------------------------ */
/*					Object Class Declarations					*/
/* ------------------------------------------------------------ */



/* ------------------------------------------------------------ */
/*					Variable Declarations						*/
/* ------------------------------------------------------------ */



/* ------------------------------------------------------------ */
/*					Procedure Declarations						*/
/* ------------------------------------------------------------ */

void	OledInit();
void	OledClear();
void	OledClearBuffer();
void	OledUpdate();
void	oled_bw_config_spi();

/* ------------------------------------------------------------ */

#endif

/************************************************************************/
