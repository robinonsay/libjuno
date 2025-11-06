/*
    MIT License

    Copyright (c) 2025 Robin A. Onsay

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files
    (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.
*/
/**
 * @file crc.h
 * @brief CRC update routines (ARC, BinHex, CCITT, CCITT32, Kermit, ZIP).
 * @defgroup juno_crc CRC utilities
 * @ingroup juno_math
 * @details
 *  Streaming update functions for several common CRC variants implemented in
 *  src/juno_*.c. Each function updates a running CRC with a data buffer and
 *  returns the new CRC value. Initial (seed) values are provided as macros
 *  (JUNO_*_CRC_INIT) for first use.
 *
 *  Behavior and usage
 *  - Passing a NULL data pointer or zero length returns 0 (no update).
 *    This differs from returning the variant's seed value for some CRCs
 *    (e.g., CCITT/CCITT32/ZIP) and is intentional; see tests and callers.
 *  - All functions are pure and reentrant: no global state is modified.
 *  - CRC-32 ZIP applies a final XOR (0xFFFFFFFF) before returning when data
 *    is processed. For multi-chunk streaming with ZIP, undo that XOR on the
 *    returned value (res ^ 0xFFFFFFFF) before passing it back as the next
 *    iCrc.
 *
 *  Example (streaming across two chunks)
 *  @code{.c}
 *  // ARC (16-bit, no final XOR):
 *  uint16_t crc = JUNO_ARC_CRC_INIT;
 *  crc = Juno_CrcArcUpdate(crc, chunk1, len1);
 *  crc = Juno_CrcArcUpdate(crc, chunk2, len2);
 *
 *  // ZIP (CRC-32) with final XOR at each call boundary:
 *  uint32_t zip = JUNO_ZIP_CRC_INIT;
 *  zip = Juno_CrcZipUpdate(zip, chunk1, len1);           // returns XOR'ed
 *  zip = Juno_CrcZipUpdate(zip ^ 0xFFFFFFFF, chunk2, len2); // undo XOR first
 *  @endcode
 */
#ifndef JUNO_CRC_H
#define JUNO_CRC_H
#ifdef __cplusplus
extern "C"
{
#endif
#include <stddef.h>
#include <stdint.h>

/**
 * @name Internal masks (implementation detail)
 * @brief Bit masks used by table-driven CRC update routines.
 * @{ 
 */
#define M1_16 0xff        /**< Low byte mask for 16-bit CRC. */
#define M2_16 0xff00      /**< High byte mask for 16-bit CRC. */
#define M1_32 0xffffffff  /**< Low word mask for 32-bit CRC. */
#define M2_32 0xffffff00  /**< High word mask for 32-bit CRC. */
/** @} */

/**
 * @brief Initial seed for CRC-16/ARC (IBM-16) computations.
 * @details Expands to 0 (all zeros). Use as the first @p iCrc value when
 *  starting a new ARC CRC computation.
 */
#define JUNO_ARC_CRC_INIT (0)
/**
 * @brief Update ARC CRC with new data.
 * @param iCrc     Running CRC value (use JUNO_ARC_CRC_INIT for first chunk).
 * @param pcData   Pointer to data buffer (can be NULL when @p zDataSize is 0).
 * @param zDataSize Size of data buffer in bytes.
 * @return Updated CRC value; returns 0 when @p pcData is NULL or @p zDataSize is 0.
 */
uint16_t Juno_CrcArcUpdate(uint16_t iCrc, const void *pcData, size_t zDataSize);


/**
 * @brief Initial seed for BinHex 16-bit CRC computations.
 * @details Expands to 0 (all zeros). Use as the first @p iCrc value for BinHex.
 */
#define JUNO_BINHEX_CRC_INIT (0)
/**
 * @brief Update BinHex CRC with new data.
 * @param iCrc     Running CRC value (use JUNO_BINHEX_CRC_INIT initially).
 * @param pcData   Pointer to data buffer (can be NULL when @p zDataSize is 0).
 * @param zDataSize Size of data buffer in bytes.
 * @return Updated CRC value; returns 0 when @p pcData is NULL or @p zDataSize is 0.
 */
uint16_t Juno_CrcBinhexUpdate(uint16_t iCrc, const void *pcData, size_t zDataSize);


/**
 * @brief Initial seed for CRC-16/CCITT computations (0xFFFF).
 * @details Expands to -1 (all ones) as a C integer literal; when passed to the
 *  16-bit function parameter it yields 0xFFFF. Use as the first @p iCrc value
 *  when starting a new CCITT CRC computation.
 */
#define JUNO_CCITT_CRC_INIT (-1)
/**
 * @brief Update CCITT CRC (16-bit) with new data.
 * @param iCrc     Running CRC value (use JUNO_CCITT_CRC_INIT initially).
 * @param pcData   Pointer to data buffer (can be NULL when @p zDataSize is 0).
 * @param zDataSize Size of data buffer in bytes.
 * @return Updated CRC value; returns 0 when @p pcData is NULL or @p zDataSize is 0.
 */
uint16_t Juno_CrcCcittUpdate(uint16_t iCrc, const void *pcData, size_t zDataSize);


/**
 * @brief Initial seed for 32-bit CCITT-style CRC computations (0xFFFFFFFF).
 * @details Expands to -1 (all ones) as a C integer literal; when passed to the
 *  32-bit function parameter it yields 0xFFFFFFFF. Use as the first @p iCrc
 *  value when starting a new CCITT32 computation.
 */
#define JUNO_CCITT32_CRC_INIT (-1)
/**
 * @brief Update CCITT32 CRC (32-bit) with new data.
 * @param iCrc     Running CRC value (use JUNO_CCITT32_CRC_INIT initially).
 * @param pcData   Pointer to data buffer (can be NULL when @p zDataSize is 0).
 * @param zDataSize Size of data buffer in bytes.
 * @return Updated CRC value; returns 0 when @p pcData is NULL or @p zDataSize is 0.
 */
uint32_t Juno_CrcCcitt32Update(uint32_t iCrc, const void *pcData, size_t zDataSize);


/**
 * @brief Initial seed for CRC-16/KERMIT computations.
 * @details Expands to 0 (all zeros). Use as the first @p iCrc value for Kermit.
 */
#define JUNO_KERMIT_CRC_INIT (0)
/**
 * @brief Update Kermit CRC with new data.
 * @param iCrc     Running CRC value (use JUNO_KERMIT_CRC_INIT initially).
 * @param pcData   Pointer to data buffer (can be NULL when @p zDataSize is 0).
 * @param zDataSize Size of data buffer in bytes.
 * @return Updated CRC value; returns 0 when @p pcData is NULL or @p zDataSize is 0.
 */
uint16_t Juno_CrcKermitUpdate(uint16_t iCrc, const void *pcData, size_t zDataSize);

/**
 * @brief Initial seed for ZIP/PKZIP CRC-32 computations (0xFFFFFFFF).
 * @details Expands to -1 (all ones) as a C integer literal; when passed to the
 *  32-bit function parameter it yields 0xFFFFFFFF. Use as the first @p iCrc
 *  value when starting a new ZIP CRC computation.
 */
#define JUNO_ZIP_CRC_INIT (-1)
/**
 * @brief Update ZIP CRC (CRC-32) with new data.
 * @details Applies a final XOR (0xFFFFFFFF) to the computed value before
 *  returning when data is processed. If streaming across multiple chunks,
 *  undo the XOR on the returned value before using it as the next @p iCrc.
 *  When @p pcData is NULL or @p zDataSize is 0, the function returns 0
 *  (no XOR is applied in that case).
 * @param iCrc     Running CRC value (use JUNO_ZIP_CRC_INIT initially).
 * @param pcData   Pointer to data buffer (can be NULL when @p zDataSize is 0).
 * @param zDataSize Size of data buffer in bytes.
 * @return Updated CRC-32 value; returns 0 when @p pcData is NULL or @p zDataSize is 0.
 */
uint32_t Juno_CrcZipUpdate(uint32_t iCrc, const void *pcData, size_t zDataSize);

#ifdef __cplusplus
}
#endif
#endif
