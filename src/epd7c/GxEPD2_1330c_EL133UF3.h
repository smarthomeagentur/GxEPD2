// Display Library for SPI e-paper panels from Dalian Good Display and boards from Waveshare.
// Requires HW SPI and Adafruit_GFX. Caution: the e-paper panels require 3.3V supply AND data lines!
//
// based on Demo Example from Good Display: https://www.good-display.com/product/442.html
// Panel: GDEY073D46 : https://www.good-display.com/product/442.html
// Controller: unknown
//
// Author: Jean-Marc Zingg
//
// Version: see library.properties
//
// Library: https://github.com/ZinggJM/GxEPD2

#ifndef _GxEPD2_1330c_EL133UF3_H_
#define _GxEPD2_1330c_EL133UF3_H_
#define EPD_13IN3E_WIDTH        1200
#define EPD_13IN3E_HEIGHT       1600    
#define RESET_DURATION          30

#define EPD_13IN3E_BLACK        0x0
#define EPD_13IN3E_WHITE        0x1
#define EPD_13IN3E_YELLOW       0x2
#define EPD_13IN3E_RED          0x3
#define EPD_13IN3E_BLUE         0x5
#define EPD_13IN3E_GREEN        0x6

#define PSR             0x00
#define PWR_epd         0x01
#define POF             0x02
#define PON             0x04
#define BTST_N          0x05
#define BTST_P          0x06
#define SLEEP           0x07
#define DTM             0x10
#define DRF             0x12
#define CDI             0x50
#define TCON            0x60
#define TRES            0x61
#define AN_TM           0x74
#define AGID            0x86
#define BUCK_BOOST_VDDN 0xB0
#define TFT_VCOM_POWER  0xB1
#define EN_BUF          0xB6
#define BOOST_VDDP_EN   0xB7
#define CCSET           0xE0
#define PWS             0xE3
#define CMD66           0xF0


#include "../GxEPD2_EPD.h"

class GxEPD2_1330c_EL133UF3 : public GxEPD2_EPD
{
  private:
    // const uint8_t spiCsPin[2] = {
    // 		SPI_CS0, SPI_CS1
    // };
    const uint8_t PSR_V[2] = {
      0xDF, 0x69
    };
    const uint8_t PWR_V[6] = {
      0x0F, 0x00, 0x28, 0x2C, 0x28, 0x38
    };
    const uint8_t POF_V[1] = {
      0x00
    };
    const uint8_t DRF_V[1] = {
      0x00
    };
    const uint8_t CDI_V[1] = {
      0xF7
    };
    const uint8_t TCON_V[2] = {
      0x03, 0x03
    };
    const uint8_t TRES_V[4] = {
      0x04, 0xB0, 0x03, 0x20
    };
    const uint8_t CMD66_V[6] = {
      0x49, 0x55, 0x13, 0x5D, 0x05, 0x10
    };
    const uint8_t EN_BUF_V[1] = {
      0x07
    };
    const uint8_t CCSET_V[1] = {
      0x01
    };
    const uint8_t PWS_V[1] = {
      0x22
    };
    const uint8_t AN_TM_V[9] = {
      0xC0, 0x1C, 0x1C, 0xCC, 0xCC, 0xCC, 0x15, 0x15, 0x55
    };
    const uint8_t AGID_V[1] = {
      0x10
    };
    const uint8_t BTST_P_V[2] = {
      0xE8, 0x28
    };
    const uint8_t BOOST_VDDP_EN_V[1] = {
      0x01
    };
    const uint8_t BTST_N_V[2] = {
      0xE8, 0x28
    };
    const uint8_t BUCK_BOOST_VDDN_V[1] = {
      0x01
    };
    const uint8_t TFT_VCOM_POWER_V[1] = {
      0x02
    };
    const uint8_t SLEEP_V[1] = {
      0xA5
    };

  public:
    // attributes
    static const uint16_t WIDTH = EPD_13IN3E_WIDTH;
    static const uint16_t WIDTH_VISIBLE = WIDTH;
    static const uint16_t HEIGHT = EPD_13IN3E_HEIGHT;
    static const uint16_t HALF_WIDTH = (WIDTH % 2 == 0) ? (WIDTH / 2) : (WIDTH / 2 + 1);
    static const GxEPD2::Panel panel = GxEPD2::GDEY073D46;
    static const bool hasColor = true;
    static const bool hasPartialUpdate = false;
    static const bool hasFastPartialUpdate = false;
    static const uint16_t power_on_time = 200; // ms, e.g. 172000us
    static const uint16_t power_off_time = 150; // ms, e.g. 145000us
    static const uint16_t full_refresh_time = 40000; // ms, e.g. 38858000us
    static const uint16_t partial_refresh_time = 40000; // ms, e.g. 38858000us
    // constructor
    GxEPD2_1330c_EL133UF3(int16_t cs, int16_t cs_slave, int16_t dc, int16_t rst, int16_t busy);
    void init(uint32_t serial_diag_bitrate = 0); // serial_diag_bitrate = 0 : disabled
    void init(uint32_t serial_diag_bitrate, bool initial, uint16_t reset_duration = 10, bool pulldown_rst_mode = false);
    // methods (virtual)
    //  Support for Bitmaps (Sprites) to Controller Buffer and to Screen
    void clearScreen(uint8_t value = 0xFF); // init controller memory and screen (default white)
    void clearScreen(uint8_t black_value, uint8_t color_value); // init controller memory and screen
    void writeScreenBuffer(uint8_t value = 0xFF); // init controller memory (default white)
    void writeScreenBuffer(uint8_t black_value, uint8_t color_value); // init controller memory
    // write to controller memory, without screen refresh; x and w should be multiple of 8
    void writeImage(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false);
    void writeImagePart(const uint8_t bitmap[], int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
                        int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false);
    void writeImage(const uint8_t* black, const uint8_t* color, int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false);
    void writeImagePart(const uint8_t* black, const uint8_t* color, int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
                        int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false);
    // write sprite of native data to controller memory, without screen refresh; x and w should be multiple of 8
    void writeNative(const uint8_t* data1, const uint8_t* data2, int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false);
    void writeNativePart(const uint8_t* data1, const uint8_t* data2, int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
                         int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false);
    // write to controller memory, with screen refresh; x and w should be multiple of 8
    void drawImage(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false);
    void drawImagePart(const uint8_t bitmap[], int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
                       int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false);
    void drawImage(const uint8_t* black, const uint8_t* color, int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false);
    void drawImagePart(const uint8_t* black, const uint8_t* color, int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
                       int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false);
    // write sprite of native data to controller memory, with screen refresh; x and w should be multiple of 8
    void drawNative(const uint8_t* data1, const uint8_t* data2, int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false);
    void writeDemoBitmap(const uint8_t* data1, const uint8_t* data2, int16_t x, int16_t y, int16_t w, int16_t h, int16_t mode = 0, bool mirror_y = false, bool pgm = false);
    void drawDemoBitmap(const uint8_t* data1, const uint8_t* data2, int16_t x, int16_t y, int16_t w, int16_t h, int16_t mode = 0, bool mirror_y = false, bool pgm = false);
    void refresh(bool partial_update_mode = false); // screen refresh from controller memory to full screen
    void refresh(int16_t x, int16_t y, int16_t w, int16_t h); // screen refresh from controller memory, partial screen
    void powerOff(); // turns off generation of panel driving voltages, avoids screen fading over time
    void hibernate(); // turns powerOff() and sets controller to deep sleep for minimum power use, ONLY if wakeable by RST (rst >= 0)
    void setPaged(); // for GxEPD2_154c and GxEPD2_565c and GxEPD2_730c_GDEY073D46 paged workaround
  private:
    uint8_t _colorOfDemoBitmap(uint8_t from);
    void _powerOn();
    void _InitDisplay();
    void _writeEN133UF3Cmd(uint8_t cmd, const uint8_t *data, uint8_t data_len, CsType cs_type = CsType::MASTER);
    void _psr(CsType cs_type = CsType::MASTER);
    void _pwr(CsType cs_type = CsType::MASTER);
    void _pof(CsType cs_type = CsType::MASTER);
    void _pon(CsType cs_type = CsType::MASTER);
    void _drf(CsType cs_type = CsType::MASTER);
    void _cdi(CsType cs_type = CsType::MASTER);
    void _tcon(CsType cs_type = CsType::MASTER);
    void _tres(CsType cs_type = CsType::MASTER);
    void _cmd66(CsType cs_type = CsType::MASTER);
    void _en_buf(CsType cs_type = CsType::MASTER);
    void _ccset(CsType cs_type = CsType::MASTER);
    void _pws(CsType cs_type = CsType::MASTER);
    void _an_tm(CsType cs_type = CsType::MASTER);
    void _agid(CsType cs_type = CsType::MASTER);
    void _btst_p(CsType cs_type = CsType::MASTER);
    void _btst_n(CsType cs_type = CsType::MASTER);
    void _boost_vddp_en(CsType cs_type = CsType::MASTER);
    void _buck_boost_vddn(CsType cs_type = CsType::MASTER);
    void _tft_vcom_power(CsType cs_type = CsType::MASTER);
    void _writeColor(uint8_t color_value, CsType cs_type = CsType::MASTER);
  private:
    bool _paged;
};

#endif
