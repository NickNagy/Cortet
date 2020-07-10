#ifndef EXTERNAL_SRAM_H
#define EXTERNAL_SRAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "fmc.h"

#define EXTERNAL_SRAM_BANK          2
#define EXTERNAL_SRAM_BASE_ADDRESS  NORSRAM_BANK_BASE_ADDRESS(EXTERNAL_SRAM_BANK)

/* HM6116-3:
 *
 * 2048x8b
 *
 *             | Min | Max
 * ------------------------
 * tCYC(WRITE) |150ns| -
 * tCYC(READ)  |150ns| -
 * tWRLW       | 90ns| -
 * tACC        |  -  |150ns
 * tAS         | 20ns| -
 */

#define EXTERNAL_SRAM_ID HM6116_3

// feel free to add own cases
#if EXTERNAL_SRAM_ID == HM6116_3
#define EXTERNAL_SRAM_DATA_SIZE  8
#define EXTERNAL_SRAM_SIZE       2048
#define EXTERNAL_SRAM_TCYC_WRITE 150
#define EXTERNAL_SRAM_TCYC_READ  150
#define EXTERNAL_SRAM_TWRLW		 90
#define EXTERNAL_SRAM_TACC		 150
#define EXTERNAL_SRAM_TAS		 20
#else
#endif

#if EXTERNAL_SRAM_DATA_SIZE == 8
#define EXTERNAL_SRAM_DATA_T uint8_t
#define EXTERNAL_SRAM_SEND_DATA(address, data) FMC_SRAM_8BIT_SEND_DATA(EXTERNAL_SRAM_BANK, address, data)
#define EXTERNAL_SRAM_READ_DATA(address) FMC_SRAM_8BIT_READ_DATA(EXTERNAL_SRAM_BANK, address)
#elif EXTERNAL_SRAM_DATA_SIZE == 16
#define EXTERNAL_SRAM_DATA_T uint16_t
#define EXTERNAL_SRAM_SEND_DATA(address, data) FMC_SRAM_16BIT_SEND_DATA(EXTERNAL_SRAM_BANK, address, data)
#define EXTERNAL_SRAM_READ_DATA(address) FMC_SRAM_16BIT_READ_DATA(EXTERNAL_SRAM_BANK, address)
#else // 32bit
#define EXTERNAL_SRAM_DATA_T uint32_t
#define EXTERNAL_SRAM_SEND_DATA(address, data) FMC_SRAM_32BIT_SEND_DATA(EXTERNAL_SRAM_BANK, address, data)
#define EXTERNAL_SRAM_READ_DATA(address) FMC_SRAM_32BIT_READ_DATA(EXTERNAL_SRAM_BANK, address)
#endif

#define EXTERNAL_SRAM_PTR_T EXTERNAL_SRAM_DATA_T*

#ifdef __cplusplus
}
#endif

#endif
