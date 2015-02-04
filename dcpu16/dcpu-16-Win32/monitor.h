#include "SDL_rotozoom.h"
#include "hardware.h"

#define displayLines 12
#define displayCharsPerLine 32
#define displayCharsWidth 4 //9
#define displayCharsWidthSpace 0
#define displayCharsHeight 8 //16
#define displayCharsHeightSpace 0
#define displayBorderSize 8
#define wordsPerCharFont 2
#define pixelLinesPerCharFontWord 2
#define fontCharCount 128
#define palletSize 16

#define windowSizeMultiplier 3

class Monitor : public Hardware
{
private:
	static const bool fontChangeBlackToWhite = false;
	static const bool treatCharZeroAsBlank = false;

	int screenWidth, screenHeight;

	bool disconnected;

	SDL_Surface* surfWindow;
	SDL_Surface* surfDisplay;
	SDL_Surface* surfFont;
	SDL_Surface* surfFontCol;

	unsigned short memoryVideoStartLocation;
	unsigned short memoryVideoFontStartLocation;
	unsigned short memoryVideoPalletStartLocation;
	unsigned short memoryVideoBorderColour;

	unsigned short fontMemory[fontCharCount * 2];

	unsigned short palletMemory[palletSize];

	bool useDefaultFont;
	bool useDefaultPallet;

	static const int fpsScreen = 60;
	static const int fpsTextFlash = 3;
	
	//unsigned short wordDisplayCurrent[displayLines * displayCharsPerLine];
	unsigned short wordFontCurrent[displayLines][displayCharsPerLine][wordsPerCharFont]; //character location on display y, x, font data byte
	unsigned short wordPalletFGCurrent[displayLines][displayCharsPerLine];
	unsigned short wordPalletBGCurrent[displayLines][displayCharsPerLine];
	unsigned short wordPalletBorderCurrent;
	bool textFlashCurrent;

	bool textFlash;

public:
	Monitor();
	~Monitor();
	int run();
	void interruptCompute();

};