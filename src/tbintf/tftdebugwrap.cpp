#include <systemc.h>
#include "tftdebugwrap.h"

tftdebugwrap::tftdebugwrap(int16_t _W, int16_t _H): TFT_eSPI(_W, _H) {
   skip = false;
}


void tftdebugwrap::drawChar(int32_t x, int32_t y, uint16_t c, uint32_t color,
      uint32_t bg, uint8_t size) {
   bool wasskip = pushskip(x, y, x, y, color, bg, sz, String(c));
   TFT_eSPI::drawChar(x, y, c, color, bg, size);
   popskip(wasskip);
}

void tftdebugwrap::drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1,
      uint32_t color) {
   bool wasskip = pushskip(x0, y0, x1, y1, color, color, 0, String("<line>"));
   TFT_eSPI::drawLine(x0, y0, x1, y1, color);
   popskip(wasskip);
}
void tftdebugwrap::drawFastVLine(int32_t x, int32_t y, int32_t h,
      uint32_t color) {
   bool wasskip = pushskip(x, y, x, y+h, color, color, 0, String("<line>"));
   TFT_eSPI::drawFastVLine(x, y, h, color);
   popskip(wasskip);
}
void tftdebugwrap::drawFastHLine(int32_t x, int32_t y, int32_t w,
      uint32_t color) {
   bool wasskip = pushskip(x, y, x+w, y, color, color, 0, String("<line>"));
   TFT_eSPI::drawFastHLine(x, y, w, color);
   popskip(wasskip);
}
void tftdebugwrap::fillRect(int32_t x, int32_t y, int32_t w, int32_t h,
      uint32_t color) {
   bool wasskip = pushskip(x, y, x+w, y+h, color, -1, 0, String("<rect>"));
   TFT_eSPI::fillRect(x, y, w, h, color);
   popskip(wasskip);
}

int16_t tftdebugwrap::drawChar(uint16_t uniCode, int32_t x, int32_t y,
      uint8_t font) {
   int16_t resp;
   bool wasskip = pushskip(x, y, x, y, textcolor, textbgcolor, font,
      String("<")+uniCode+">");
   resp = TFT_eSPI::drawChar(uniCode, x, y, font);
   popskip(wasskip);
   return resp;
}

int16_t tftdebugwrap::drawChar(uint16_t uniCode, int32_t x, int32_t y) {
   int16_t resp;
   bool wasskip = pushskip(x, y, x, y, textcolor, textbgcolor, textfont,
      String("<")+uniCode+">");
   resp = TFT_eSPI::drawChar(uniCode, x, y);
   popskip(wasskip);
   return resp;
}

void tftdebugwrap::setWindow(int32_t xs, int32_t ys, int32_t xe, int32_t ye) {
   bool wasskip = pushskip(xs, ys, xe, ye, 0, 0, 0, String("<window>"));
   TFT_eSPI::setWindow(xs, ys, xe, ye);
   popskip(wasskip);
}

void tftdebugwrap::drawRect(int32_t x, int32_t y, int32_t w, int32_t h,
      uint32_t color){
   bool wasskip = pushskip(x, y, x+w, y+h, color, 0, 0, String("<rect>"));
   TFT_eSPI::drawRect(x, y, w, h, color);
   popskip(wasskip);
}

void tftdebugwrap::drawRoundRect(int32_t x0, int32_t y0, int32_t w, int32_t h,
      int32_t radius, uint32_t color) {
   bool wasskip = pushskip(x0, y0, x0+w, y0+h, color, 0, radius,
      String("<rect>"));
   TFT_eSPI::drawRoundRect(x0,y0,w,h,radius,color);
   popskip(wasskip);
}
void tftdebugwrap::fillRoundRect(int32_t x0, int32_t y0, int32_t w, int32_t h,
      int32_t radius, uint32_t color) {
   bool wasskip = pushskip(x0, y0, x0+w, y0+h, color, 0, radius,
      String("<rect>"));
   TFT_eSPI::fillRoundRect(x0,y0,w,h,radius,color);
   popskip(wasskip);
}

void tftdebugwrap::setRotation(uint8_t r) {
   bool wasskip = pushskip(0, 0, 0, 0, 0, 0, r, String("<setrot:")+r+">");
   TFT_eSPI::setRotation(r);
   popskip(wasskip);
}

void tftdebugwrap::invertDisplay(boolean i) {
   bool wasskip = pushskip(0, 0, 0, 0, 0, 0, 0, String("<invert:")+i+">");
   TFT_eSPI::invertDisplay(i);
   popskip(wasskip);
}

void tftdebugwrap::drawCircle(int32_t x0, int32_t y0, int32_t r,
      uint32_t color) {
   bool wasskip = pushskip(x0-r, y0-r, x0+r, y0+r, color, -1, r,
      String("<circ>"));
   TFT_eSPI::drawCircle(x0,y0,r,color);
   popskip(wasskip);
}
void tftdebugwrap::fillCircle(int32_t x0, int32_t y0, int32_t r,
      uint32_t color) {
   bool wasskip = pushskip(x0-r, y0-r, x0+r, y0+r, color, color, r,
      String("<circ>"));
   TFT_eSPI::fillCircle(x0,y0,r,color);
   popskip(wasskip);
}
void tftdebugwrap::drawEllipse(int16_t x0, int16_t y0, int32_t rx, int32_t ry,
      uint16_t color) {
   bool wasskip = pushskip(x0-rx, y0-ry, x0+rx, y0+ry, color, -1, rx,
      String("<ellipse>"));
   TFT_eSPI::drawEllipse(x0,y0,rx,ry,color);
   popskip(wasskip);
}
void tftdebugwrap::fillEllipse(int16_t x0, int16_t y0, int32_t rx, int32_t ry,
      uint16_t color) {
   bool wasskip = pushskip(x0-rx, y0-ry, x0+rx, y0+ry, color, -1, rx,
      String("<ellipse>"));
   TFT_eSPI::fillEllipse(x0,y0,rx,ry,color);
   popskip(wasskip);
}

void tftdebugwrap::drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap,
      int16_t w, int16_t h, uint16_t color) {
   bool wasskip = pushskip(x, y, x+w, y+w, color, -1, 0, String("<bmp>"));
   TFT_eSPI::drawBitmap(x,y,bitmap,w,h,color);
   popskip(wasskip);
}
void tftdebugwrap::pushImage(int32_t x0, int32_t y0, int32_t w, int32_t h,
      uint16_t *data) {
   bool wasskip = pushskip(x0, y0, x0+w, y0+w, 0, 0, 0, String("<img>"));
   TFT_eSPI::pushImage(x0,y0,w,h,data);
   popskip(wasskip);
}
void tftdebugwrap::pushImage(int32_t x0, int32_t y0, int32_t w, int32_t h,
      uint16_t *data, uint16_t transparent) {
   bool wasskip = pushskip(x0, y0, x0+w, y0+w, 0, -1, 0, String("<img>"));
   TFT_eSPI::pushImage(x0,y0,w,h,data,transparent);
   popskip(wasskip);
}

void tftdebugwrap::pushImage(int32_t x0, int32_t y0, int32_t w, int32_t h,
      const uint16_t *data, uint16_t transparent) {
   bool wasskip = pushskip(x0, y0, x0+w, y0+w, 0, -1, 0, String("<img>"));
   TFT_eSPI::pushImage(x0,y0,w,h,data,transparent);
   popskip(wasskip);
}

void tftdebugwrap::pushImage(int32_t x0, int32_t y0, int32_t w, int32_t h,
      const uint16_t *data) {
   bool wasskip = pushskip(x0, y0, x0+w, y0+w, 0, 0, 0, String("<img>"));
   TFT_eSPI::pushImage(x0,y0,w,h,data);
   popskip(wasskip);
}

void tftdebugwrap::pushImage(int32_t x0, int32_t y0, int32_t w, int32_t h,
      uint8_t  *data, bool bpp8) {
   bool wasskip = pushskip(x0, y0, x0+w, y0+w, 0, 0, 0, String("<img>"));
   TFT_eSPI::pushImage(x0,y0,w,h,data,bpp8);
   popskip(wasskip);
}
void tftdebugwrap::pushImage(int32_t x0, int32_t y0, int32_t w, int32_t h,
      uint8_t  *data, uint8_t  transparent, bool bpp8) {
   bool wasskip = pushskip(x0, y0, x0+w, y0+w, 0, -1, 0, String("<img>"));
   TFT_eSPI::pushImage(x0,y0,w,h,data,transparent,bpp8);
   popskip(wasskip);
}

int16_t tftdebugwrap::drawNumber(long long_num, int32_t poX, int32_t poY,
      uint8_t font) {
   int16_t resp;
   bool wasskip = pushskip(poX, poY, poX, poY, textcolor, textbgcolor, font,
      String(long_num));
   resp = TFT_eSPI::drawNumber(long_num,poX,poY,font);
   popskip(wasskip);
   return resp;
}
int16_t tftdebugwrap::drawNumber(long long_num, int32_t poX, int32_t poY) {
   int16_t resp;
   bool wasskip = pushskip(poX, poY, poX, poY, textcolor, textbgcolor, textfont,
      String(long_num));
   resp = TFT_eSPI::drawNumber(long_num,poX,poY);
   popskip(wasskip);
   return resp;
}
int16_t tftdebugwrap::drawFloat(float floatNumber, uint8_t decimal, int32_t poX,
     int32_t poY, uint8_t font) {
   int16_t resp;
   bool wasskip = pushskip(poX, poY, poX, poY, textcolor, textbgcolor, font,
      String(floatNumber, decimal));
   resp = TFT_eSPI::drawFloat(floatNumber,decimal,poX,poY,font);
   popskip(wasskip);
   return resp;
}
int16_t tftdebugwrap::drawFloat(float floatNumber, uint8_t decimal, int32_t poX,
      int32_t poY) {
   int16_t resp;
   bool wasskip = pushskip(poX, poY, poX, poY, textcolor, textbgcolor, textfont,
      String(floatNumber, decimal));
   resp = TFT_eSPI::drawFloat(floatNumber,decimal,poX,poY,textfont);
   popskip(wasskip);
   return resp;
}

int16_t tftdebugwrap::drawString(const char *string, int32_t poX, int32_t poY,
      uint8_t font) {
   int16_t resp;
   bool wasskip = pushskip(poX, poY, poX, poY, textcolor, textbgcolor, font,
      String(string));
   resp = TFT_eSPI::drawString(string, poX, poY, font);
   popskip(wasskip);
   return resp;
}
int16_t tftdebugwrap::drawString(const char *string, int32_t poX, int32_t poY) {
   int16_t resp;
   bool wasskip = pushskip(poX, poY, poX, poY, textcolor, textbgcolor, textfont,
      String(string));
   resp = TFT_eSPI::drawString(string, poX, poY);
   popskip(wasskip);
   return resp;
}
        // Handle String type
int16_t tftdebugwrap::drawString(const String& string, int32_t poX, int32_t poY,
      uint8_t font) {
   int16_t resp;
   bool wasskip = pushskip(poX, poY, poX, poY, textcolor, textbgcolor, font,
      string);
   resp = TFT_eSPI::drawString(string, poX, poY, font);
   popskip(wasskip);
   return resp;
}
int16_t tftdebugwrap::drawString(const String& string, int32_t poX,
      int32_t poY) {
   int16_t resp;
   bool wasskip = pushskip(poX, poY, poX, poY, textcolor, textbgcolor, textfont,
      string);
   resp = TFT_eSPI::drawString(string, poX, poY);
   popskip(wasskip);
   return resp;
}

void tftdebugwrap::startcmd(const char *nm, int nx1, int ny1, int nx2, int ny2,
      int nfg, int nbg, int nsz) {
   _x1 = nx1; _y1 = ny1; _x2 = nx2; _y2 = ny2;
   fg = nfg; bg = nbg; sz = nsz;
   msg = nm;
   skip = true;
   start.notify();
}

void tftdebugwrap::endcmd() {
   skip = false;
   end.notify();
   wait(SC_ZERO_TIME);
}

bool tftdebugwrap::pushskip(int nx1, int ny1, int nx2, int ny2,
      int nfg, int nbg, int nsz, String nm) {
   bool wasskip = skip;
   if (!skip) {
      _x1 = nx1; _y1 = ny1; _x2 = nx2; _y2 = ny2;
      fg = nfg; bg = nbg; sz = nsz;
      msg = nm;
      skip = true;
      start.notify();
   }
   return wasskip;
}

void tftdebugwrap::popskip(bool wasskip) {
   skip = wasskip;
   if (!skip) {
      end.notify();
      wait(SC_ZERO_TIME);
   }
}
