/**
 * Copyrights (C) 2015 - 2020
 * All rights reserved.
 *
 * File : crc32.h
 *
 * 生成 CRC32 校验码
 *
 * Change Logs:
 * Date         Author          Note
 * 2015-08-19   chenfei         Create this file
 */
#ifndef __CRC32_H__
#define __CRC32_H__

#include <stdint.h>

#ifdef  __cplusplus
extern "C" {
#endif

int crc32_check(const uint8_t *buf, int size);

uint32_t crc32(uint32_t crc, const uint8_t *buf, uint32_t size);

uint16_t crc16(unsigned char *puchMsg, uint32_t usDataLen);

#ifdef  __cplusplus
}
#endif

#endif  //__CRC32_H__
