/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   toto.c                                           .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/06/13 14:10:16 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/06/13 15:37:11 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "terminal_version/ram_registers.h"

void Emulator::RenderTiles( )
{
	WORD tileData = 0;
	WORD backgroundMemory =0;
	bool unsig = true;

	// where to draw the visual area and the window
	BYTE scrollY = ReadMemory(SCY);
	BYTE scrollX = ReadMemory(SCX);
	BYTE windowY = ReadMemory(WY);
	BYTE windowX = ReadMemory(WX) - 7;

	bool usingWindow = false;

	// is the window enabled?
	if (TestBit(lcdControl, 5))
	{
		// is the current scanline we're drawing
		// within the windows Y pos?,
		if (windowY <= ReadMemory(LY))
			usingWindow = true;
	}

	// which tile data are we using?
	if (TestBit(lcdControl, 4))
	{
		tileData = 0x8000;
	}
	else
	{
		// IMPORTANT: This memory region uses signed
		// bytes as tile identifiers
		tileData = 0x8800;
		unsig = false;
	}

	// which background mem?
	if (usingWindow == false)
	{
		if (TestBit(lcdControl, 3))
			backgroundMemory = 0x9C00;
		else
			backgroundMemory = 0x9800;
	}
	else
	{
		// which window memory?
		if (TestBit(lcdControl, 6))
			backgroundMemory = 0x9C00;
		else
			backgroundMemory = 0x9800;
	}

	BYTE yPos = 0;

	// yPos is used to calculate which of 32 vertical tiles the
	// current scanline is drawing
	if (!usingWindow)
		yPos = scrollY + ReadMemory(LY);
	else
		yPos = ReadMemory(LY) - windowY;

	// which of the 8 vertical pixels of the current
	// tile is the scanline on?
	WORD tileRow = (((BYTE)(yPos / 8)) * 32);

	// time to start drawing the 160 horizontal pixels
	// for this scanline
	for (int pixel = 0; pixel < 160; pixel++)
	{
		BYTE xPos = pixel + scrollX;

		// translate the current x pos to window space if necessary
		if (usingWindow)
		{
			if (pixel >= windowX)
			{
				xPos = pixel - windowX;
			}
		}

		// which of the 32 horizontal tiles does this xPos fall within?
		WORD tileCol = (xPos / 8);
		SIGNED_WORD tileNum;

		// get the tile identity number. Remember it can be signed
		// or unsigned
		WORD tileAddrss = backgroundMemory + tileRow + tileCol;
		if(unsig)
			tileNum = (BYTE)ReadMemory(tileAddrss);
		else
			tileNum = (SIGNED_BYTE)ReadMemory(tileAddrss);

		// deduce where this tile identifier is in memory. Remember i
		// shown this algorithm earlier
		WORD tileLocation = tileData;

		if (unsig)
			tileLocation += (tileNum * 16);
		else
			tileLocation += ((tileNum + 128) * 16);

		// find the correct vertical line we're on of the
		// tile to get the tile data
		//from in memory
		BYTE line = yPos % 8;
		line *= 2; // each vertical line takes up two bytes of memory
		BYTE data1 = ReadMemory(tileLocation + line);
		BYTE data2 = ReadMemory(tileLocation + line + 1);

		// pixel 0 in the tile is it 7 of data 1 and data2.
		// Pixel 1 is bit 6 etc..
		int colourBit = xPos % 8;
		colourBit -= 7;
		colourBit *= -1;

		// combine data 2 and data 1 to get the colour id for this pixel
		// in the tile
		int colourNum = BitGetVal(data2, colourBit);
		colourNum <<= 1;
		colourNum |= BitGetVal(data1, colourBit);

		// now we have the colour id get the actual
		// colour from palette 0xFF47
		COLOUR col = GetColour(colourNum, BGP);
		int red = 0;
		int green = 0;
		int blue = 0;

		// setup the RGB values
		switch(col)
		{
			case WHITE:
				red = 255;	green = 255;	blue = 255;
				break;
			case LIGHT_GRAY:
				red = 0xCC;	green = 0xCC;	blue = 0xCC;
				break;
			case DARK_GRAY:
				red = 0x77;	green = 0x77;	blue = 0x77;
				break;
		}

		int finaly = ReadMemory(LY);

		// safety check to make sure what im about
		// to set is int the 160x144 bounds
		if ((finaly < 0) || (finaly > 143) || (pixel < 0) || (pixel > 159))
		{
			continue;
		}

		m_ScreenData[pixel][finaly][0] = red;
		m_ScreenData[pixel][finaly][1] = green;
		m_ScreenData[pixel][finaly][2] = blue;
	}
} 
