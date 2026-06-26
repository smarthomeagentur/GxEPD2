#include "GxEPD2_1330c_EL133UF3.h"

GxEPD2_1330c_EL133UF3::GxEPD2_1330c_EL133UF3(int16_t cs, int16_t cs_slave, int16_t dc, int16_t rst, int16_t busy) : GxEPD2_EPD(cs, dc, rst, busy, LOW, 60000000, WIDTH, HEIGHT, panel, hasColor, hasPartialUpdate, hasFastPartialUpdate) {
   _paged = false;  //?
   _reset_duration = 30;
   _cs_slave = cs_slave;
}

void GxEPD2_1330c_EL133UF3::init(uint32_t serial_diag_bitrate) {
   _use_alt_init = false;
   init(serial_diag_bitrate, true, 30, false);
}

void GxEPD2_1330c_EL133UF3::initAlt(uint32_t serial_diag_bitrate) {
   _use_alt_init = true;
   init(serial_diag_bitrate, true, 30, false);
}

void GxEPD2_1330c_EL133UF3::init(uint32_t serial_diag_bitrate, bool initial, uint16_t reset_duration, bool pulldown_rst_mode) {
   _initial_write = initial;
   _initial_refresh = initial;
   _pulldown_rst_mode = pulldown_rst_mode;
   _power_is_on = false;
   _using_partial_mode = false;
   _hibernating = false;
   _init_display_done = false;
   _reset_duration = reset_duration;
   _paging_step = 0;

   if (serial_diag_bitrate > 0) {
      Serial.begin(serial_diag_bitrate);
      _diag_enabled = true;
   }
   if (_cs >= 0) {
      pinMode(_cs, OUTPUT);
      digitalWrite(_cs, HIGH);  // set (needed e.g. for RP2040)
   }
   if (_cs_slave >= 0) {
      pinMode(_cs_slave, OUTPUT);
      digitalWrite(_cs_slave, HIGH);
   }
   if (_busy >= 0) {
      pinMode(_busy, INPUT_PULLUP);
   }

   // _pSPIx->begin(); // may steal _rst pin (Waveshare Pico-ePaper-2.9) REMOVED: User calls SPI.begin() with custom pins.

   if (_rst >= 0) {
      pinMode(_rst, OUTPUT);
      digitalWrite(_rst, HIGH);  // set (needed e.g. for RP2040)
   }
   if (_cs >= 0) {
      pinMode(_cs, OUTPUT);
      digitalWrite(_cs, HIGH);  // set (needed e.g. for RP2040)
   }
   if (_cs_slave >= 0) {
      pinMode(_cs_slave, OUTPUT);
      digitalWrite(_cs_slave, HIGH);  // set (needed e.g. for RP2040)
   }
   if (_dc >= 0) {
      pinMode(_dc, OUTPUT);
      digitalWrite(_dc, HIGH);
   }
}

void GxEPD2_1330c_EL133UF3::enableQuickRefresh(int16_t refresh_stop_time, bool enable) {
   // TODO: needs to be implemented
   _epd_quick = enable;
   _epd_quick_stop_time = refresh_stop_time;

   if (!_epd_quick) {
      pinMode(_rst, OUTPUT);  // just in case
      digitalWrite(_rst, HIGH);
      delay(50);  // needs a little longer
      digitalWrite(_rst, LOW);
      delay(20);
      digitalWrite(_rst, HIGH);
      delay(10);  // 4ms measured
   }
}

void GxEPD2_1330c_EL133UF3::clearScreen(uint8_t value) {
   writeScreenBuffer(value);
}

void GxEPD2_1330c_EL133UF3::clearScreen(uint8_t black_value, uint8_t color_value) {
   writeScreenBuffer(black_value, color_value);
}

void GxEPD2_1330c_EL133UF3::writeScreenBuffer(uint8_t value) {
   writeScreenBuffer(value, 0xFF);
}

void GxEPD2_1330c_EL133UF3::writeScreenBuffer(uint8_t color_set, uint8_t color_value) {
   uint8_t color = (color_set << 4) | (color_set & 0x0F);

   if (!_init_display_done) {
      if (_use_alt_init) {
         _InitDisplayAlt();
      } else {
         _InitDisplay();
      }
   }

   // _powerOn(); // REMOVED: Power On should happen in refresh(), after data is written.
   delay(1);
   _writeColor(color, CsType::CS_MASTER);
   delay(1);
   _writeColor(color, CsType::CS_SLAVE);

   _waitWhileBusy();
}

void GxEPD2_1330c_EL133UF3::writeImage(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm) {
   if (!_init_display_done)
      if (_use_alt_init) {
         _InitDisplayAlt();
      } else {
         _InitDisplay();
      }
   int16_t w_half = WIDTH / 2;
   int16_t bytes_per_line_half = w_half / 2;

   if (_paged && (x == 0) && (w == int16_t(WIDTH)) && (h < int16_t(HEIGHT))) {
      int16_t bytes_per_line_bitmap = w / 2;

      if (_paging_step == 1) {  // MASTER
         _pSPIx->beginTransaction(_spi_settings);
         if (y == 0) {
            _set_cs(CsType::CS_MASTER, LOW);
            _pSPIx->transfer(DTM);
         }

         for (int16_t row = 0; row < h; row++) {
            const uint8_t* val_ptr = bitmap + (row * bytes_per_line_bitmap);
            for (int16_t col_byte = 0; col_byte < bytes_per_line_half; col_byte++) {
               uint8_t val = pgm ? pgm_read_byte(&val_ptr[col_byte]) : val_ptr[col_byte];
               val = _remap_byte(val);
               if (invert)
                  val = ~val;
               _pSPIx->transfer(val);
            }
         }
         if (y + h == HEIGHT) {
            _set_cs(CsType::CS_MASTER, HIGH);
            _paged = false;
         }
         _pSPIx->endTransaction();
      } else if (_paging_step == 2)  // SLAVE
      {
         _pSPIx->beginTransaction(_spi_settings);
         if (y == 0) {
            _set_cs(CsType::CS_SLAVE, LOW);
            _pSPIx->transfer(DTM);
         }

         for (int16_t row = 0; row < h; row++) {
            const uint8_t* val_ptr = bitmap + (row * bytes_per_line_bitmap);
            for (int16_t col_byte = 0; col_byte < bytes_per_line_half; col_byte++) {
               uint8_t val =
                   pgm ? pgm_read_byte(&val_ptr[bytes_per_line_half + col_byte]) : val_ptr[bytes_per_line_half + col_byte];
               val = _remap_byte(val);
               if (invert)
                  val = ~val;
               _pSPIx->transfer(val);
            }
         }
         if (y + h == HEIGHT) {
            _set_cs(CsType::CS_SLAVE, HIGH);
            _paged = false;
         }
         _pSPIx->endTransaction();
      }
   } else {
      _paged = false;
      int16_t wb = (w + 1) / 2;  // bytes per line input (2 pix per byte)
      w += x % 4;
      x -= x % 4;
      w = 4 * ((w + 3) / 4);

      h += y % 2;
      y -= y % 2;
      h = 2 * ((h + 1) / 2);

      if ((w <= 0) || (h <= 0))
         return;

      // Master Chip
      int16_t x_master = x;
      int16_t w_master = w;
      if (x_master < 600) {
          if (x_master + w_master > 600) w_master = 600 - x_master;
          _setPartialRamArea(x_master, y, w_master, h, CsType::CS_MASTER);
          
          _pSPIx->beginTransaction(_spi_settings);
          _set_cs(CsType::CS_MASTER, LOW);
          _pSPIx->transfer(DTM);
          
          for (int16_t i = 0; i < h; i++) {
             for (int16_t j = 0; j < w_master; j += 2) {
                int16_t bitmap_x = x_master + j - x;
                int16_t bitmap_y = i;
                uint32_t idx = mirror_y ? (bitmap_x / 2) + uint32_t((h - 1 - bitmap_y)) * wb
                                        : (bitmap_x / 2) + uint32_t(bitmap_y) * wb;
                uint8_t data = pgm ? pgm_read_byte(&bitmap[idx]) : bitmap[idx];
                data = _remap_byte(data);
                if (invert) data = ~data;
                _pSPIx->transfer(data);
             }
          }
          _set_cs(CsType::CS_MASTER, HIGH);
          _pSPIx->endTransaction();
      }

      // Slave Chip
      int16_t x_slave = x;
      int16_t w_slave = w;
      if (x_slave + w_slave > 600) {
          if (x_slave < 600) {
              w_slave = w_slave - (600 - x_slave);
              x_slave = 600;
          }
          _setPartialRamArea(x_slave - 600, y, w_slave, h, CsType::CS_SLAVE);
          
          _pSPIx->beginTransaction(_spi_settings);
          _set_cs(CsType::CS_SLAVE, LOW);
          _pSPIx->transfer(DTM);
          
          for (int16_t i = 0; i < h; i++) {
             for (int16_t j = 0; j < w_slave; j += 2) {
                int16_t bitmap_x = x_slave + j - x;
                int16_t bitmap_y = i;
                uint32_t idx = mirror_y ? (bitmap_x / 2) + uint32_t((h - 1 - bitmap_y)) * wb
                                        : (bitmap_x / 2) + uint32_t(bitmap_y) * wb;
                uint8_t data = pgm ? pgm_read_byte(&bitmap[idx]) : bitmap[idx];
                data = _remap_byte(data);
                if (invert) data = ~data;
                _pSPIx->transfer(data);
             }
          }
          _set_cs(CsType::CS_SLAVE, HIGH);
          _pSPIx->endTransaction();
      }
   }
}

void GxEPD2_1330c_EL133UF3::writeImagePart(const uint8_t bitmap[], int16_t x_part, int16_t y_part, int16_t w_bitmap,
                                           int16_t h_bitmap, int16_t x, int16_t y, int16_t w, int16_t h, bool invert,
                                           bool mirror_y, bool pgm) {
   if (!_init_display_done)
      if (_use_alt_init) {
         _InitDisplayAlt();
      } else {
         _InitDisplay();
      }
   
   if ((w_bitmap < 0) || (h_bitmap < 0) || (w < 0) || (h < 0)) return;
   if ((x_part < 0) || (x_part >= w_bitmap)) return;
   if ((y_part < 0) || (y_part >= h_bitmap)) return;
   
   int16_t wb_bitmap = (w_bitmap + 1) / 2; // width bytes, bitmaps are padded
   x_part -= x_part % 4;
   w = w_bitmap - x_part < w ? w_bitmap - x_part : w; // limit
   h = h_bitmap - y_part < h ? h_bitmap - y_part : h; // limit
   w += x % 4;
   x -= x % 4;
   w = 4 * ((w + 3) / 4);

   h += y % 2;
   y -= y % 2;
   h = 2 * ((h + 1) / 2);

   if ((w <= 0) || (h <= 0))
      return;

   // Master Chip
   int16_t x_master = x;
   int16_t w_master = w;
   if (x_master < 600) {
       if (x_master + w_master > 600) w_master = 600 - x_master;
       _setPartialRamArea(x_master, y, w_master, h, CsType::CS_MASTER);
       
       _pSPIx->beginTransaction(_spi_settings);
       _set_cs(CsType::CS_MASTER, LOW);
       _pSPIx->transfer(DTM);
       
       for (int16_t i = 0; i < h; i++) {
          for (int16_t j = 0; j < w_master; j += 2) {
             int16_t bitmap_x = x_master + j - x;
             int16_t bitmap_y = i;
             uint32_t idx = mirror_y ? (x_part / 2) + (bitmap_x / 2) + uint32_t((h_bitmap - 1 - (y_part + bitmap_y))) * wb_bitmap
                                     : (x_part / 2) + (bitmap_x / 2) + uint32_t(y_part + bitmap_y) * wb_bitmap;
             uint8_t data = pgm ? pgm_read_byte(&bitmap[idx]) : bitmap[idx];
             data = _remap_byte(data);
             if (invert) data = ~data;
             _pSPIx->transfer(data);
          }
       }
       _set_cs(CsType::CS_MASTER, HIGH);
       _pSPIx->endTransaction();
   }

   // Slave Chip
   int16_t x_slave = x;
   int16_t w_slave = w;
   if (x_slave + w_slave > 600) {
       if (x_slave < 600) {
           w_slave = w_slave - (600 - x_slave);
           x_slave = 600;
       }
       _setPartialRamArea(x_slave - 600, y, w_slave, h, CsType::CS_SLAVE);
       
       _pSPIx->beginTransaction(_spi_settings);
       _set_cs(CsType::CS_SLAVE, LOW);
       _pSPIx->transfer(DTM);
       
       for (int16_t i = 0; i < h; i++) {
          for (int16_t j = 0; j < w_slave; j += 2) {
             int16_t bitmap_x = x_slave + j - x;
             int16_t bitmap_y = i;
             uint32_t idx = mirror_y ? (x_part / 2) + (bitmap_x / 2) + uint32_t((h_bitmap - 1 - (y_part + bitmap_y))) * wb_bitmap
                                     : (x_part / 2) + (bitmap_x / 2) + uint32_t(y_part + bitmap_y) * wb_bitmap;
             uint8_t data = pgm ? pgm_read_byte(&bitmap[idx]) : bitmap[idx];
             data = _remap_byte(data);
             if (invert) data = ~data;
             _pSPIx->transfer(data);
          }
       }
       _set_cs(CsType::CS_SLAVE, HIGH);
       _pSPIx->endTransaction();
   }
}

void GxEPD2_1330c_EL133UF3::writeImage(const uint8_t* black, const uint8_t* color, int16_t x, int16_t y, int16_t w,
                                       int16_t h, bool invert, bool mirror_y, bool pgm) {
}

void GxEPD2_1330c_EL133UF3::writeImagePart(const uint8_t* black, const uint8_t* color, int16_t x_part, int16_t y_part,
                                           int16_t w_bitmap, int16_t h_bitmap, int16_t x, int16_t y, int16_t w,
                                           int16_t h, bool invert, bool mirror_y, bool pgm) {
}

void GxEPD2_1330c_EL133UF3::writeImageFromPSRAM(const uint8_t* buffer, int16_t x, int16_t y, int16_t w, int16_t h, uint8_t rotation) {
   if (!_init_display_done)
      if (_use_alt_init) {
         _InitDisplayAlt();
      } else {
         _InitDisplay();
      }

   _paged = false;
   int16_t wb = w / 2 + (w % 2 != 0 ? 1 : 0);  // bytes per line input (2 pix per byte)

   // Direct map from PSRAM color index to Hardware color index
   static const uint8_t psram_to_hw_lut[16] = {
       0x0, 0x5, 0x6, 0x3, 0x3, 0x2, 0x1, 0x1,
       0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1};

   auto getPixel = [&](int16_t hw_x, int16_t hw_y) -> uint8_t {
      int16_t log_x, log_y;
      if (rotation == 1) {  // 90 CW
         log_x = hw_y;
         log_y = 1199 - hw_x;
      } else if (rotation == 2) {  // 180
         log_x = 1199 - hw_x;
         log_y = 1599 - hw_y;
      } else if (rotation == 3) {  // 270 CW / 90 CCW
         log_x = 1599 - hw_y;
         log_y = hw_x;
      } else {  // 0
         log_x = hw_x;
         log_y = hw_y;
      }

      if (log_x >= x && log_x < x + w && log_y >= y && log_y < y + h) {
         int16_t bx = log_x - x;
         int16_t by = log_y - y;
         uint8_t raw = buffer[(bx / 2) + by * wb];
         uint8_t p = (bx % 2 == 0) ? (raw >> 4) : (raw & 0x0F);
         return psram_to_hw_lut[p & 0x0F];
      }
      return 0x1;  // Clean/White
   };

   // Master
   _pSPIx->beginTransaction(_spi_settings);
   _set_cs(CsType::CS_MASTER, LOW);
   _pSPIx->transfer(DTM);

   for (int16_t i = 0; i < int16_t(HEIGHT); i++) {
      for (int16_t byte_idx = 0; byte_idx < 300; byte_idx++) {
         int16_t hw_x1 = byte_idx * 2;
         int16_t hw_x2 = hw_x1 + 1;

         uint8_t c1 = getPixel(hw_x1, i);
         uint8_t c2 = getPixel(hw_x2, i);

         _pSPIx->transfer((c1 << 4) | c2);
      }
   }
   _set_cs(CsType::CS_MASTER, HIGH);
   _pSPIx->endTransaction();

   // Slave
   _pSPIx->beginTransaction(_spi_settings);
   _set_cs(CsType::CS_SLAVE, LOW);
   _pSPIx->transfer(DTM);

   for (int16_t i = 0; i < int16_t(HEIGHT); i++) {
      for (int16_t byte_idx = 0; byte_idx < 300; byte_idx++) {
         int16_t hw_x1 = 600 + byte_idx * 2;
         int16_t hw_x2 = hw_x1 + 1;

         uint8_t c1 = getPixel(hw_x1, i);
         uint8_t c2 = getPixel(hw_x2, i);

         _pSPIx->transfer((c1 << 4) | c2);
      }
   }
   _set_cs(CsType::CS_SLAVE, HIGH);
   _pSPIx->endTransaction();
}

void GxEPD2_1330c_EL133UF3::writeNative(const uint8_t* data1, const uint8_t* data2, int16_t x, int16_t y, int16_t w,
                                        int16_t h, bool invert, bool mirror_y, bool pgm) {
   if (data2)
      return;
   writeImage(data1, x, y, w, h, invert, mirror_y, pgm);
}

void GxEPD2_1330c_EL133UF3::writeNativePart(const uint8_t* data1, const uint8_t* data2, int16_t x_part, int16_t y_part,
                                            int16_t w_bitmap, int16_t h_bitmap, int16_t x, int16_t y, int16_t w,
                                            int16_t h, bool invert, bool mirror_y, bool pgm) {
   if (data2)
      return;
   writeImagePart(data1, x_part, y_part, w_bitmap, h_bitmap, x, y, w, h, invert, mirror_y, pgm);
}

void GxEPD2_1330c_EL133UF3::drawImage(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert,
                                      bool mirror_y, bool pgm) {
}

void GxEPD2_1330c_EL133UF3::drawImagePart(const uint8_t bitmap[], int16_t x_part, int16_t y_part, int16_t w_bitmap,
                                          int16_t h_bitmap, int16_t x, int16_t y, int16_t w, int16_t h, bool invert,
                                          bool mirror_y, bool pgm) {
}

void GxEPD2_1330c_EL133UF3::drawImage(const uint8_t* black, const uint8_t* color, int16_t x, int16_t y, int16_t w,
                                      int16_t h, bool invert, bool mirror_y, bool pgm) {
}

void GxEPD2_1330c_EL133UF3::drawImagePart(const uint8_t* black, const uint8_t* color, int16_t x_part, int16_t y_part,
                                          int16_t w_bitmap, int16_t h_bitmap, int16_t x, int16_t y, int16_t w,
                                          int16_t h, bool invert, bool mirror_y, bool pgm) {
}

void GxEPD2_1330c_EL133UF3::drawNative(const uint8_t* data1, const uint8_t* data2, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm) {
}

void GxEPD2_1330c_EL133UF3::writeDemoBitmap(const uint8_t* data1, const uint8_t* data2, int16_t x, int16_t y, int16_t w, int16_t h, int16_t mode, bool mirror_y, bool pgm) {
}

void GxEPD2_1330c_EL133UF3::drawDemoBitmap(const uint8_t* data1, const uint8_t* data2, int16_t x, int16_t y, int16_t w,
                                           int16_t h, int16_t mode, bool mirror_y, bool pgm) {
}

void GxEPD2_1330c_EL133UF3::refresh(bool partial_update_mode) {
   if (_suspend_refresh) return;
   if (_paging_step == 1)
      return;
   _powerOn();
   _waitWhileBusy();  // for the love of god, please do not remove this
   delay(30);

   if (!partial_update_mode) {
       _clearPartialRamArea(CsType::CS_MASTER_SLAVE);
   }

   _drf(CsType::CS_MASTER_SLAVE);
   delay(1);
   if (_epd_quick) {
      delay(_epd_quick_stop_time);  // Time until force stop refresh
      pinMode(_rst, OUTPUT);        // just in case
      digitalWrite(_rst, HIGH);
      delay(50);  // needs a little longer
      digitalWrite(_rst, LOW);
      delay(20);
      digitalWrite(_rst, HIGH);
      delay(10);  // 4ms measured
   }
   _waitWhileBusy("refresh", full_refresh_time);
   _paging_step = 0;
}

void GxEPD2_1330c_EL133UF3::refresh(int16_t x, int16_t y, int16_t w, int16_t h) {
   if (_suspend_refresh) return;
   if (_paging_step == 1)
      return;
   _powerOn();
   _waitWhileBusy();
   delay(30);

   int16_t x_master = x;
   int16_t w_master = w;
   CsType refresh_cs = CsType::CS_NONE;

   if (x_master < 600) {
       if (x_master + w_master > 600) w_master = 600 - x_master;
       _setPartialRamArea(x_master, y, w_master, h, CsType::CS_MASTER);
       refresh_cs = refresh_cs | CsType::CS_MASTER;
   } else {
       // Master is not part of the visual update, BUT it must execute a refresh cycle
       // in order to generate the high-voltage DCDC power for the Slave controller!
       // We set a tiny 2x2 dummy window on the Master so it doesn't wipe the screen.
       _setPartialRamArea(0, 0, 2, 2, CsType::CS_MASTER);
       refresh_cs = refresh_cs | CsType::CS_MASTER;
   }

   int16_t x_slave = x;
   int16_t w_slave = w;
   if (x_slave + w_slave > 600) {
       if (x_slave < 600) {
           w_slave = w_slave - (600 - x_slave);
           x_slave = 600;
       }
       _setPartialRamArea(x_slave - 600, y, w_slave, h, CsType::CS_SLAVE);
       refresh_cs = refresh_cs | CsType::CS_SLAVE;
   } else {
       // Slave is not part of the visual update, BUT we set a tiny 2x2 dummy window
       // to keep the panel refresh synchronized and prevent weird glitches/pulses on the right side.
       _setPartialRamArea(0, 0, 2, 2, CsType::CS_SLAVE);
       refresh_cs = refresh_cs | CsType::CS_SLAVE;
   }

   if (refresh_cs != CsType::CS_NONE) {
       _drf(refresh_cs);
       delay(1);
       if (_epd_quick) {
          delay(_epd_quick_stop_time);  // Time until force stop refresh
          pinMode(_rst, OUTPUT);        // just in case
          digitalWrite(_rst, HIGH);
          delay(50);  // needs a little longer
          digitalWrite(_rst, LOW);
          delay(20);
          digitalWrite(_rst, HIGH);
          delay(10);  // 4ms measured
       }
       _waitWhileBusy("refresh", full_refresh_time);
   }
   _paging_step = 0;
}

void GxEPD2_1330c_EL133UF3::powerOff() {
   if (_paging_step == 1)
      return;
   _pof(CsType::CS_MASTER_SLAVE);
   _waitWhileBusy("powerOff", power_off_time);
   _power_is_on = false;
}

void GxEPD2_1330c_EL133UF3::hibernate() {
   powerOff();
   _writeEN133UF3DataCmd(SLEEP, SLEEP_V, sizeof(SLEEP_V), CsType::CS_MASTER_SLAVE);
}

void GxEPD2_1330c_EL133UF3::setPaged() {
   _paged = true;
   _initial_write = false;
   _initial_refresh = false;
   _paging_step = (_paging_step % 2) + 1;
}

void GxEPD2_1330c_EL133UF3::_writeEN133UF3DataCmd(uint8_t cmd, const uint8_t* data, const uint8_t data_len, CsType cs_type) {
   _pSPIx->beginTransaction(_spi_settings);
   _set_cs(cs_type, LOW);
   _pSPIx->transfer(cmd);
   _pSPIx->transferBytes(data, NULL, data_len);
   _set_cs(cs_type, HIGH);
   _pSPIx->endTransaction();
}

void GxEPD2_1330c_EL133UF3::_writeEN133UF3Cmd(uint8_t cmd, CsType cs_type) {
   _pSPIx->beginTransaction(_spi_settings);
   _set_cs(cs_type, LOW);
   _pSPIx->transfer(cmd);
   _set_cs(cs_type, HIGH);
   _pSPIx->endTransaction();
}

inline void GxEPD2_1330c_EL133UF3::_psr(CsType cs_type) {
   _writeEN133UF3DataCmd(PSR, PSR_V, sizeof(PSR_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_pwr(CsType cs_type) {
   _writeEN133UF3DataCmd(PWR_epd, PWR_V, sizeof(PWR_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_pof(CsType cs_type) {
   _writeEN133UF3DataCmd(POF, POF_V, sizeof(POF_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_pon(CsType cs_type) {
   _writeEN133UF3Cmd(PON, cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_drf(CsType cs_type) {
   _writeEN133UF3DataCmd(DRF, DRF_V, sizeof(DRF_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_cdi(CsType cs_type) {
   _writeEN133UF3DataCmd(CDI, CDI_V, sizeof(CDI_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_tcon(CsType cs_type) {
   _writeEN133UF3DataCmd(TCON, TCON_V, sizeof(TCON_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_tres(CsType cs_type) {
   _writeEN133UF3DataCmd(TRES, TRES_V, sizeof(TRES_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_cmd66(CsType cs_type) {
   _writeEN133UF3DataCmd(CMD66, CMD66_V, sizeof(CMD66_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_en_buf(CsType cs_type) {
   _writeEN133UF3DataCmd(EN_BUF, EN_BUF_V, sizeof(EN_BUF_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_ccset(CsType cs_type) {
   _writeEN133UF3DataCmd(CCSET, CCSET_V, sizeof(CCSET_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_pws(CsType cs_type) {
   _writeEN133UF3DataCmd(PWS, PWS_V, sizeof(PWS_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_an_tm(CsType cs_type) {
   _writeEN133UF3DataCmd(AN_TM, AN_TM_V, sizeof(AN_TM_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_agid(CsType cs_type) {
   _writeEN133UF3DataCmd(AGID, AGID_V, sizeof(AGID_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_btst_p(CsType cs_type) {
   _writeEN133UF3DataCmd(BTST_P, BTST_P_V, sizeof(BTST_P_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_btst_n(CsType cs_type) {
   _writeEN133UF3DataCmd(BTST_N, BTST_N_V, sizeof(BTST_N_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_boost_vddp_en(CsType cs_type) {
   _writeEN133UF3DataCmd(BOOST_VDDP_EN, BOOST_VDDP_EN_V, sizeof(BOOST_VDDP_EN_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_buck_boost_vddn(CsType cs_type) {
   _writeEN133UF3DataCmd(BUCK_BOOST_VDDN, BUCK_BOOST_VDDN_V, sizeof(BUCK_BOOST_VDDN_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_tft_vcom_power(CsType cs_type) {
   _writeEN133UF3DataCmd(TFT_VCOM_POWER, TFT_VCOM_POWER_V, sizeof(TFT_VCOM_POWER_V), cs_type);
}

void GxEPD2_1330c_EL133UF3::_writeColor(uint8_t color_value, CsType cs_type) {
   uint8_t remapped = _remap_byte(color_value);
   _pSPIx->beginTransaction(_spi_settings);
   _set_cs(cs_type, LOW);
   _pSPIx->transfer(DTM);

   if (color_value <= 0xAA) {
      for (uint16_t y = 0; y < HEIGHT; y++) {
         for (uint16_t x = 0; x < HALF_WIDTH / 2; x++) {
            _pSPIx->transfer(remapped);
         }
      }
      _set_cs(cs_type, HIGH);
      _pSPIx->endTransaction();
      return;
   }

   int colorSet = 0x00;
   for (uint16_t aa = 0; aa < 17; aa++) {
      for (uint16_t y = 0; y < HEIGHT / 17; y++) {
         for (uint16_t x = 0; x < HALF_WIDTH / 2; x++) {
            _pSPIx->transfer(colorSet);
         }
      }
      colorSet = colorSet + 0x11;
      if (colorSet > 0xFF)
         colorSet = 0xFF;
   }
   _set_cs(cs_type, HIGH);
   _pSPIx->endTransaction();
}

uint8_t GxEPD2_1330c_EL133UF3::_remap_byte(uint8_t byte) {
   // Mapping:
   // Standard GxEPD2 | EL133UF3
   // 0 Black         | 0 Black
   // 1 White         | 1 White
   // 2 Green         | 6 Green
   // 3 Blue          | 5 Blue
   // 4 Red           | 3 Red
   // 5 Yellow        | 2 Yellow
   // 6 Orange        | 4 Orange
   // 7 (Clean/User)  | 7 (Clean?)

   static const uint8_t lut[16] = {
       0x0,  // 0 -> 0
       0x1,  // 1 -> 1
       0x6,  // 2 -> 6
       0x5,  // 3 -> 5
       0x3,  // 4 -> 3
       0x2,  // 5 -> 2
       0x4,  // 6 -> 4
       0x7,  // 7 -> 7 ?
       // If > 7 (garbage or unused bits from GxEPD2 buffer packing issues?)
       // Map identity?
       0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF};

   uint8_t h = (byte >> 4) & 0x0F;
   uint8_t l = byte & 0x0F;
   return (lut[h] << 4) | lut[l];
}

void GxEPD2_1330c_EL133UF3::_InitDisplay() {
   _reset();

   _waitWhileBusy("initReset", power_off_time);

   _an_tm(CsType::CS_MASTER);
   _cmd66(CsType::CS_MASTER_SLAVE);
   _psr(CsType::CS_MASTER_SLAVE);
   _cdi(CsType::CS_MASTER_SLAVE);
   _tcon(CsType::CS_MASTER_SLAVE);
   _agid(CsType::CS_MASTER_SLAVE);
   _pws(CsType::CS_MASTER_SLAVE);
   _ccset(CsType::CS_MASTER_SLAVE);
   _tres(CsType::CS_MASTER_SLAVE);

   _pwr(CsType::CS_MASTER);
   _en_buf(CsType::CS_MASTER);
   _btst_p(CsType::CS_MASTER);
   _boost_vddp_en(CsType::CS_MASTER);
   _btst_n(CsType::CS_MASTER);
   _buck_boost_vddn(CsType::CS_MASTER);
   _tft_vcom_power(CsType::CS_MASTER);

   _init_display_done = true;
}

void GxEPD2_1330c_EL133UF3::_InitDisplayAlt() {
   _reset();

   _waitWhileBusy("initReset", power_off_time);

   const uint8_t AN_TM_V[] = {0xC0, 0x1C, 0x1C, 0xCC, 0xCC, 0xCC, 0x15, 0x15, 0x55};
   const uint8_t CMD66_V[] = {0x49, 0x55, 0x13, 0x5D, 0x05, 0x10};
   const uint8_t PSR_V[] = {0xDF, 0x6B};
   const uint8_t DCDC_V[] = {0x44, 0x54, 0x00};
   const uint8_t PLL_V[] = {0x08};
   const uint8_t CDI_V[] = {0x37};
   const uint8_t TCON_V[] = {0x03, 0x03};
   const uint8_t POFS_MV[] = {0x00, 0xC0, 0x03, 0xA4};
   const uint8_t POFS_SV[] = {0x00, 0xC0, 0x03, 0x95};
   const uint8_t AGID_V[] = {0x10};
   const uint8_t PWS_V[] = {0x22};
   const uint8_t CCSET_V[] = {0x01};
   const uint8_t TRES_V[] = {0x04, 0xB0, 0x03, 0x20};
   const uint8_t CMDA4_V[] = {0x03, 0x00, 0x01, 0x03, 0x00, 0x03, 0x00, 0x00, 0x00};
   const uint8_t PWR_V[] = {0x0F, 0x00, 0x28, 0x2C, 0x28, 0x38};
   const uint8_t EN_BUF_V[] = {0x07};
   const uint8_t BTST_P_V[] = {0xD8, 0x18};
   const uint8_t BOOST_VDDP_EN_V[] = {0x01};
   const uint8_t BTST_N_V[] = {0xD8, 0x18};
   const uint8_t BUCK_BOOST_VDDN_V[] = {0x01};
   const uint8_t TFT_VCOM_POWER_V[] = {0x02};

   _writeEN133UF3DataCmd(0x74, AN_TM_V, sizeof(AN_TM_V), CsType::CS_MASTER);
   _writeEN133UF3DataCmd(0xF0, CMD66_V, sizeof(CMD66_V), CsType::CS_MASTER_SLAVE);
   _writeEN133UF3DataCmd(0x00, PSR_V, sizeof(PSR_V), CsType::CS_MASTER_SLAVE);
   _writeEN133UF3DataCmd(0xA5, DCDC_V, sizeof(DCDC_V), CsType::CS_MASTER);
   _writeEN133UF3DataCmd(0x30, PLL_V, sizeof(PLL_V), CsType::CS_MASTER_SLAVE);
   _writeEN133UF3DataCmd(0x50, CDI_V, sizeof(CDI_V), CsType::CS_MASTER_SLAVE);
   _writeEN133UF3DataCmd(0x60, TCON_V, sizeof(TCON_V), CsType::CS_MASTER_SLAVE);

   _writeEN133UF3DataCmd(0x03, POFS_MV, sizeof(POFS_MV), CsType::CS_MASTER);
   _writeEN133UF3DataCmd(0x03, POFS_SV, sizeof(POFS_SV), CsType::CS_SLAVE);

   _writeEN133UF3DataCmd(0x86, AGID_V, sizeof(AGID_V), CsType::CS_MASTER_SLAVE);
   _writeEN133UF3DataCmd(0xE3, PWS_V, sizeof(PWS_V), CsType::CS_MASTER_SLAVE);
   _writeEN133UF3DataCmd(0xE0, CCSET_V, sizeof(CCSET_V), CsType::CS_MASTER_SLAVE);
   _writeEN133UF3DataCmd(0x61, TRES_V, sizeof(TRES_V), CsType::CS_MASTER_SLAVE);

   _writeEN133UF3DataCmd(0xA4, CMDA4_V, sizeof(CMDA4_V), CsType::CS_MASTER);
   _writeEN133UF3DataCmd(0x01, PWR_V, sizeof(PWR_V), CsType::CS_MASTER);
   _writeEN133UF3DataCmd(0xB6, EN_BUF_V, sizeof(EN_BUF_V), CsType::CS_MASTER);
   _writeEN133UF3DataCmd(0x06, BTST_P_V, sizeof(BTST_P_V), CsType::CS_MASTER);
   _writeEN133UF3DataCmd(0xB7, BOOST_VDDP_EN_V, sizeof(BOOST_VDDP_EN_V), CsType::CS_MASTER);
   _writeEN133UF3DataCmd(0x05, BTST_N_V, sizeof(BTST_N_V), CsType::CS_MASTER);
   _writeEN133UF3DataCmd(0xB0, BUCK_BOOST_VDDN_V, sizeof(BUCK_BOOST_VDDN_V), CsType::CS_MASTER);
   _writeEN133UF3DataCmd(0xB1, TFT_VCOM_POWER_V, sizeof(TFT_VCOM_POWER_V), CsType::CS_MASTER);

   _init_display_done = true;
}

void GxEPD2_1330c_EL133UF3::_powerOn() {
   if (!_power_is_on) {
      _pon(CsType::CS_MASTER_SLAVE);
      _waitWhileBusy("powerOn");
   }
   _power_is_on = true;
}

inline void GxEPD2_1330c_EL133UF3::_set_cs(const CsType cs_type, const uint8_t level) {
   if ((cs_type & CsType::CS_MASTER) == CsType::CS_MASTER && _cs >= 0) digitalWrite(_cs, level);
   if ((cs_type & CsType::CS_SLAVE) == CsType::CS_SLAVE && _cs_slave >= 0) digitalWrite(_cs_slave, level);
}

void GxEPD2_1330c_EL133UF3::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h, CsType cs_type) {
   uint16_t HRST = x * 2;
   uint16_t HRED = (x + w) * 2 - 1;
   uint16_t VRST = y / 2;
   uint16_t VRED = (y + h) / 2 - 1;

   uint8_t data[9] = {
       (uint8_t)(HRST >> 8),
       (uint8_t)(HRST & 0xFF),
       (uint8_t)(HRED >> 8),
       (uint8_t)(HRED & 0xFF),
       (uint8_t)(VRST >> 8),
       (uint8_t)(VRST & 0xFF),
       (uint8_t)(VRED >> 8),
       (uint8_t)(VRED & 0xFF),
       0x01 // PTLW_ENABLE
   };

   _writeEN133UF3DataCmd(0x83, data, 9, cs_type);
   _writeEN133UF3Cmd(0x91, cs_type);
}

void GxEPD2_1330c_EL133UF3::_clearPartialRamArea(CsType cs_type) {
   uint8_t data[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
   _writeEN133UF3DataCmd(0x83, data, 9, cs_type);
}