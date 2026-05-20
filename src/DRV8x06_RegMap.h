#pragma once
#include <stdint.h>

// DRV8x06-Q1 / DRV871x-Q1 SPI Register Addresses
// Source: TI DRV87xx_DRV8106-Q1EVM firmware v1.5

// DRV870xS/DRV8106S registers
#define IC_STAT_1       ((uint8_t)0x00)
#define VGS_VDS_STAT    ((uint8_t)0x01)
#define IC_STAT_2       ((uint8_t)0x02)
#define RSVD_STAT       ((uint8_t)0x03)
#define IC_CTRL         ((uint8_t)0x04)
#define BRG_CTRL        ((uint8_t)0x05)
#define DRV_CTRL_1      ((uint8_t)0x06)
#define DRV_CTRL_2      ((uint8_t)0x07)
#define DRV_CTRL_3      ((uint8_t)0x08)
#define VDS_CTRL_1      ((uint8_t)0x09)
#define VDS_CTRL_2      ((uint8_t)0x0A)
#define OLSC_CTRL       ((uint8_t)0x0B)
#define UVOV_CTRL       ((uint8_t)0x0C)
#define CSA_CTRL        ((uint8_t)0x0D)

// DRV871xS registers
#define IC_STAT_1_DRV871XX      ((uint8_t)0x00)
#define VDS_STAT1_DRV871XX      ((uint8_t)0x01)
#define VDS_STAT2_DRV871XX      ((uint8_t)0x02)
#define VGS_STAT1_DRV871XX      ((uint8_t)0x03)
#define VGS_STAT2_DRV871XX      ((uint8_t)0x04)
#define IC_STAT2_DRV871XX       ((uint8_t)0x05)
#define IC_STAT3_DRV871XX       ((uint8_t)0x06)
#define IC_CTRL1_DRV871XX       ((uint8_t)0x07)
#define IC_CTRL2_DRV871XX       ((uint8_t)0x08)
#define BRG_CTRL1_DRV871XX      ((uint8_t)0x09)
#define BRG_CTRL2_DRV871XX      ((uint8_t)0x0A)
#define PWM_CTRL1_DRV871XX      ((uint8_t)0x0B)
#define PWM_CTRL2_DRV871XX      ((uint8_t)0x0C)
#define PWM_CTRL3_DRV871XX      ((uint8_t)0x0D)
#define PWM_CTRL4_DRV871XX      ((uint8_t)0x0E)
#define IDRV_CTRL1_DRV871XX     ((uint8_t)0x0F)
#define IDRV_CTRL2_DRV871XX     ((uint8_t)0x10)
#define IDRV_CTRL3_DRV871XX     ((uint8_t)0x11)
#define IDRV_CTRL4_DRV871XX     ((uint8_t)0x12)
#define IDRV_CTRL5_DRV871XX     ((uint8_t)0x13)
#define IDRV_CTRL6_DRV871XX     ((uint8_t)0x14)
#define IDRV_CTRL7_DRV871XX     ((uint8_t)0x15)
#define IDRV_CTRL8_DRV871XX     ((uint8_t)0x16)
#define IDRV_CTRL9_DRV871XX     ((uint8_t)0x17)
#define DRV_CTRL1_DRV871XX      ((uint8_t)0x18)
#define DRV_CTRL2_DRV871XX      ((uint8_t)0x19)
#define DRV_CTRL3_DRV871XX      ((uint8_t)0x1A)
#define DRV_CTRL4_DRV871XX      ((uint8_t)0x1B)
#define DRV_CTRL5_DRV871XX      ((uint8_t)0x1C)
#define DRV_CTRL6_DRV871XX      ((uint8_t)0x1D)
#define DRV_CTRL7_DRV871XX      ((uint8_t)0x1E)
#define VDS_CTRL1_DRV871XX      ((uint8_t)0x1F)
#define VDS_CTRL2_DRV871XX      ((uint8_t)0x20)
#define VDS_CTRL3_DRV871XX      ((uint8_t)0x21)
#define VDS_CTRL4_DRV871XX      ((uint8_t)0x22)
#define OLSC_CTRL1_DRV871XX     ((uint8_t)0x23)
#define OLSC_CTRL2_DRV871XX     ((uint8_t)0x24)
#define UVOV_CTRL_DRV871XX      ((uint8_t)0x25)
#define CSA_CTRL1_DRV871XX      ((uint8_t)0x26)
#define CSA_CTRL2_DRV871XX      ((uint8_t)0x27)
#define CSA_CTRL3_DRV871XX      ((uint8_t)0x28)
#define AGD_CTRL1_DRV871XX      ((uint8_t)0x2A)
#define PDR_CTRL1_DRV871XX      ((uint8_t)0x2B)
#define PDR_CTRL2_DRV871XX      ((uint8_t)0x2C)
#define PDR_CTRL3_DRV871XX      ((uint8_t)0x2D)
#define PDR_CTRL4_DRV871XX      ((uint8_t)0x2E)
#define PDR_CTRL5_DRV871XX      ((uint8_t)0x2F)
#define PDR_CTRL6_DRV871XX      ((uint8_t)0x30)
#define PDR_CTRL7_DRV871XX      ((uint8_t)0x31)
#define PDR_CTRL8_DRV871XX      ((uint8_t)0x32)
#define PDR_CTRL9_DRV871XX      ((uint8_t)0x33)
#define PDR_CTRL10_DRV871XX     ((uint8_t)0x34)
#define STC_CTRL1_DRV871XX      ((uint8_t)0x35)
#define STC_CTRL2_DRV871XX      ((uint8_t)0x36)
#define STC_CTRL3_DRV871XX      ((uint8_t)0x37)
#define STC_CTRL4_DRV871XX      ((uint8_t)0x38)
#define DCC_CTRL1_DRV871XX      ((uint8_t)0x39)
#define PST_CTRL1_DRV871XX      ((uint8_t)0x3A)
#define PST_CTRL2_DRV871XX      ((uint8_t)0x3B)
#define SGD_STAT1_DRV871XX      ((uint8_t)0x3C)
#define SGD_STAT2_DRV871XX      ((uint8_t)0x3D)
#define SGD_STAT3_DRV871XX      ((uint8_t)0x3E)

// IC_CTRL / IC_CTRL1 bit masks
#define CLR_FLT_MASK    ((uint8_t)0x01)

// BRG_CTRL PWM mode field (bits 6:5), DRV870x/DRV8106
#define BRG_CTRL_PWM_MODE_MASK  ((uint8_t)0x60)
#define BRG_CTRL_PWM_MODE_SHIFT 5

// IC_CTRL1 PWM mode field (bits 5:4), DRV871x
#define IC_CTRL1_PWM_MODE_MASK  ((uint8_t)0x30)
#define IC_CTRL1_PWM_MODE_SHIFT 4

// Mirrored register image — one struct covers both chip families
typedef struct {
    // DRV870xS / DRV8106S
    uint8_t ic_status_1_reg;
    uint8_t vgs_vds_stat_reg;
    uint8_t ic_status_2_reg;
    uint8_t rsvd_stat_reg;
    uint8_t ic_ctrl_reg;
    uint8_t brg_ctrl_reg;
    uint8_t drv_ctrl_1_reg;
    uint8_t drv_ctrl_2_reg;
    uint8_t drv_ctrl_3_reg;
    uint8_t vds_ctrl_1_reg;
    uint8_t vds_ctrl_2_reg;
    uint8_t olsc_ctrl_reg;
    uint8_t uvov_ctrl_reg;
    uint8_t csa_ctrl_reg;

    // DRV871xS
    uint8_t ic_stat1_reg;
    uint8_t vds_stat1_reg;
    uint8_t vds_stat2_reg;
    uint8_t vgs_stat1_reg;
    uint8_t vgs_stat2_reg;
    uint8_t ic_stat2_reg;
    uint8_t ic_stat3_reg;
    uint8_t ic_ctrl1_reg;
    uint8_t ic_ctrl2_reg;
    uint8_t brg_ctrl1_reg;
    uint8_t brg_ctrl2_reg;
    uint8_t pwm_ctrl1_reg;
    uint8_t pwm_ctrl2_reg;
    uint8_t pwm_ctrl3_reg;
    uint8_t pwm_ctrl4_reg;
    uint8_t idrv_ctrl1_reg;
    uint8_t idrv_ctrl2_reg;
    uint8_t idrv_ctrl3_reg;
    uint8_t idrv_ctrl4_reg;
    uint8_t idrv_ctrl5_reg;
    uint8_t idrv_ctrl6_reg;
    uint8_t idrv_ctrl7_reg;
    uint8_t idrv_ctrl8_reg;
    uint8_t idrv_ctrl9_reg;
    uint8_t drv_ctrl1_reg;
    uint8_t drv_ctrl2_reg;
    uint8_t drv_ctrl3_reg;
    uint8_t drv_ctrl4_reg;
    uint8_t drv_ctrl5_reg;
    uint8_t drv_ctrl6_reg;
    uint8_t drv_ctrl7_reg;
    uint8_t vds_ctrl1_reg;
    uint8_t vds_ctrl2_reg;
    uint8_t vds_ctrl3_reg;
    uint8_t vds_ctrl4_reg;
    uint8_t olsc_ctrl1_reg;
    uint8_t olsc_ctrl2_reg;
    uint8_t csa_ctrl1_reg;
    uint8_t csa_ctrl2_reg;
    uint8_t csa_ctrl3_reg;
    uint8_t agd_ctrl1_reg;
    uint8_t pdr_ctrl1_reg;
    uint8_t pdr_ctrl2_reg;
    uint8_t pdr_ctrl3_reg;
    uint8_t pdr_ctrl4_reg;
    uint8_t pdr_ctrl5_reg;
    uint8_t pdr_ctrl6_reg;
    uint8_t pdr_ctrl7_reg;
    uint8_t pdr_ctrl8_reg;
    uint8_t pdr_ctrl9_reg;
    uint8_t pdr_ctrl10_reg;
    uint8_t stc_ctrl1_reg;
    uint8_t stc_ctrl2_reg;
    uint8_t stc_ctrl3_reg;
    uint8_t stc_ctrl4_reg;
    uint8_t dcc_ctrl1_reg;
    uint8_t pst_ctrl1_reg;
    uint8_t pst_ctrl2_reg;
    uint8_t sgd_stat1_reg;
    uint8_t sgd_stat2_reg;
    uint8_t sgd_stat3_reg;
} DRV8x06_Q1_REG_t;
