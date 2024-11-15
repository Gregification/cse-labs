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
#include <iostream>

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
    struct BlockStandard {
        DISK_UNIT   next_block;
        BLOCK_UNIT  data_len;
    };

    union {
        std::array<uint8_t, std::numeric_limits<BLOCK_UNIT>::max()-1> raw_bytes;
        struct{
            BlockStandard   standard;
            union{
                struct{
                    char        asFileStart_name    [(sizeof(raw_bytes)-sizeof(standard)) / 2];
                    char        asFileStart_owner   [(sizeof(raw_bytes)-sizeof(standard)) - sizeof(asFileStart_name)];
                };
                DISK_UNIT       asFAT_filestart     [(sizeof(raw_bytes)-sizeof(standard)) / sizeof(DISK_UNIT)];
                uint8_t         data_start          [sizeof(raw_bytes)-sizeof(standard)];
            };
        };
    };
};

#define MAX_DATA_SIZE           (sizeof(Block::raw_bytes)-sizeof(Block::data_start))

//--------------------------------------------------------------
// general tests
//--------------------------------------------------------------

static_assert(MAX_DISK_SIZE >= sizeof(Block) * 10);
static_assert(std::is_standard_layout<Block>::value);

int main(){
    std::cout << "block         " << std::to_string(sizeof(Block))              << std::endl;
    std::cout << "struct:       " << std::to_string(sizeof(Block::standard))    << std::endl;
    std::cout << "raw data:     " << std::to_string(sizeof(Block::raw_bytes))   << std::endl;
    std::cout << "data start:   " << std::to_string(sizeof(Block::data_start))  << std::endl;

    return 0;
}

#endif
