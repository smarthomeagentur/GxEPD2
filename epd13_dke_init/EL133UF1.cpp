#include "EL133UF1.h"

const unsigned char PSR_V[2] = {
    0xDF, 0x6B

};


const unsigned char PWR_V[6]    = { 0x0F, 0x00, 0x28, 0x2C, 0x28, 0x38 };

const unsigned char POF_V[1]    = { 0x01 };    // SampleCodeFor133_20250917
const unsigned char POFS_MV[4] = {
	0x00, 0xC0, 0x03, 0xA4
};    // SampleCodeFor133_20250917

const unsigned char POFS_SV[4] = {
	0x00, 0xC0, 0x03, 0x95
};    // SampleCodeFor133_20250917
const unsigned char DRF_V[1]    = { 0x00 }; // 0: AC VCOM, VCOM Will Follow LUTC when updating image (default)
                                            // 1: DC VCOM, VCOM will always be VCOMDC when updating image
const unsigned char CDI_V[1]    = { 0x37 }; // SampleCodeFor133_20250917
const unsigned char TCON_V[2]   = { 0x03, 0x03 };
const unsigned char TRES_V[4]   = { 0x04, 0xB0, 0x03, 0x20 };
const unsigned char CMD66_V[6]  = { 0x49, 0x55, 0x13, 0x5D, 0x05, 0x10 };
const unsigned char EN_BUF_V[1] = { 0x07 };
const unsigned char CCSET_V[1]  = { 0x01 };
const unsigned char PWS_V[1]    = { 0x22 };
const unsigned char AN_TM_V[9]  = { 0xC0, 0x1C, 0x1C, 0xCC, 0xCC, 0xCC, 0x15, 0x15, 0x55 };
/*
[High I bias] => 0x00, 0x0C, 0x0C, 0xD9, 0xDD, 0xDD, 0x15, 0x15, 0x55
It is recommended to use this parameter.
Enhanced driver capability to resolve source voltage drop when updating image, but the power consumption will increase.

[Low I bias] => 0xC0, 0x1C, 0x1C, 0xCC, 0xCC, 0xCC, 0x15, 0x15, 0x55
The driving capability remains the same as the EL133UF1. 
The source voltage will drop when updating image, but the power consumption will be lower.
*/

const unsigned char AGID_V[1]  = { 0x10 };
const unsigned char CMDA4_V[9] = {0x03, 0x00, 0x01, 0x03, 0x00, 0x03, 0x00, 0x00, 0x00};

const unsigned char BTST_P_V[2]          = { 0xD8, 0x18 };
// const unsigned char BTST_P_V[2]          = { 0xE0, 0x20 }; // High I bias BTST
const unsigned char BOOST_VDDP_EN_V[1]   = { 0x01 };
const unsigned char BTST_N_V[2]          = { 0xD8, 0x18 };
// const unsigned char BTST_N_V[2]          = { 0xE0, 0x20 }; // High I bias BTST

const unsigned char BUCK_BOOST_VDDN_V[1] = { 0x01 };
const unsigned char TFT_VCOM_POWER_V[1]  = { 0x02 };

const unsigned char AMV_V[2] = {0x01, 0x00};

const unsigned char DSLP_V[1] = {0xA5};

const unsigned char DCDC_V[3] = {
	0x44, 0x54 ,0x00
};
// SampleCodeFor133_20250917
//Support DCVCOM => 0x00, 0x54 ,0x00 (default) => VDDP & VDDN Voltage Setting +-16V
//Support ACVCOM => 0x44, 0x54 ,0x00=> VDDP & VDDN Voltage Setting +-18V

const unsigned char PLL_V[1] = { 0x08 };

EPD_IO epd_io;

EL133UF1::EL133UF1() {
}

EL133UF1::~EL133UF1() {
}

int EL133UF1::EL133UF1_Deinit(void) {
  epd_io.EPD_IO_Power_Off();
  // Serial.println("EL133UF1 Power Off.");
  epd_io.EPD_IO_Deinitialize();
  // Serial.println("EL133UF1 Deinitialize.");
  return 0;
}
int EL133UF1::EL133UF1_Init(void) {
  epd_io.EPD_IO_Initialize();
  // Serial.println("EL133UF1 Initialize.");
  epd_io.EPD_IO_Power_On();
  // Serial.println("EL133UF1 Power On.");
  epd_io.EPD_IO_Reset();
  epd_io.EPD_IO_CheckBusy_H();

  epd_io.EPD_IO_WriteCommandData_2CH(R74_AN_TM, AN_TM_V, sizeof(AN_TM_V), CS_MASK_MASTER);

  epd_io.EPD_IO_WriteCommandData_2CH(RF0_CMD66, CMD66_V, sizeof(CMD66_V), CS_MASK_MASTER_SLAVE);
  epd_io.EPD_IO_WriteCommandData_2CH(R00_PSR, PSR_V, sizeof(PSR_V), CS_MASK_MASTER_SLAVE);

  epd_io.EPD_IO_WriteCommandData_2CH(RA5_DCDC, DCDC_V, sizeof(DCDC_V), CS_MASK_MASTER);

  epd_io.EPD_IO_WriteCommandData_2CH(R30_PLL, PLL_V, sizeof(PLL_V), CS_MASK_MASTER_SLAVE);
  epd_io.EPD_IO_WriteCommandData_2CH(R50_CDI, CDI_V, sizeof(CDI_V), CS_MASK_MASTER_SLAVE);
  epd_io.EPD_IO_WriteCommandData_2CH(R60_TCON, TCON_V, sizeof(TCON_V), CS_MASK_MASTER_SLAVE);

  epd_io.EPD_IO_WriteCommandData_2CH(R03_POFS, POFS_MV, sizeof(POFS_MV), CS_MASK_MASTER);
  epd_io.EPD_IO_WriteCommandData_2CH(R03_POFS, POFS_SV, sizeof(POFS_SV), CS_MASK_SLAVE);

  epd_io.EPD_IO_WriteCommandData_2CH(R86_AGID, AGID_V, sizeof(AGID_V), CS_MASK_MASTER_SLAVE);
  epd_io.EPD_IO_WriteCommandData_2CH(RE3_PWS, PWS_V, sizeof(PWS_V), CS_MASK_MASTER_SLAVE);
  epd_io.EPD_IO_WriteCommandData_2CH(RE0_CCSET, CCSET_V, sizeof(CCSET_V), CS_MASK_MASTER_SLAVE);
  epd_io.EPD_IO_WriteCommandData_2CH(R61_TRES, TRES_V, sizeof(TRES_V), CS_MASK_MASTER_SLAVE);

  epd_io.EPD_IO_WriteCommandData_2CH(RA4_CMDA4, CMDA4_V, sizeof(CMDA4_V), CS_MASK_MASTER);
  epd_io.EPD_IO_WriteCommandData_2CH(R01_PWR, PWR_V, sizeof(PWR_V), CS_MASK_MASTER);
  epd_io.EPD_IO_WriteCommandData_2CH(RB6_EN_BUF, EN_BUF_V, sizeof(EN_BUF_V), CS_MASK_MASTER);
  epd_io.EPD_IO_WriteCommandData_2CH(R06_BTST_P, BTST_P_V, sizeof(BTST_P_V), CS_MASK_MASTER);
  epd_io.EPD_IO_WriteCommandData_2CH(RB7_BOOST_VDDP_EN, BOOST_VDDP_EN_V, sizeof(BOOST_VDDP_EN_V), CS_MASK_MASTER);
  epd_io.EPD_IO_WriteCommandData_2CH(R05_BTST_N, BTST_N_V, sizeof(BTST_N_V), CS_MASK_MASTER);
  epd_io.EPD_IO_WriteCommandData_2CH(RB0_BUCK_BOOST_VDDN, BUCK_BOOST_VDDN_V, sizeof(BUCK_BOOST_VDDN_V), CS_MASK_MASTER);
  epd_io.EPD_IO_WriteCommandData_2CH(RB1_TFT_VCOM_POWER, TFT_VCOM_POWER_V, sizeof(TFT_VCOM_POWER_V), CS_MASK_MASTER);

  Serial.println("EL133UF1 Initialize OK.");
  /* EL133UF1 hardware init end */
  return 0;
}

void EL133UF1::EL133UF1_DisplayFrame(const unsigned char* frame_buffer_m, const unsigned char* frame_buffer_s) {
  // epd_io.EPD_IO_WriteCommandData_2CH(SPIM, SPIM_V, sizeof(SPIM_V), CS_MASK_MASTER_SLAVE);
  //ATTENTION: 原本是在一个CS下拉周期里完成命令和数据的发送。此处待测试
  epd_io.EPD_IO_CS_M_Ctrl(0);
  epd_io.EPD_IO_Write_byte(R10_DTM);
  epd_io.EPD_IO_WriteDataBytes(frame_buffer_m, EPD_WIDTH * EPD_HEIGHT / 4);
  epd_io.EPD_IO_CS_M_Ctrl(1);

  epd_io.EPD_IO_CS_S_Ctrl(0);
  epd_io.EPD_IO_Write_byte(R10_DTM);
  epd_io.EPD_IO_WriteDataBytes(frame_buffer_s, EPD_WIDTH * EPD_HEIGHT / 4);
  epd_io.EPD_IO_CS_S_Ctrl(1);

  Serial.println("EL133UF1_DisplayFrame Ready.");
  EL133UF1_Update();
}

void EL133UF1::EL133UF1_Update(void) {
  epd_io.EPD_IO_CS_M_Ctrl(0);
  epd_io.EPD_IO_CS_S_Ctrl(0);
  epd_io.EPD_IO_Write_byte(R04_PON);
  epd_io.EPD_IO_CS_M_Ctrl(1);
  epd_io.EPD_IO_CS_S_Ctrl(1);
  epd_io.EPD_IO_CheckBusy_H();

  epd_io.DelayMs(30);
  epd_io.EPD_IO_WriteCommandData_2CH(R12_DRF, DRF_V, sizeof(DRF_V), CS_MASK_MASTER_SLAVE);
  epd_io.EPD_IO_CheckBusy_H();

  epd_io.DelayMs(30);
  epd_io.EPD_IO_WriteCommandData_2CH(R02_POF, POF_V, sizeof(POF_V), CS_MASK_MASTER_SLAVE);
  epd_io.EPD_IO_CheckBusy_H();
  epd_io.DelayMs(1000);
}

/**
 *  @brief: After this command is transmitted, the chip would enter the 
 *          deep-sleep mode to save power. 
 *          The deep sleep mode would return to standby by hardware reset. 
 *          You can use Init() to awaken
 */
void EL133UF1::EL133UF1_Sleep(void) {

  epd_io.EPD_IO_WriteCommandData_2CH(R07_DSLP, DSLP_V, sizeof(DSLP_V),CS_MASK_MASTER_SLAVE);
  epd_io.DelayMs(100);
  // Serial.println("EL133UF1_Sleep.");
}
/* END OF FILE */


void EL133UF1::EL133UF1_DisplayColor(unsigned char color, unsigned char* frame_buffer_m, unsigned char* frame_buffer_s) {
  unsigned int i, j;

  Serial.println("EL133UF1_DisplayColor Prepare.");
  for (i = 0; i < EPD_FRAME_SIZE; i++) {
      frame_buffer_m[i] = color;
      frame_buffer_s[i] = color;
  }
  Serial.println("EL133UF1_DisplayColor Ready.");
  EL133UF1_DisplayFrame(frame_buffer_m, frame_buffer_s);
}


void EL133UF1::EL133UF1_DisplayColorBar(unsigned char* frame_buffer_m, unsigned char* frame_buffer_s) {
  unsigned int i, j;

  for (i = 0; i < EPD_HEIGHT / 8; i++) {
    for (j = 0; j < EPD_WIDTH / 4; j++) {
      frame_buffer_m[i * EPD_WIDTH / 4 + j] = WHITE;
      frame_buffer_s[i * EPD_WIDTH / 4 + j] = WHITE;

      frame_buffer_m[i * EPD_WIDTH / 4 + j + EPD_WIDTH * EPD_HEIGHT / 32] = BLACK;
      frame_buffer_s[i * EPD_WIDTH / 4 + j + EPD_WIDTH * EPD_HEIGHT / 32] = BLACK;

      frame_buffer_m[i * EPD_WIDTH / 4 + j + 2 * EPD_WIDTH * EPD_HEIGHT / 32] = WHITE;
      frame_buffer_s[i * EPD_WIDTH / 4 + j + 2 * EPD_WIDTH * EPD_HEIGHT / 32] = WHITE;

      frame_buffer_m[i * EPD_WIDTH / 4 + j + 3 * EPD_WIDTH * EPD_HEIGHT / 32] = RED;
      frame_buffer_s[i * EPD_WIDTH / 4 + j + 3 * EPD_WIDTH * EPD_HEIGHT / 32] = RED;

      frame_buffer_m[i * EPD_WIDTH / 4 + j + 4 * EPD_WIDTH * EPD_HEIGHT / 32] = YELLOW;
      frame_buffer_s[i * EPD_WIDTH / 4 + j + 4 * EPD_WIDTH * EPD_HEIGHT / 32] = YELLOW;

      frame_buffer_m[i * EPD_WIDTH / 4 + j + 5 * EPD_WIDTH * EPD_HEIGHT / 32] = BLUE;
      frame_buffer_s[i * EPD_WIDTH / 4 + j + 5 * EPD_WIDTH * EPD_HEIGHT / 32] = BLUE;

      frame_buffer_m[i * EPD_WIDTH / 4 + j + 6 * EPD_WIDTH * EPD_HEIGHT / 32] = GREEN;
      frame_buffer_s[i * EPD_WIDTH / 4 + j + 6 * EPD_WIDTH * EPD_HEIGHT / 32] = GREEN;

      frame_buffer_m[i * EPD_WIDTH / 4 + j + 7 * EPD_WIDTH * EPD_HEIGHT / 32] = BLACK;
      frame_buffer_s[i * EPD_WIDTH / 4 + j + 7 * EPD_WIDTH * EPD_HEIGHT / 32] = BLACK;
    }
  }

  EL133UF1_DisplayFrame(frame_buffer_m, frame_buffer_s);
}
