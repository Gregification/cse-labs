/**
 * cse3320 Assignment 3
 * George Boone
 * 1002055713
 */

/** file allocation table(s)
 * unit : count : purpose
 *  DSK :  1... :  starting block number of a file
 */

#ifndef _FAT_H__
#define _FAT_H__

#include <string>
#include <filesystem>
#include <optional>
#include <exception>
#include "Block.hpp"

namespace fs = std::filesystem;

// file access table, some what based off FAT fs but no where near complient

class FATManager {
    fs::path diskPath;

    /** returns the block if it exists */
    std::optional<Block> getBlock(DISK_UNIT block_num);

    /** sets the block if its a valid index */
    bool setBlock(DISK_UNIT block_num, Block * const);

public:

    FATManager() = default;
    
    fs::path currentfs();

    void add_blocks(fs::path, DISK_UNIT num_blocks);
    void open_fs(fs::path);
    void save_fs(fs::path);
    void list_files(DISK_UNIT start_idx, DISK_UNIT num_to_list);
    void remove(std::string _name);
    void rename(std::string _old, std::string _new);
    void add_from_host_fs();
    void copy_to_host_fs();
    void add_link();
    void remove_link();
};

#endif /** _FAT_H__ */
