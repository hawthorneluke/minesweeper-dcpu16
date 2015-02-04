#pragma once

#include "monitor.h"
#include "dcpu-16.h"

Monitor::~Monitor()
{
	SDL_FreeSurface(surfDisplay);
	SDL_FreeSurface(surfWindow);
}

Monitor::Monitor()
{
	id = 0x7349f615;
	version = 0x1802;
	manufacturer = 0x1c6c8b36;

	frequency = 60;

	disconnected = true;


	memoryVideoStartLocation = 0x8000;
	memoryVideoFontStartLocation = 0x8180;
	memoryVideoPalletStartLocation = 0x8280;
	memoryVideoBorderColour = 0;

	useDefaultFont = true;
	useDefaultPallet = true;

	memset(fontMemory,0,sizeof(fontMemory));
	memset(palletMemory,0,sizeof(palletMemory));


	surfDisplay = NULL;

	SDL_putenv("SDL_VIDEO_CENTERED=center");

	// Set the title bar
	SDL_WM_SetCaption("DCPU-16", "");

	SDL_EnableUNICODE(1);

	screenWidth = displayCharsPerLine*(displayCharsWidth+displayCharsWidthSpace)+(displayBorderSize*2);
	screenHeight = displayLines*(displayCharsHeight+displayCharsHeightSpace)+(displayBorderSize*2);
 

    if((surfWindow = SDL_SetVideoMode((int)(screenWidth * windowSizeMultiplier), (int)(screenHeight * windowSizeMultiplier), 8, SDL_SWSURFACE)) == NULL) {
    }
	surfDisplay = SDL_CreateRGBSurface(SDL_SWSURFACE,screenWidth,screenHeight,8,0,0,0,0);
	surfDisplay = SDL_DisplayFormat(surfDisplay);


	//memset(wordDisplayCurrent, 0, sizeof(wordDisplayCurrent));
	memset(wordFontCurrent, 0, sizeof(wordFontCurrent));
	memset(wordPalletFGCurrent, 0, sizeof(wordPalletFGCurrent));
	memset(wordPalletBGCurrent, 0, sizeof(wordPalletBGCurrent));
	wordPalletBorderCurrent = 0;
	textFlashCurrent = false;


	textFlash = false;




	if((surfFont = SDL_LoadBMP("font.bmp")) == NULL)
	{
		cerr << "SDL_FillRect() Failed: " << SDL_GetError() << endl;
	}


	




	//map font
	
	if(SDL_MUSTLOCK(surfFont)) SDL_LockSurface(surfFont); //may need to lock surface

	Uint8 bpp = surfFont->format->BytesPerPixel;

	for (int i = 0; i < surfFont->h / displayCharsHeight; i++) //character line
	{
		for (int j = 0; j < surfFont->w / displayCharsWidth; j++) //character
		{
			for (int k = 0; k < wordsPerCharFont; k++) //word of character font
			{
				bool vertLine[displayCharsHeight*wordsPerCharFont];

				for (int l = 0; l < pixelLinesPerCharFontWord; l++) //left or right part of word
				{
					for (int m = 0; m < displayCharsHeight; m++) //bit of word
					{
						Uint8 *p = (Uint8*)surfFont->pixels + (((i * displayCharsHeight * surfFont->pitch) + (j * displayCharsWidth) + (k * wordsPerCharFont) + l + (m *  surfFont->pitch)) * bpp); //get pixel

						vertLine[m+(displayCharsHeight*(1-l))] = *p > 0 ? true : false; //fill high byte to low byte, going smallest bit to highest bit
					}
				}
				unsigned short word = CPU::bitsToUshort(vertLine, 0, displayCharsHeight * pixelLinesPerCharFontWord); //create word from bits
				//memoryWrite(memoryVideoFontStartLocation + k + (j * wordsPerCharFont) + (i * (surfFont->w / displayCharsWidth) * wordsPerCharFont), word);
				fontMemory[k + (j * wordsPerCharFont) + (i * (surfFont->w / displayCharsWidth) * wordsPerCharFont)] = word; //write word to font memory
			}
		}
	}

	if(SDL_MUSTLOCK(surfFont)) SDL_UnlockSurface(surfFont); //finally unlock surface



	/*
	surfFont = SDL_DisplayFormat(surfFont);

	surfFontCol = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCCOLORKEY,displayCharsWidth,displayCharsHeight,8,0,0,0,0);
	surfFontCol = SDL_DisplayFormat(surfFontCol);
	*/

	SDL_FreeSurface(surfFont); //no more need for the image so free it



	//map pallet
	palletMemory[0] = 0x000; //0
	palletMemory[1] = 0x00a; //b
	palletMemory[2] = 0x0a0; //g
	palletMemory[3] = 0x0aa; //g b
	palletMemory[4] = 0xa00; //r
	palletMemory[5] = 0xa0a; //r b
	palletMemory[6] = 0xa50; //r g
	palletMemory[7] = 0xaaa; //r g b
	palletMemory[8] = 0x555; //h
	palletMemory[9] = 0x55f; //h b
	palletMemory[10] = 0x5f5; //h g
	palletMemory[11] = 0x5ff; //h g b
	palletMemory[12] = 0xf55; //h r
	palletMemory[13] = 0xf5f; //h r b
	palletMemory[14] = 0xff5; //h r g
	palletMemory[15] = 0xfff; //h r g b


}

int Monitor::run()
{
	if (!disconnected)
	{

		bool change = false;

		//locations for character blitting source/destinations
		SDL_Rect *destRect = new SDL_Rect();
		//SDL_Rect *srcRect = new SDL_Rect();


	




		//border colour
		unsigned short wordBorder = memoryVideoBorderColour;

		unsigned short wordPalletBorder;

		if (useDefaultPallet)
		{
			wordPalletBorder = palletMemory[wordBorder];
		}
		else
		{
			wordPalletBorder = memoryRead(memoryVideoPalletStartLocation + wordBorder);
		}

		//if border colour changed (only update if border colour changed)
		if (wordPalletBorder != wordPalletBorderCurrent)
		{
			wordPalletBorderCurrent = wordPalletBorder;
			change = true;

			int r = ((wordPalletBorder >> 8) &0xf) * 16;
			int g = ((wordPalletBorder >> 4) &0xf) * 16;
			int b = (wordPalletBorder &0xf) * 16;

			//border blit
			//left
			destRect->x = 0;
			destRect->y = 0;
			destRect->w = displayBorderSize;
			destRect->h = screenHeight;

			SDL_FillRect(surfDisplay, destRect, SDL_MapRGB(surfDisplay->format, r,g,b));

			//right
			destRect->x = screenWidth-displayBorderSize;
			destRect->y = 0;
			destRect->w = displayBorderSize;
			destRect->h = screenHeight;

			SDL_FillRect(surfDisplay, destRect, SDL_MapRGB(surfDisplay->format, r,g,b));

			//top
			destRect->x = displayBorderSize;
			destRect->y = 0;
			destRect->w = screenWidth - (displayBorderSize*2);
			destRect->h = displayBorderSize;

			SDL_FillRect(surfDisplay, destRect, SDL_MapRGB(surfDisplay->format, r,g,b));

			//bottom
			destRect->x = displayBorderSize;
			destRect->y = screenHeight - displayBorderSize;
			destRect->w = screenWidth - (displayBorderSize*2);
			destRect->h = displayBorderSize;

			SDL_FillRect(surfDisplay, destRect, SDL_MapRGB(surfDisplay->format, r,g,b));
		}


	
		//for each character position on display
		for (int i = 0; i < displayLines; i++)
		{
			for (int j = 0; j < displayCharsPerLine; j++)
			{


				unsigned short word = memoryRead(memoryVideoStartLocation + (unsigned short)(j + (displayCharsPerLine * i))); //get character word to display

				unsigned short ch = word & 0x7F; //first 7 bits = character ASCII code


				//get font word data for character and check to see if that was changed
				unsigned short wordFont[wordsPerCharFont];
				bool wordFontChanged = false;

			
				for (int k = 0; k < wordsPerCharFont; k++) //two words make up a character
				{
					unsigned short d = (ch * wordsPerCharFont) + k; //location of word making up font for character

					if (useDefaultFont)
					{
						wordFont[k] = fontMemory[d];
					}
					else
					{
						wordFont[k] = memoryRead(memoryVideoFontStartLocation + d); //get the word making up the font part for the character to display
					}


					if (wordFont[k] != wordFontCurrent[i][j][k])
					{
						wordFontChanged = true;

						wordFontCurrent[i][j][k] = wordFont[k];
					}
				}

				unsigned short wordBG = (word >> 8) & 0xf;
				unsigned short wordFG = (word >> 12) & 0xf;

				unsigned short wordPalletFG;
				unsigned short wordPalletBG;


				if (useDefaultPallet)
				{
					wordPalletFG = palletMemory[wordFG];
					wordPalletBG = palletMemory[wordBG];
				}
				else
				{
					wordPalletFG = memoryRead(memoryVideoPalletStartLocation + wordFG);
					wordPalletBG = memoryRead(memoryVideoPalletStartLocation + wordBG);
				}

				//if the character has changed (only update changed characters) (or it flashed on/off, or its font data or pallet data has changed)
				if (/*(word!=wordDisplayCurrent[j + (displayCharsPerLine * i)]) ||*/ (((word>>7)&1) && (textFlash!=textFlashCurrent)) || wordFontChanged || wordPalletFGCurrent[i][j] != wordPalletFG || wordPalletBGCurrent[i][j] != wordPalletBG)
				//if ((word!=wordDisplayCurrent[j + (displayCharsPerLine * i)]) && !wordFontChanged)
				{
					//printf("%i -> %i\n", wordDisplayCurrent[j + (displayCharsPerLine * i)], word);

					//printf("%i\n",ch);
					//wordDisplayCurrent[j + (displayCharsPerLine * i)] = word;
					wordPalletFGCurrent[i][j] = wordPalletFG;
					wordPalletBGCurrent[i][j] = wordPalletBG;
				
					change = true;

			

				

					/*
					//position on font image
					srcRect->x = (ch % displayCharsPerLine) * displayCharsWidth;
					srcRect->y = ((int)ch / (int)displayCharsPerLine) * displayCharsHeight;
					srcRect->w = displayCharsWidth;
					srcRect->h = displayCharsHeight;
					*/

					//position on display
					destRect->x = j * (displayCharsWidth + displayCharsWidthSpace) + displayBorderSize;
					destRect->y = i * (displayCharsHeight + displayCharsHeightSpace) + displayBorderSize;
					destRect->w = displayCharsWidth + displayCharsWidthSpace;
					destRect->h = displayCharsHeight + displayCharsHeightSpace;


				

					int rFG = ((wordPalletFG >> 8) &0xf) * 16;
					int gFG = ((wordPalletFG >> 4) &0xf) * 16;
					int bFG = (wordPalletFG &0xf) * 16;

					int rBG = ((wordPalletBG >> 8) &0xf) * 16;
					int gBG = ((wordPalletBG >> 4) &0xf) * 16;
					int bBG = (wordPalletBG &0xf) * 16;
				


					/*
					//colour character
					if (!wordBits[7] || (wordBits[7] && textFlash))
					{
						SDL_FillRect(surfFontCol, NULL, SDL_MapRGB( surfFontCol->format, textColFG.r, textColFG.g, textColFG.b)); //set surface to character colour
						SDL_SetColorKey(surfFont, SDL_SRCCOLORKEY|SDL_RLEACCEL, SDL_MapRGB(surfFont->format, 255, 255, 255)); //make the white of the character surface transparent
						SDL_BlitSurface(surfFont, srcRect, surfFontCol, NULL); //blit character surface to one with colour
						SDL_SetColorKey(surfFontCol, SDL_SRCCOLORKEY|SDL_RLEACCEL, SDL_MapRGB(surfFontCol->format, 0, 0, 0)); //set blitted character black part to be transparent instead of the was white (now a colour) part
					}

					//background colour blit
					SDL_FillRect(surfDisplay, destRect, SDL_MapRGB(surfDisplay->format, textColBG.r, textColBG.g, textColBG.b));
				
					//character blit
					if (!wordBits[7] || (wordBits[7] && textFlash))
					{
						SDL_BlitSurface(surfFontCol, NULL, surfDisplay, destRect);
					}
					*/






					//character blit
					if(SDL_MUSTLOCK(surfDisplay)) SDL_LockSurface(surfDisplay); //may need to lock surface

					Uint8 colFG = SDL_MapRGB(surfDisplay->format, rFG, gFG, bFG);
					Uint8 colBG = SDL_MapRGB(surfDisplay->format, rBG, gBG, bBG);
					Uint8 bpp = surfDisplay->format->BitsPerPixel;
					int pixelXY = (destRect->y * surfDisplay->pitch) + (destRect->x);

					for (int k = 0; k < wordsPerCharFont; k++) //two words make up a character
					{
						for (int l = 0; l < pixelLinesPerCharFontWord; l++) //two lines make up a word
						{
							for (int m = 0; m < displayCharsHeight; m++) //the pixels in the line
							{
								Uint8 *p = (Uint8*)surfDisplay->pixels + pixelXY + (l + (k * wordsPerCharFont)) + (m * surfDisplay->pitch); //get pixel location on surface
								int place = (displayCharsHeight - (displayCharsHeight * l) + m);

								if ((wordFont[k] >> place) & 1 || (treatCharZeroAsBlank && ch == 0)) //if there's a pixel
								{
									*p = colFG; //colour it
								}
								else //if not
								{
									*p = colBG;//colour it the background colour
								}
							}
						}
					}

					if(SDL_MUSTLOCK(surfDisplay)) SDL_LockSurface(surfDisplay); //finally unlock surface


				}
			}
		}
	

		//Update the display/window
		//position on window
		if (change) //if there was a change
		{
			SDL_FillRect(surfWindow, NULL, 0); //clear window

			//calc zoom factor
			double zoomx = (double)surfWindow->w / (double)surfDisplay->w;
			double zoomy = (double)surfWindow->h / (double)surfDisplay->h;

			SDL_Surface *temp = rotozoomSurfaceXY(surfDisplay, 0, zoomx, zoomy, 0); //zoom surfDisplay creating a new surface

			SDL_BlitSurface(temp, NULL, surfWindow, NULL); //blit surfDisplay to surfWindow

			SDL_FreeSurface(temp); //delete newly created zoomed surface
		}

		SDL_Flip(surfWindow);


		delete destRect;
		//delete srcRect;


		textFlashCurrent = textFlash;

	}

	return -1;
}

void Monitor::interruptCompute()
{
	switch(registerRead(CPU::regA))
	{
		case 0: //MEM_MAP_SCREEN
		{
			unsigned short b = registerRead(CPU::regB);
			memoryVideoStartLocation = b;
			disconnected = (b == 0);
			break;
		}

		case 1: //MEM_MAP_FONT
		{
			unsigned short b = registerRead(CPU::regB);
			memoryVideoFontStartLocation = b;
			useDefaultFont = (b == 0);
			break;
		}

		case 2: //MEM_MAP_PALETTE
		{
			unsigned short b = registerRead(CPU::regB);
			memoryVideoPalletStartLocation = b;
			useDefaultPallet = (b == 0);
			break;
		}

		case 3: //SET_BORDER_COLOR
		{
			unsigned short b = registerRead(CPU::regB);
			memoryVideoBorderColour = b & 0xf;
			break;
		}

		case 4: //MEM_DUMP_FONT
		{
			unsigned short b = registerRead(CPU::regB);
			for (int i = 0; i < fontCharCount * 2; i++)
			{
				memoryWrite(b + i, fontMemory[i]);
			}
			computer->cpu->incCyclesTaken(fontCharCount * 2);
			break;
		}

		case 5: //MEM_DUMP_PALETTE
		{
			unsigned short b = registerRead(CPU::regB);
			for (int i = 0; i < palletSize; i++)
			{
				memoryWrite(b + i, palletMemory[i]);
			}
			computer->cpu->incCyclesTaken(palletSize);
			break;
		}
	}
}