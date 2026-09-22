#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>

/* ------------------------------------------------------------------ */
/* M5Stack Tab5 (ST7121 TDDI) — NABO / XiaoZhi board config             */
/* Ref: https://docs.m5stack.com/en/core/Tab5                           */
/* ------------------------------------------------------------------ */

#define AUDIO_INPUT_SAMPLE_RATE  24000
#define AUDIO_OUTPUT_SAMPLE_RATE 24000
#define AUDIO_INPUT_REFERENCE    true

/* I2S — ES8388 playback / ES7210 capture share bus timing */
#define AUDIO_I2S_GPIO_MCLK      GPIO_NUM_30
#define AUDIO_I2S_GPIO_WS        GPIO_NUM_29
#define AUDIO_I2S_GPIO_BCLK      GPIO_NUM_27
#define AUDIO_I2S_GPIO_DIN       GPIO_NUM_28  /* ES7210 ASDOUT */
#define AUDIO_I2S_GPIO_DOUT      GPIO_NUM_26  /* ES8388 DSDIN */

#define AUDIO_CODEC_I2C_SDA_PIN  GPIO_NUM_31
#define AUDIO_CODEC_I2C_SCL_PIN  GPIO_NUM_32
#define AUDIO_CODEC_ES8388_ADDR  0x10
#define AUDIO_CODEC_ES7210_ADDR  0x40
/* SPK_EN is on PI4IOE-1 P1, not a raw GPIO */
#define AUDIO_CODEC_PA_PIN       GPIO_NUM_NC

#define BOOT_BUTTON_GPIO         GPIO_NUM_0

/* Display (MIPI-DSI ST7121 / ILI9881C legacy) */
#define DISPLAY_WIDTH            1280
#define DISPLAY_HEIGHT           720
#define DISPLAY_MIRROR_X         false
#define DISPLAY_MIRROR_Y         false
#define DISPLAY_SWAP_XY          false
#define DISPLAY_OFFSET_X         0
#define DISPLAY_OFFSET_Y         0
#define DISPLAY_BACKLIGHT_PIN    GPIO_NUM_22
#define DISPLAY_BACKLIGHT_OUTPUT_INVERT true

/* Panel batch: 0=ILI9881C+GT911, 1=ST7121 TDDI (this SKU) */
#define NABO_PANEL_ST7121        1

/* Touch — ST7121 TDDI (addr 0x55); legacy GT911 used 0x14/0x5D */
#define TOUCH_I2C_ADDR           0x55
#define TOUCH_INT_GPIO           GPIO_NUM_23
#define TOUCH_RST_VIA_PI4IOE     1  /* E1.P5 TP_RST */

/* I2C shared: codec, touch, BMI270(0x68), RX8130CE(0x32), INA226(0x41), PI4IOE */
#define BOARD_I2C_SDA_PIN        GPIO_NUM_31
#define BOARD_I2C_SCL_PIN        GPIO_NUM_32

#define PI4IOE1_ADDR             0x43
#define PI4IOE2_ADDR             0x44

/* Power / battery (INA226 + IP2326 via PI4IOE) */
#define HAS_INA226               1
#define INA226_ADDR              0x41
#define HAS_RTC_RX8130CE         1
#define RTC_RX8130CE_ADDR        0x32

/* BMI270 */
#define HAS_BMI270               1
#define BMI270_ADDR              0x68

/* microSD — SDMMC 4-bit */
#define SDMMC_CLK_PIN            GPIO_NUM_43
#define SDMMC_CMD_PIN            GPIO_NUM_44
#define SDMMC_D0_PIN             GPIO_NUM_39
#define SDMMC_D1_PIN             GPIO_NUM_40
#define SDMMC_D2_PIN             GPIO_NUM_41
#define SDMMC_D3_PIN             GPIO_NUM_42

/* NABO assets root on SD */
#define NABO_ASSET_ROOT          "/sdcard/nabo"

#endif /* _BOARD_CONFIG_H_ */
