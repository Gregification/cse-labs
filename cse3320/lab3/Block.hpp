/**
 * cse3320 Assignment 3
 * George Boone
 * 1002055713
 */

/** block format
 * data type    : count : purpose
 *  BLK         :  1    : size of data
 *  byte        :  ...  : data
 *  DSK         :  -1   : next block
 */

#ifndef _BLOCK_H__
#define _BLOCK_H__

#include <array>
#include <fstream>
#include <cstdint>
#include <limits>
#include <array>

struct Block;

#define DISK_UNIT       uint16_t
#define BLOCK_UNIT      uint8_t

/** file format
 * block   : purpose
 * A       :    name
 * A       :    metadata
 * B       :    actual data
 */

/** 'disk' format
 * block #  : purpose
 *  0       : meta data
 *  1       : FAT
 *  >1      : data
 */
#define MAX_DISK_SIZE           ((DISK_UNIT)std::numeric_limits<DISK_UNIT>::max())

#define MAX_NUM_BLOCKS          ((DISK_UNIT)(MAX_DISK_SIZE / sizeof(Block)))

struct Block {
    union {
        std::array<uint8_t, std::numeric_limits<BLOCK_UNIT>::max()-1> raw_bytes;
        struct{
            DISK_UNIT   next_block;
            BLOCK_UNIT  data_len;
            char        asFileStart_name    [(sizeof(raw_bytes)-3) / 2];
            char        asFileStart_owner   [(sizeof(raw_bytes)-3) - sizeof(asFileStart_name)];
        };
    };
};

//--------------------------------------------------------------
// general tests
//--------------------------------------------------------------

static_assert(MAX_DISK_SIZE >= sizeof(Block));
static_assert(std::is_standard_layout<Block>::value);

#endif
