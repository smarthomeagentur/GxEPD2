#include "GxEPD2_1330c_EL133UF3.h"

GxEPD2_1330c_EL133UF3::GxEPD2_1330c_EL133UF3(int16_t cs, int16_t cs_slave, int16_t dc, int16_t rst, int16_t busy) : GxEPD2_EPD(cs, cs_slave, dc, rst, busy, LOW, 60000000, WIDTH, HEIGHT, panel, hasColor, hasPartialUpdate, hasFastPartialUpdate)
{
  _paged = false; //?
  _reset_duration = 30;
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

  if (serial_diag_bitrate > 0)
  {
    Serial.begin(serial_diag_bitrate);
    _diag_enabled = true;
  }
  if (_cs >= 0)
  {
    digitalWrite(_cs, HIGH); // preset (less glitch for any analyzer)
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, HIGH); // set (needed e.g. for RP2040)
  }
  if (_cs_slave >= 0) {
    digitalWrite(_cs_slave, HIGH);
    pinMode(_cs_slave, OUTPUT);
    digitalWrite(_cs_slave, HIGH);
  }
  if (_busy >= 0)
  {
    pinMode(_busy, INPUT_PULLUP);
  }

  _reset();
  _pSPIx->begin(); // may steal _rst pin (Waveshare Pico-ePaper-2.9)

  if (_rst >= 0)
  {
    digitalWrite(_rst, HIGH); // preset (less glitch for any analyzer)
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, HIGH); // set (needed e.g. for RP2040)
  }
  if (_cs >= 0)
  {
    digitalWrite(_cs, HIGH); // preset (less glitch for any analyzer)
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, HIGH); // set (needed e.g. for RP2040)
  }
  if (_cs_slave >= 0)
  {
    digitalWrite(_cs_slave, HIGH); // preset (less glitch for any analyzer)
    pinMode(_cs_slave, OUTPUT);
    digitalWrite(_cs_slave, HIGH); // set (needed e.g. for RP2040)
  }
  if (_dc >= 0)
  {
    digitalWrite(_dc, HIGH); // preset (less glitch for any analyzer)
    pinMode(_dc, OUTPUT);
    digitalWrite(_dc, HIGH); // set (needed e.g. for RP2040)
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

void GxEPD2_1330c_EL133UF3::writeScreenBuffer(uint8_t black_value, uint8_t color_value)
{
  uint8_t color = (color << 4) | color;

  if(!_init_display_done) 
  {
    _InitDisplay();
  }

  _powerOn(); //needed? i don't fucking know. Seems like a sensible thing to do here.

  _writeColor(color, CsType::MASTER);
  _writeColor(color, CsType::SLAVE);

  _waitWhileBusy();
}

void GxEPD2_1330c_EL133UF3::writeImage(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
}

void GxEPD2_1330c_EL133UF3::writeImagePart(const uint8_t bitmap[], int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
}

void GxEPD2_1330c_EL133UF3::writeImage(const uint8_t *black, const uint8_t *color, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
}

void GxEPD2_1330c_EL133UF3::writeImagePart(const uint8_t *black, const uint8_t *color, int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
}

void GxEPD2_1330c_EL133UF3::writeNative(const uint8_t *data1, const uint8_t *data2, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
}

void GxEPD2_1330c_EL133UF3::writeNativePart(const uint8_t *data1, const uint8_t *data2, int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
}

void GxEPD2_1330c_EL133UF3::drawImage(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
}

void GxEPD2_1330c_EL133UF3::drawImagePart(const uint8_t bitmap[], int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
}

void GxEPD2_1330c_EL133UF3::drawImage(const uint8_t *black, const uint8_t *color, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
}

void GxEPD2_1330c_EL133UF3::drawImagePart(const uint8_t *black, const uint8_t *color, int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
}

void GxEPD2_1330c_EL133UF3::drawNative(const uint8_t *data1, const uint8_t *data2, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
}

void GxEPD2_1330c_EL133UF3::writeDemoBitmap(const uint8_t *data1, const uint8_t *data2, int16_t x, int16_t y, int16_t w, int16_t h, int16_t mode, bool mirror_y, bool pgm)
{
}

void GxEPD2_1330c_EL133UF3::drawDemoBitmap(const uint8_t *data1, const uint8_t *data2, int16_t x, int16_t y, int16_t w, int16_t h, int16_t mode, bool mirror_y, bool pgm)
{
}

void GxEPD2_1330c_EL133UF3::refresh(bool partial_update_mode)
{
  _powerOn();
  _waitWhileBusy(); //for the love of god, please do not remove this
  _drf(CsType::MASTER_SLAVE);
  _waitWhileBusy("refresh", full_refresh_time);
}

void GxEPD2_1330c_EL133UF3::refresh(int16_t x, int16_t y, int16_t w, int16_t h)
{
  refresh();
}

void GxEPD2_1330c_EL133UF3::powerOff()
{
  _pof(CsType::MASTER_SLAVE);
  _waitWhileBusy("powerOff", power_off_time);
  _power_is_on = false;
}

void GxEPD2_1330c_EL133UF3::hibernate()
{
  powerOff();
  _writeEN133UF3Cmd(SLEEP, SLEEP_V, sizeof(SLEEP_V), CsType::MASTER_SLAVE);
}

void GxEPD2_1330c_EL133UF3::setPaged()
{
}

void GxEPD2_1330c_EL133UF3::_writeEN133UF3Cmd(uint8_t cmd, const uint8_t *data, const uint8_t data_len, CsType cs_type)
{
  _startTransfer(cs_type);
  _transfer(cmd);
  for (uint8_t i = 0; i < data_len; i++)
  {
    _transfer(data[i]);
  }
  _endTransfer(cs_type);
}

void GxEPD2_1330c_EL133UF3::_psr(CsType cs_type)
{
  _writeEN133UF3Cmd(PSR, PSR_V, sizeof(PSR_V), cs_type);
}

void GxEPD2_1330c_EL133UF3::_pwr(CsType cs_type)
{
  _writeEN133UF3Cmd(PWR_epd, PWR_V, sizeof(PWR_V), cs_type);
}

void GxEPD2_1330c_EL133UF3::_pof(CsType cs_type)
{
  _writeEN133UF3Cmd(POF, POF_V, sizeof(POF_V), cs_type);
}

void GxEPD2_1330c_EL133UF3::_pon(CsType cs_type)
{
  _writeCommand(PON, cs_type);
}

void GxEPD2_1330c_EL133UF3::_drf(CsType cs_type)
{
  _writeEN133UF3Cmd(DRF, DRF_V, sizeof(DRF_V), cs_type);
}

void GxEPD2_1330c_EL133UF3::_cdi(CsType cs_type)
{
  _writeEN133UF3Cmd(CDI, CDI_V, sizeof(CDI_V), cs_type);
}

void GxEPD2_1330c_EL133UF3::_tcon(CsType cs_type)
{
  _writeEN133UF3Cmd(TCON, TCON_V, sizeof(TCON_V), cs_type);
}

void GxEPD2_1330c_EL133UF3::_tres(CsType cs_type)
{
  _writeEN133UF3Cmd(TRES, TRES_V, sizeof(TRES_V), cs_type);
}

void GxEPD2_1330c_EL133UF3::_cmd66(CsType cs_type)
{
  _writeEN133UF3Cmd(CMD66, CMD66_V, sizeof(CMD66_V), cs_type);
}

void GxEPD2_1330c_EL133UF3::_en_buf(CsType cs_type)
{
  _writeEN133UF3Cmd(EN_BUF, EN_BUF_V, sizeof(EN_BUF_V), cs_type);
}

void GxEPD2_1330c_EL133UF3::_ccset(CsType cs_type)
{
  _writeEN133UF3Cmd(CCSET, CCSET_V, sizeof(CCSET_V), cs_type);
}

void GxEPD2_1330c_EL133UF3::_pws(CsType cs_type)
{
  _writeEN133UF3Cmd(PWS, PWS_V, sizeof(PWS_V), cs_type);
}

void GxEPD2_1330c_EL133UF3::_an_tm(CsType cs_type)
{
  _writeEN133UF3Cmd(AN_TM, AN_TM_V, sizeof(AN_TM_V), cs_type);
}

void GxEPD2_1330c_EL133UF3::_agid(CsType cs_type)
{
  _writeEN133UF3Cmd(AGID, AGID_V, sizeof(AGID_V), cs_type);
}

void GxEPD2_1330c_EL133UF3::_btst_p(CsType cs_type)
{
  _writeEN133UF3Cmd(BTST_P, BTST_P_V, sizeof(BTST_P_V), cs_type);
}

void GxEPD2_1330c_EL133UF3::_btst_n(CsType cs_type)
{
  _writeEN133UF3Cmd(BTST_N, BTST_N_V, sizeof(BTST_N_V), cs_type);
}

void GxEPD2_1330c_EL133UF3::_boost_vddp_en(CsType cs_type)
{
  _writeEN133UF3Cmd(BOOST_VDDP_EN, BOOST_VDDP_EN_V, sizeof(BOOST_VDDP_EN_V), cs_type);
}

void GxEPD2_1330c_EL133UF3::_buck_boost_vddn(CsType cs_type)
{
  _writeEN133UF3Cmd(BUCK_BOOST_VDDN, BUCK_BOOST_VDDN_V, sizeof(BUCK_BOOST_VDDN_V), cs_type);
}

void GxEPD2_1330c_EL133UF3::_tft_vcom_power(CsType cs_type)
{
  _writeEN133UF3Cmd(TFT_VCOM_POWER, TFT_VCOM_POWER_V, sizeof(TFT_VCOM_POWER_V), cs_type);
}

void GxEPD2_1330c_EL133UF3::_writeColor(uint8_t color_value, CsType cs_type)
{
  _startTransfer(cs_type);
  _transfer(DRF);
  for (uint16_t y = 0; y < HEIGHT; y++)
  {
    for (uint16_t x = 0; x < HALF_WIDTH / 2; x++)
    {
      _transfer(color_value);
    }
  }
  _endTransfer(cs_type);
}

void GxEPD2_1330c_EL133UF3::_InitDisplay()
{
  _reset();

  _an_tm();
  _cmd66(CsType::MASTER_SLAVE);
  _psr(CsType::MASTER_SLAVE);
  _cdi(CsType::MASTER_SLAVE);
  _tcon(CsType::MASTER_SLAVE);
  _agid(CsType::MASTER_SLAVE);
  _pws(CsType::MASTER_SLAVE);
  _ccset(CsType::MASTER_SLAVE);
  _tres(CsType::MASTER_SLAVE);

  _pwr();
  _en_buf();
  _btst_p();
  _boost_vddp_en();
  _btst_n();
  _buck_boost_vddn();
  _tft_vcom_power();

  _init_display_done = true;
}

void GxEPD2_1330c_EL133UF3::_powerOn()
{
  if(!_power_is_on) {
    _pon(CsType::MASTER_SLAVE);
    _waitWhileBusy();
  }
  _power_is_on = true;
}
