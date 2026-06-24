#ifndef __EL133UF1_H__
#define __EL133UF1_H__

#include <Arduino.h>
#include <stdlib.h>
#include "EPD_IO.h"

#define BLACK 0x00
#define WHITE 0x11
#define YELLOW 0x22
#define RED 0x33
#define BLUE 0x55
#define GREEN 0x66

const unsigned char colors[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };

#define R00_PSR             0x00
#define R01_PWR             0x01
#define R02_POF             0x02
#define R03_POFS            0x03
#define R04_PON             0x04
#define R05_BTST_N          0x05
#define R06_BTST_P          0x06
#define R10_DTM             0x10
#define R12_DRF             0x12
#define R20_LUT0            0x20
#define R30_PLL             0x30
#define R40_TSC             0x40
#define R50_CDI             0x50
#define R60_TCON            0x60
#define R61_TRES            0x61
#define R80_AMV             0x80
#define R81_VV              0x81
#define R82_VDCS            0x82
#define R83_PTLW            0x83
#define R90_PGM             0x90
#define R91_APG             0x91
#define R92_ROTP            0x92
#define RE0_CCSET           0xE0
#define RE3_PWS             0xE3
#define R07_DSLP            0x07
#define RA5_DCDC            0xA5
#define R74_AN_TM           0x74
#define R86_AGID            0x86
#define RA4_CMDA4           0xA4
#define RB0_BUCK_BOOST_VDDN 0xB0
#define RB1_TFT_VCOM_POWER  0xB1
#define RB6_EN_BUF          0xB6
#define RB7_BOOST_VDDP_EN   0xB7
#define RF0_CMD66           0xF0

// Display resolution
#define EPD_WIDTH 1200
#define EPD_HEIGHT 1600
// EL133UF1，有两个CS(Frame)每个字节包含两个像素，每个像素4bit
#define EPD_IMAGE_SIZE  (EPD_WIDTH * EPD_HEIGHT / 2)  //960000
#define EPD_FRAME_SIZE (EPD_IMAGE_SIZE / 2)  //480000

class EL133UF1 {
public:
  EL133UF1(void);
  ~EL133UF1(void);

  int EL133UF1_Init(void);
  int EL133UF1_Deinit(void);
  void EL133UF1_DisplayFrame(const unsigned char* frame_buffer_m, const unsigned char* frame_buffer_s);
  void EL133UF1_Update(void);
  void EL133UF1_Sleep(void);

  //for testing
  void EL133UF1_DisplayColor(unsigned char color, unsigned char* frame_buffer_m, unsigned char* frame_buffer_s);
  void EL133UF1_DisplayColorBar(unsigned char* frame_buffer_m, unsigned char* frame_buffer_s);
};

#endif /* __EL133UF1_H__ */

/* END OF FILE */
