/**
 * cse3320 Assignment 3
 * George Boone
 * 1002055713
 */

#include "FAT.hpp"

std::optional<std::ifstream> getifstream(fs::path pth){
    if(fs::exists(pth)){
        std::ifstream fin(pth);
        
        if(fin.is_open()){
            return std::move(fin);
        } else 
            throw std::runtime_error("unable to open file");

    } else
        throw std::runtime_error("file does not exist");
}
std::optional<std::ofstream> getofstream(fs::path pth){
    if(fs::exists(pth)){
        std::ofstream fin(pth);
        
        if(fin.is_open()){
            return std::move(fin);
        } else 
            throw std::runtime_error("unable to open file");

    } else
        throw std::runtime_error("file does not exist");
}

std::optional<Block> FATManager::getBlock(DISK_UNIT block_num){
    if(auto fin = getifstream(diskPath)){
        fin->seekg(sizeof(Block) * block_num, std::ios::beg);

        Block blk;
        fin->read(reinterpret_cast<char*>(&blk), sizeof(blk));

        fin->close();

        // if no errors during read
        if(fin.value())
            return {blk};
    }

    return {};
}

bool FATManager::setBlock(DISK_UNIT block_num, Block * const blk){
    if(auto fout = getofstream(diskPath)){
        fout->seekp(sizeof(Block) * block_num, std::ios::beg);

        fout->write(reinterpret_cast<char*>(blk), sizeof(Block));

        fout->close();

        // if no errors during read
        if(fout.value())
            return true;
    } 

    return {};
}

void FATManager::add_blocks(fs::path, DISK_UNIT num_blocks){
    if(auto fout = getofstream(diskPath)){
        fout->seekp(0, std::ios_base::end);

        // struct initilization dosent play well with unions
        Block fill_blk;
            fill_blk.data_len   = 0;
            fill_blk.next_block = 0;

        for(; num_blocks > 0; num_blocks--){
            fout->write(reinterpret_cast<char*>(&fill_blk), sizeof(Block));
        }

        fout->close();
    }
}

void FATManager::open_fs(fs::path pth){
    diskPath = pth;
}

void FATManager::save_fs(fs::path pth){
    // everyhting is already saved
}

void FATManager::list_files(){
    if(auto fin = getifstream(diskPath)){
        fin->seekg(sizeof(Block), std::ios::beg);

        Block blk;
        fin->read(reinterpret_cast<char*>(&blk), sizeof(blk));

        fin->close();

        // if no errors during read
        if(fin.value())
            return {blk};
    }
}

void FATManager::remove(std::string _name){

}

void FATManager::rename(std::string _old, std::string _new){

}

void FATManager::add_from_host_fs(){

}

void FATManager::copy_to_host_fs(){

}

void FATManager::add_link(){

}

void FATManager::remove_link(){

}
