#include "GxEPD2_1330c_EL133UF3.h"

GxEPD2_1330c_EL133UF3::GxEPD2_1330c_EL133UF3(int16_t cs, int16_t cs_slave, int16_t dc, int16_t rst, int16_t busy) :
    GxEPD2_EPD(cs, dc, rst, busy, LOW, 60000000, WIDTH, HEIGHT, panel, hasColor, hasPartialUpdate, hasFastPartialUpdate)
{
  _paged = false; //?
  _reset_duration = 30;
  _cs_slave = cs_slave;
}

void GxEPD2_1330c_EL133UF3::init(uint32_t serial_diag_bitrate)
{
  init(serial_diag_bitrate, true, 30, false);
}

void GxEPD2_1330c_EL133UF3::init(uint32_t serial_diag_bitrate, bool initial, uint16_t reset_duration, bool pulldown_rst_mode)
{
  _initial_write = initial;
  _initial_refresh = initial;
  _pulldown_rst_mode = pulldown_rst_mode;
  _power_is_on = false;
  _using_partial_mode = false;
  _hibernating = false;
  _init_display_done = false;
  _reset_duration = reset_duration;
  _paging_step = 0;

  if (serial_diag_bitrate > 0)
  {
    Serial.begin(serial_diag_bitrate);
    _diag_enabled = true;
  }
  if (_cs >= 0)
  {
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, HIGH); // set (needed e.g. for RP2040)
  }
  if (_cs_slave >= 0)
  {
    pinMode(_cs_slave, OUTPUT);
    digitalWrite(_cs_slave, HIGH);
  }
  if (_busy >= 0)
  {
    pinMode(_busy, INPUT_PULLUP);
  }

  // _pSPIx->begin(); // may steal _rst pin (Waveshare Pico-ePaper-2.9) REMOVED: User calls SPI.begin() with custom pins.

  if (_rst >= 0)
  {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, HIGH); // set (needed e.g. for RP2040)
  }
  if (_cs >= 0)
  {
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, HIGH); // set (needed e.g. for RP2040)
  }
  if (_cs_slave >= 0)
  {
    pinMode(_cs_slave, OUTPUT);
    digitalWrite(_cs_slave, HIGH); // set (needed e.g. for RP2040)
  }
  if (_dc >= 0)
  {
    pinMode(_dc, OUTPUT);
    digitalWrite(_dc, HIGH);
  }
}

void GxEPD2_1330c_EL133UF3::clearScreen(uint8_t value)
{
  writeScreenBuffer(value);
}

void GxEPD2_1330c_EL133UF3::clearScreen(uint8_t black_value, uint8_t color_value)
{
  writeScreenBuffer(black_value, color_value);
}

void GxEPD2_1330c_EL133UF3::writeScreenBuffer(uint8_t value)
{
  writeScreenBuffer(value, 0xFF);
}

void GxEPD2_1330c_EL133UF3::writeScreenBuffer(uint8_t color_set, uint8_t color_value)
{
  uint8_t color = (color_set << 4) | (color_set & 0x0F);

  if (!_init_display_done)
  {
    Serial.println("[DISP] Init Start");
    _InitDisplay();
    Serial.println("[DISP] Init End");
  }

  // _powerOn(); // REMOVED: Power On should happen in refresh(), after data is written.

  _writeColor(color, CsType::MASTER);
  _writeColor(color, CsType::SLAVE);

  _waitWhileBusy();
}

void GxEPD2_1330c_EL133UF3::writeImage(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  if (!_init_display_done)
    _InitDisplay();
  int16_t w_half = WIDTH / 2;
  int16_t bytes_per_line_half = w_half / 2;

  if (_paged && (x == 0) && (w == int16_t(WIDTH)) && (h < int16_t(HEIGHT)))
  {

    int16_t bytes_per_line_bitmap = w / 2;

    if (_paging_step == 1)
    { // MASTER {
      _pSPIx->beginTransaction(_spi_settings);
      if (y == 0)
      {
        _set_cs(CsType::MASTER, LOW);
        _pSPIx->transfer(DTM);
      }

      for (int16_t row = 0; row < h; row++)
      {
        const uint8_t* val_ptr = bitmap + (row * bytes_per_line_bitmap);
        for (int16_t col_byte = 0; col_byte < bytes_per_line_half; col_byte++)
        {
          uint8_t val = pgm ? pgm_read_byte(&val_ptr[col_byte]) : val_ptr[col_byte];
          if (invert)
            val = ~val;
          _pSPIx->transfer(val);
        }
      }
      if (y + h == HEIGHT)
      {
        _set_cs(CsType::MASTER, HIGH);
        _paged = false;
      }
      _pSPIx->endTransaction();
    }
    else if (_paging_step == 2) // SLAVE
    {
      _pSPIx->beginTransaction(_spi_settings);
      if (y == 0)
      {
        _set_cs(CsType::SLAVE, LOW);
        _pSPIx->transfer(DTM);
      }

      for (int16_t row = 0; row < h; row++)
      {
        const uint8_t* val_ptr = bitmap + (row * bytes_per_line_bitmap);
        for (int16_t col_byte = 0; col_byte < bytes_per_line_half; col_byte++)
        {
          uint8_t val =
              pgm ? pgm_read_byte(&val_ptr[bytes_per_line_half + col_byte]) : val_ptr[bytes_per_line_half + col_byte];
          if (invert)
            val = ~val;
          _pSPIx->transfer(val);
        }
      }
      if (y + h == HEIGHT)
      {
        _set_cs(CsType::SLAVE, HIGH);
        _paged = false;
      }
      _pSPIx->endTransaction();
    }
  }
  else
  {
    // TODO: still untested without paging, this is just vibe coded!
    _paged = false;
    int16_t wb = (w + 1) / 2; // bytes per line input (2 pix per byte)
    x -= x % 8;               // dummy alignment if needed, but here we work on bytes
    w = 2 * ((w + 1) / 2);    // align to byte

    if ((w <= 0) || (h <= 0))
      return;

    // Master
    _pSPIx->beginTransaction(_spi_settings);
    _set_cs(CsType::MASTER, LOW);
    _pSPIx->transfer(DTM);

    for (int16_t i = 0; i < int16_t(HEIGHT); i++)
    {
      for (int16_t j = 0; j < int16_t(WIDTH / 2); j += 2) // Master is 0..599 pixels. Input 4-bit means 0..300 bytes?
        // Wait, let's just loop bytes. 300 bytes.
        // Master covers pixels 0 to 599.
        // j counts pixels? No, let's count bytes.
        for (int16_t byte_idx = 0; byte_idx < 300; byte_idx++)
        {
          // This byte corresponds to pixels (byte_idx*2) and (byte_idx*2 + 1) in Master column.
          int16_t pixel_x_base = byte_idx * 2;
          // Check if this pixel location is inside the update window (x,y,w,h)
          // Since we write 2 pixels at once, checks are roughly per byte.
          // If (pixel_x_base >= x) && (pixel_x_base < x+w) && (i >= y) && (i < y+h)

          // Simplified check: assume bytes.
          // Master handles x_global 0..599.

          uint8_t data = 0xFF; // Clean/White

          // Where is this data in the bitmap?
          // Global X for this byte is byte_idx*2.
          int16_t global_x = byte_idx * 2;

          if ((global_x >= x) && (global_x < x + w) && (i >= y) && (i < y + h))
          {
            int16_t bitmap_x = global_x - x; // relative x in bitmap
            int16_t bitmap_y = i - y;        // relative y

            // Index in bitmap
            uint32_t idx = mirror_y ? (bitmap_x / 2) + uint32_t((h - 1 - bitmap_y)) * wb
                                    : (bitmap_x / 2) + uint32_t(bitmap_y) * wb;

            if (pgm)
              data = pgm_read_byte(&bitmap[idx]);
            else
              data = bitmap[idx];

            if (invert)
              data = ~data;
          }
          _pSPIx->transfer(data);
        }
    }
    _set_cs(CsType::MASTER, HIGH);
    _pSPIx->endTransaction();

    // Slave
    _pSPIx->beginTransaction(_spi_settings);
    _set_cs(CsType::SLAVE, LOW);
    _pSPIx->transfer(DTM);

    for (int16_t i = 0; i < int16_t(HEIGHT); i++)
    {
      for (int16_t byte_idx = 0; byte_idx < 300; byte_idx++)
      {
        int16_t pixel_x_base = 600 + byte_idx * 2; // Slave starts at 600
        int16_t global_x = pixel_x_base;

        uint8_t data = 0xFF; // Clean/White

        if ((global_x >= x) && (global_x < x + w) && (i >= y) && (i < y + h))
        {
          int16_t bitmap_x = global_x - x;
          int16_t bitmap_y = i - y;
          uint32_t idx =
              mirror_y ? (bitmap_x / 2) + uint32_t((h - 1 - bitmap_y)) * wb : (bitmap_x / 2) + uint32_t(bitmap_y) * wb;

          if (pgm)
            data = pgm_read_byte(&bitmap[idx]);
          else
            data = bitmap[idx];
          if (invert)
            data = ~data;
        }
        _pSPIx->transfer(data);
      }
    }
    _set_cs(CsType::SLAVE, HIGH);
    _pSPIx->endTransaction();
  }
}

void GxEPD2_1330c_EL133UF3::writeImagePart(const uint8_t bitmap[], int16_t x_part, int16_t y_part, int16_t w_bitmap,
                                           int16_t h_bitmap, int16_t x, int16_t y, int16_t w, int16_t h, bool invert,
                                           bool mirror_y, bool pgm)
{
  // TODO: implement the partial full refresh op spectra 6
  //  For now, redirect to writeImage if it looks like a full page write that matches our simplistic assumption
  //  or just implement the write loop assuming DTM works sequentially if we don't reset?
  //  No, we cannot assume DTM continuation across transactions.
  //  We will attempt to use writeImage logic for the part.
  //  If the controller doesn't support windowing, this will write to the top-left!
  //  But since we can't easily find the window command, this is the best effort.
  //  If w == WIDTH and h == HEIGHT, it works.
  writeImage(bitmap, x, y, w, h, invert, mirror_y, pgm); // Simply forward.
}

void GxEPD2_1330c_EL133UF3::writeImage(const uint8_t* black, const uint8_t* color, int16_t x, int16_t y, int16_t w,
                                       int16_t h, bool invert, bool mirror_y, bool pgm)
{
}

void GxEPD2_1330c_EL133UF3::writeImagePart(const uint8_t* black, const uint8_t* color, int16_t x_part, int16_t y_part,
                                           int16_t w_bitmap, int16_t h_bitmap, int16_t x, int16_t y, int16_t w,
                                           int16_t h, bool invert, bool mirror_y, bool pgm)
{
}

void GxEPD2_1330c_EL133UF3::writeNative(const uint8_t* data1, const uint8_t* data2, int16_t x, int16_t y, int16_t w,
                                        int16_t h, bool invert, bool mirror_y, bool pgm)
{
  if (data2)
    return;
  writeImage(data1, x, y, w, h, invert, mirror_y, pgm);
}

void GxEPD2_1330c_EL133UF3::writeNativePart(const uint8_t* data1, const uint8_t* data2, int16_t x_part, int16_t y_part,
                                            int16_t w_bitmap, int16_t h_bitmap, int16_t x, int16_t y, int16_t w,
                                            int16_t h, bool invert, bool mirror_y, bool pgm)
{
  if (data2)
    return;
  writeImagePart(data1, x_part, y_part, w_bitmap, h_bitmap, x, y, w, h, invert, mirror_y, pgm);
}

void GxEPD2_1330c_EL133UF3::drawImage(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert,
                                      bool mirror_y, bool pgm)
{
}

void GxEPD2_1330c_EL133UF3::drawImagePart(const uint8_t bitmap[], int16_t x_part, int16_t y_part, int16_t w_bitmap,
                                          int16_t h_bitmap, int16_t x, int16_t y, int16_t w, int16_t h, bool invert,
                                          bool mirror_y, bool pgm)
{
}

void GxEPD2_1330c_EL133UF3::drawImage(const uint8_t* black, const uint8_t* color, int16_t x, int16_t y, int16_t w,
                                      int16_t h, bool invert, bool mirror_y, bool pgm)
{
}

void GxEPD2_1330c_EL133UF3::drawImagePart(const uint8_t* black, const uint8_t* color, int16_t x_part, int16_t y_part,
                                          int16_t w_bitmap, int16_t h_bitmap, int16_t x, int16_t y, int16_t w,
                                          int16_t h, bool invert, bool mirror_y, bool pgm)
{
}

void GxEPD2_1330c_EL133UF3::drawNative(const uint8_t* data1, const uint8_t* data2, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
}

void GxEPD2_1330c_EL133UF3::writeDemoBitmap(const uint8_t* data1, const uint8_t* data2, int16_t x, int16_t y, int16_t w, int16_t h, int16_t mode, bool mirror_y, bool pgm)
{
}

void GxEPD2_1330c_EL133UF3::drawDemoBitmap(const uint8_t* data1, const uint8_t* data2, int16_t x, int16_t y, int16_t w,
                                           int16_t h, int16_t mode, bool mirror_y, bool pgm)
{
}

void GxEPD2_1330c_EL133UF3::refresh(bool partial_update_mode)
{
  if (_paging_step == 1)
    return;
  Serial.println("[DISP] Refresh Start");
  _powerOn();
  _waitWhileBusy(); // for the love of god, please do not remove this
  delay(30);
  _drf(CsType::MASTER_SLAVE);
  _waitWhileBusy("refresh", full_refresh_time);
  _paging_step = 0;
}

void GxEPD2_1330c_EL133UF3::refresh(int16_t x, int16_t y, int16_t w, int16_t h)
{
  refresh();
}

void GxEPD2_1330c_EL133UF3::powerOff()
{
  if (_paging_step == 1)
    return;
  Serial.println("[DISP] Power Off");
  _pof(CsType::MASTER_SLAVE);
  _waitWhileBusy("powerOff", power_off_time);
  _power_is_on = false;
}

void GxEPD2_1330c_EL133UF3::hibernate()
{
  powerOff();
  _writeEN133UF3DataCmd(SLEEP, SLEEP_V, sizeof(SLEEP_V), CsType::MASTER_SLAVE);
}

void GxEPD2_1330c_EL133UF3::setPaged()
{
  _paged = true;
  _initial_write = false;
  _initial_refresh = false;
  _paging_step = (_paging_step % 2) + 1;
}

void GxEPD2_1330c_EL133UF3::_writeEN133UF3DataCmd(uint8_t cmd, const uint8_t* data, const uint8_t data_len, CsType cs_type)
{
  _pSPIx->beginTransaction(_spi_settings);
  _set_cs(cs_type, LOW);
  _pSPIx->transfer(cmd);
  _pSPIx->transferBytes(data, NULL, data_len);
  _set_cs(cs_type, HIGH);
  _pSPIx->endTransaction();
}

void GxEPD2_1330c_EL133UF3::_writeEN133UF3Cmd(uint8_t cmd, CsType cs_type)
{
  _pSPIx->beginTransaction(_spi_settings);
  _set_cs(cs_type, LOW);
  _pSPIx->transfer(cmd);
  _set_cs(cs_type, HIGH);
  _pSPIx->endTransaction();
}

inline void GxEPD2_1330c_EL133UF3::_psr(CsType cs_type)
{
  _writeEN133UF3DataCmd(PSR, PSR_V, sizeof(PSR_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_pwr(CsType cs_type)
{
  _writeEN133UF3DataCmd(PWR_epd, PWR_V, sizeof(PWR_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_pof(CsType cs_type)
{
  _writeEN133UF3DataCmd(POF, POF_V, sizeof(POF_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_pon(CsType cs_type)
{
  _writeEN133UF3Cmd(PON, cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_drf(CsType cs_type)
{
  _writeEN133UF3DataCmd(DRF, DRF_V, sizeof(DRF_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_cdi(CsType cs_type)
{
  _writeEN133UF3DataCmd(CDI, CDI_V, sizeof(CDI_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_tcon(CsType cs_type)
{
  _writeEN133UF3DataCmd(TCON, TCON_V, sizeof(TCON_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_tres(CsType cs_type)
{
  _writeEN133UF3DataCmd(TRES, TRES_V, sizeof(TRES_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_cmd66(CsType cs_type)
{
  _writeEN133UF3DataCmd(CMD66, CMD66_V, sizeof(CMD66_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_en_buf(CsType cs_type)
{
  _writeEN133UF3DataCmd(EN_BUF, EN_BUF_V, sizeof(EN_BUF_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_ccset(CsType cs_type)
{
  _writeEN133UF3DataCmd(CCSET, CCSET_V, sizeof(CCSET_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_pws(CsType cs_type)
{
  _writeEN133UF3DataCmd(PWS, PWS_V, sizeof(PWS_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_an_tm(CsType cs_type)
{
  _writeEN133UF3DataCmd(AN_TM, AN_TM_V, sizeof(AN_TM_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_agid(CsType cs_type)
{
  _writeEN133UF3DataCmd(AGID, AGID_V, sizeof(AGID_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_btst_p(CsType cs_type)
{
  _writeEN133UF3DataCmd(BTST_P, BTST_P_V, sizeof(BTST_P_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_btst_n(CsType cs_type)
{
  _writeEN133UF3DataCmd(BTST_N, BTST_N_V, sizeof(BTST_N_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_boost_vddp_en(CsType cs_type)
{
  _writeEN133UF3DataCmd(BOOST_VDDP_EN, BOOST_VDDP_EN_V, sizeof(BOOST_VDDP_EN_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_buck_boost_vddn(CsType cs_type)
{
  _writeEN133UF3DataCmd(BUCK_BOOST_VDDN, BUCK_BOOST_VDDN_V, sizeof(BUCK_BOOST_VDDN_V), cs_type);
}

inline void GxEPD2_1330c_EL133UF3::_tft_vcom_power(CsType cs_type)
{
  _writeEN133UF3DataCmd(TFT_VCOM_POWER, TFT_VCOM_POWER_V, sizeof(TFT_VCOM_POWER_V), cs_type);
}

void GxEPD2_1330c_EL133UF3::_writeColor(uint8_t color_value, CsType cs_type)
{
  _pSPIx->beginTransaction(_spi_settings);
  _set_cs(cs_type, LOW);
  _pSPIx->transfer(DTM);
  for (uint16_t y = 0; y < HEIGHT; y++)
  {
    for (uint16_t x = 0; x < HALF_WIDTH / 2; x++)
    {
      _pSPIx->transfer(color_value);
    }
  }
  _set_cs(cs_type, HIGH);
  _pSPIx->endTransaction();
}

void GxEPD2_1330c_EL133UF3::_InitDisplay()
{

  _reset();

  _waitWhileBusy("initReset", power_off_time);

  _an_tm(CsType::MASTER);
  _cmd66(CsType::MASTER_SLAVE);
  _psr(CsType::MASTER_SLAVE);
  _cdi(CsType::MASTER_SLAVE);
  _tcon(CsType::MASTER_SLAVE);
  _agid(CsType::MASTER_SLAVE);
  _pws(CsType::MASTER_SLAVE);
  _ccset(CsType::MASTER_SLAVE);
  _tres(CsType::MASTER_SLAVE);

  _pwr(CsType::MASTER);
  _en_buf(CsType::MASTER);
  _btst_p(CsType::MASTER);
  _boost_vddp_en(CsType::MASTER);
  _btst_n(CsType::MASTER);
  _buck_boost_vddn(CsType::MASTER);
  _tft_vcom_power(CsType::MASTER);

  _init_display_done = true;
}

void GxEPD2_1330c_EL133UF3::_powerOn()
{
  if (!_power_is_on)
  {
    _pon(CsType::MASTER_SLAVE);
    _waitWhileBusy("powerOn");
  }
  _power_is_on = true;
}

inline void GxEPD2_1330c_EL133UF3::_set_cs(const CsType cs_type, const uint8_t level)
{
  if ((cs_type & CsType::MASTER) == CsType::MASTER && _cs >= 0) digitalWrite(_cs, level);
  if ((cs_type & CsType::SLAVE) == CsType::SLAVE && _cs_slave >= 0) digitalWrite(_cs_slave, level);
}