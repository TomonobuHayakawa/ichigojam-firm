#include "SPI.h"

#define DISPLAY_DEVICE ILI9488
//#define DISPLAY_DEVICE	ILI9341

#if(DISPLAY_DEVICE == ILI9488)
#include "ILI9488.h"
//#define DOUBLE_CHARACTERS
#else
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#endif



#define TFT_DC 9
#define TFT_CS -1
//#define TFT_CS 10
#define TFT_RST 8

#if(DISPLAY_DEVICE == ILI9488)
ILI9488 tft = ILI9488(&SPI, TFT_DC, TFT_CS, TFT_RST);
#else
Adafruit_ILI9341 tft = Adafruit_ILI9341(&SPI, TFT_DC, TFT_CS, TFT_RST);
#endif

uint8 initflg = 0;

void tft_init() {
	if (initflg) {
		return;
	}
#if(DISPLAY_DEVICE == ILI9488)
  tft.begin(30000000); // 30MHz
#else
  tft.begin(40000000); // 40MHz
#endif
  //tft.begin(80000000); // 80MHz 化ける
 
#if(DISPLAY_DEVICE == ILI9488)
  // read diagnostics (optional but can help debug problems)
  uint8_t x = tft.readcommand8(ILI9488_RDMODE);
  x = tft.readcommand8(ILI9488_RDMADCTL);
  x = tft.readcommand8(ILI9488_RDPIXFMT);
  x = tft.readcommand8(ILI9488_RDIMGFMT);
  x = tft.readcommand8(ILI9488_RDSELFDIAG);
  //Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX);
  tft.setRotation(1);
  tft.fillScreen(ILI9488_BLACK);
  initflg = 1;
#else
  uint8_t x = tft.readcommand8(ILI9341_RDMODE);
  x = tft.readcommand8(ILI9341_RDMADCTL);
  x = tft.readcommand8(ILI9341_RDPIXFMT);
  x = tft.readcommand8(ILI9341_RDIMGFMT);
  x = tft.readcommand8(ILI9341_RDSELFDIAG);
  //Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX);
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  initflg = 1;
#endif
}

#if(DISPLAY_DEVICE == ILI9488)
#ifdef DOUBLE_CHARACTERS
uint8_t linebuf[240 * 16 * 3 * 2];
#else
uint8_t linebuf[256 * 8 * 3];
#endif
#else /*ILI9341*/
uint16_t linebuf[256 * 8];
#endif

void show() {
	
#if(DISPLAY_DEVICE == ILI9488)

#ifdef DOUBLE_CHARACTERS
	int x = 0;
	int y = 0;
#else
	int x = (480 - 256) / 2;
	int y = (320 - 8 * 24) / 2;
#endif


	const uint8* crom = CHAR_PATTERN;
	const uint8* crom2 = screen_pcg - ((0x100 - SIZE_PCG) * 8);

	uint8 invert = _g.screen_invert ? 0xff : 0; // invert or not

#ifdef DOUBLE_CHARACTERS
	tft.setAddrWindow(x, y, 480, 320);
#else
	tft.setAddrWindow(x, y, 256, 192);
#endif

	uint16_t white = ILI9488_WHITE;
	uint16_t black = ILI9488_BLACK;
	uint8* c = vram;
	for (int i = 0; i < SCREEN_H; i++) {
		for (int j = 0; j < SCREEN_W; j++) {
			int cflg = (frames & 0x10) && _g.cursorflg && j == _g.cursorx;
			uint8 cptn = invert;
			if (cflg && i == _g.cursory) {
				if (!key_flg.insert) {
					cptn ^= 0xf0;
				} else {
					cptn ^= 0xff;
				}
			}

			const uint8* cr = crom;
			if (*c >= 0x100 - SIZE_PCG) {
				cr = crom2;
			}
			cr += *c << 3;
			for (int k = 0; k < 8; k++) {
				for (int l = 0; l < 8; l++) {
					uint16_t tmp = (((*cr ^ cptn) >> (7 - l)) & 1) ? white : black;
#ifdef DOUBLE_CHARACTERS
					linebuf[(j * 16 + 480 * k*2 + l*2)*3] = (tmp >> 8) & 0xf8;
					linebuf[(j * 16 + 480 * k*2 + l*2)*3+1] = (tmp >> 3) & 0xfc;
					linebuf[(j * 16 + 480 * k*2 + l*2)*3+2] = tmp & 0xf8;
					linebuf[(j * 16 + 480 * k*2 + l*2+1)*3] = (tmp >> 8) & 0xf8;
					linebuf[(j * 16 + 480 * k*2 + l*2+1)*3+1] = (tmp >> 3) & 0xfc;
					linebuf[(j * 16 + 480 * k*2 + l*2+1)*3+2] = tmp & 0xf8;
					linebuf[(j * 16 + 480 * (k*2+1) + l*2)*3] = (tmp >> 8) & 0xf8;
					linebuf[(j * 16 + 480 * (k*2+1) + l*2)*3+1] = (tmp >> 3) & 0xfc;
					linebuf[(j * 16 + 480 * (k*2+1) + l*2)*3+2] = tmp & 0xf8;
					linebuf[(j * 16 + 480 * (k*2+1) + l*2+1)*3] = (tmp >> 8) & 0xf8;
					linebuf[(j * 16 + 480 * (k*2+1) + l*2+1)*3+1] = (tmp >> 3) & 0xfc;
					linebuf[(j * 16 + 480 * (k*2+1) + l*2+1)*3+2] = tmp & 0xf8;
#else
					linebuf[(j * 8 + 256 * k + l)*3] = (tmp >> 8) & 0xf8;
					linebuf[(j * 8 + 256 * k + l)*3+1] = (tmp >> 3) & 0xfc;
					linebuf[(j * 8 + 256 * k + l)*3+2] = tmp & 0xf8;
#endif
				}
				cr++;
			}
			c++;
		}
		tft.writePixels((uint8_t*)linebuf, sizeof(linebuf));
	}
	frames++;

#else /* ILI9341 */
	int x = (480 - 256) / 2;
	int y = (320 - 8 * 24) / 2;

	const uint8* crom = CHAR_PATTERN;
	const uint8* crom2 = screen_pcg - ((0x100 - SIZE_PCG) * 8);

	uint8 invert = _g.screen_invert ? 0xff : 0; // invert or not

	tft.setAddrWindow(x, y, 256, 192);
	uint16_t white = ILI9341_WHITE;
	uint16_t black = ILI9341_BLACK;
	uint8* c = vram;
	for (int i = 0; i < SCREEN_H; i++) {
		for (int j = 0; j < SCREEN_W; j++) {
			int cflg = (frames & 0x10) && _g.cursorflg && j == _g.cursorx;
			uint8 cptn = invert;
			if (cflg && i == _g.cursory) {
				if (!key_flg.insert) {
					cptn ^= 0xf0;
				} else {
					cptn ^= 0xff;
				}
			}

			const uint8* cr = crom;
			if (*c >= 0x100 - SIZE_PCG) {
				cr = crom2;
			}
			cr += *c << 3;
			for (int k = 0; k < 8; k++) {
				for (int l = 0; l < 8; l++) {
					linebuf[j * 8 + 256 * k + l] = (((*cr ^ cptn) >> (7 - l)) & 1) ? white : black;
				}
				cr++;
			}
			c++;
		}
		tft.writePixels((uint8_t*)linebuf, sizeof(linebuf));
	}
	frames++;
#endif
}

void video_on() {
#ifdef DOUBLE_CHARACTERS
	SCREEN_W = 30;
	SCREEN_H = 20;
#else
	SCREEN_W = 32;
	SCREEN_H = 24;
#endif
	screen_clear();
	tft_init();
}
inline void video_off(int clkdiv) {
}
inline int video_active() {
	return 0;
}

inline void IJB_lcd(uint mode) {
}

inline void video_waitSync(uint n) {
	show();
	//delay(n * 1000 / 60); // 16.6
	//delay(n * 2); // 16 -> 2 // 調整
}
