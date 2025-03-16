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

#include <cstring>
#include <string>
#include <filesystem>
#include <optional>
#include <exception>
#include <iterator>
#include <vector>
#include "Block.hpp"

namespace fs = std::filesystem;

// file access table, some what based off FAT fs but no where near complient

class FATManager {
    fs::path diskPath;
public:
    struct FATfileinfo;

    /** returns the block if it exists */
    static std::optional<Block> getBlock(fs::path diskPath, DISK_UNIT block_num);

    /** sets the block if its a valid index */
    static bool setBlock(fs::path diskPath, DISK_UNIT block_num, Block * const);

    FATManager() = default;
    
    fs::path currentfs();

    void add_blocks(DISK_UNIT num_blocks);
    void open_fs(fs::path);
    void save_fs(fs::path);
    void list_files(DISK_UNIT start_idx, DISK_UNIT num_to_list);
    void remove(std::string _name);
    void rename(std::string _old, std::string _new);
    void add_from_host_fs(fs::path);
    void copy_to_host_fs(std::string );
    void add_link(fs::path to, std::string link);
    void set_user(fs::path, std::string);
    void remove_link(std::string link);
    FATfileinfo find_by_name(std::string);
    DISK_UNIT total_num_blocks();
    std::vector<DISK_UNIT> find_empty_blocks(DISK_UNIT num_blocks);
    void formatfs();

    void add_fat_entry(DISK_UNIT block_num, fs::path diskPath);
    /** removes internal data and next block pointer */
    void unset_block(DISK_UNIT);

    class Iterator : public std::iterator<std::forward_iterator_tag, Block> {
    private:
        Block current;
        DISK_UNIT block_num = 0;
        bool isbad = false;
    public:
        // DISK_UNIT block_num; //gets messy if this is here
        fs::path diskPath;

        Iterator(Block blk, fs::path diskPath) : current(blk), diskPath(diskPath) {}

        Block operator*() const { return current; }
        Iterator& operator++() {
            auto blk_option = getBlock(diskPath, current.standard.next_block);
            isbad = !blk_option.has_value() || ((blk_option.value().standard.next_block == block_num) && block_num != 0);
            block_num = current.standard.next_block;

            if(isbad)
                current = Block{};
            else
                current = blk_option.value();

            return *this;
        }
        bool operator!=(const Iterator& other) const {
            return isbad ? other.isbad : (std::memcmp(&current, &other.current, sizeof(Block)) != 0);
        }
    };

    Iterator begin(Block head, DISK_UNIT block_num)  { block_num = block_num; return Iterator(head, diskPath); }
    Iterator end()              { return Iterator(Block{}, diskPath); }


    struct FATfileinfo{
        DISK_UNIT   headder_block_number    = 0;
        DISK_UNIT   FAT_block_number;
        BLOCK_UNIT  FAT_block_entry_number;
        Block       headder_block;
        Block       FAT_block;

        bool is_bad(){ return headder_block_number == 0; }
    };
};

#endif /** _FAT_H__ */
