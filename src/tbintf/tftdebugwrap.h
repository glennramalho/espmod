// Stop fonts etc being loaded multiple times
#ifndef _TFTDEBUGWRAP_H
#define _TFTDEBUGWRAP_H

#include <TFT_eSPI.h>
#include <systemc.h>
#include <stdint.h>

// Class functions and variables
class tftdebugwrap : public TFT_eSPI {
   public:

   tftdebugwrap(int16_t _W = TFT_WIDTH, int16_t _H = TFT_HEIGHT);
   sc_event start;
   sc_event end;
   String msg;
   int _x1, _y1, _x2, _y2;
   unsigned int fg, bg;
   int sz;
   bool skip;

   void drawChar(int32_t x, int32_t y, uint16_t c, uint32_t color, uint32_t bg,
      uint8_t size);
   void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1,
      uint32_t color);
   void drawFastVLine(int32_t x, int32_t y, int32_t h, uint32_t color);
   void drawFastHLine(int32_t x, int32_t y, int32_t w, uint32_t color);
   void fillRect(int32_t x, int32_t y, int32_t w, int32_t h,
      uint32_t color);
   int16_t drawChar(uint16_t uniCode, int32_t x, int32_t y,
      uint8_t font);
   virtual int16_t drawChar(uint16_t uniCode, int32_t x, int32_t y);
   void setWindow(int32_t xs, int32_t ys, int32_t xe, int32_t ye);
   void drawRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color);
   void drawRoundRect(int32_t x0, int32_t y0, int32_t w, int32_t h,
      int32_t radius, uint32_t color);
   void fillRoundRect(int32_t x0, int32_t y0, int32_t w, int32_t h,
      int32_t radius, uint32_t color);
   void setRotation(uint8_t r);
   void invertDisplay(boolean i);
   void drawCircle(int32_t x0, int32_t y0, int32_t r, uint32_t color);
   void fillCircle(int32_t x0, int32_t y0, int32_t r, uint32_t color);
   void drawEllipse(int16_t x0, int16_t y0, int32_t rx, int32_t ry,
      uint16_t color);
   void fillEllipse(int16_t x0, int16_t y0, int32_t rx, int32_t ry,
      uint16_t color);
   void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w,
      int16_t h, uint16_t color);
   void pushImage(int32_t x0, int32_t y0, int32_t w, int32_t h, uint16_t *data);
   void pushImage(int32_t x0, int32_t y0, int32_t w, int32_t h, uint16_t *data,
      uint16_t transparent);
   void pushImage(int32_t x0, int32_t y0, int32_t w, int32_t h,
      const uint16_t *data, uint16_t transparent);
   void pushImage(int32_t x0, int32_t y0, int32_t w, int32_t h,
      const uint16_t *data);
   void pushImage(int32_t x0, int32_t y0, int32_t w, int32_t h,
      uint8_t  *data, bool bpp8 = true);
   void pushImage(int32_t x0, int32_t y0, int32_t w, int32_t h,
      uint8_t  *data, uint8_t  transparent, bool bpp8 = true);
   int16_t drawNumber(long long_num, int32_t poX, int32_t poY, uint8_t font);
   int16_t drawNumber(long long_num, int32_t poX, int32_t poY);
   int16_t drawFloat(float floatNumber, uint8_t decimal, int32_t poX,
      int32_t poY, uint8_t font);
   int16_t drawFloat(float floatNumber, uint8_t decimal, int32_t poX,
      int32_t poY);
   int16_t drawString(const char *string, int32_t poX, int32_t poY,
      uint8_t font);
   int16_t drawString(const char *string, int32_t poX, int32_t poY);
   int16_t drawString(const String& string, int32_t poX, int32_t poY,
      uint8_t font);
   int16_t drawString(const String& string, int32_t poX, int32_t poY);
   void startcmd(const char *nm, int nx1, int ny1, int nx2, int ny2, int nfg,
      int nbg, int nsz);
   void endcmd();
   protected:
   bool pushskip(int nx1, int ny1, int nx2, int ny2, int nfg,
      int nbg, int nsz, String nm);
   void popskip(bool wasskip);
};

#endif
