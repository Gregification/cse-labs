/**
 * cse3320 Assignment 3
 * George Boone
 * 1002055713
 */

#include "FAT.hpp"
#include <iostream>
#include <cstddef>

fs::path FATManager::currentfs(){
    return diskPath;
}

Block getBlockThrowable(fs::path diskPath, DISK_UNIT block_num){
    if(auto op = FATManager::getBlock(diskPath, block_num))
        if(op.has_value())
            return op.value();

    throw std::runtime_error("unable to fetch from disk! block #" + std::to_string(block_num));
}

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
    std::ofstream fin(pth);
        
    if(fin.is_open()){
        return std::move(fin);
    } else 
        throw std::runtime_error("getofstream: unable to open file");
}

std::optional<Block> FATManager::getBlock(fs::path diskPath, DISK_UNIT block_num){
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

bool FATManager::setBlock(fs::path diskPath, DISK_UNIT block_num, Block * const blk){
    if(auto fout = getofstream(diskPath)){
        fout->seekp(sizeof(Block) * block_num, std::ios::beg);

        fout->write(reinterpret_cast<char*>(blk), sizeof(Block));

        fout->close();

        return !(fout.value().bad());            
    } 

    return false;
}

void FATManager::add_blocks(DISK_UNIT num_blocks){
    if(auto fout = getofstream(diskPath)){
        fout->seekp(0, std::ios_base::end);

        // struct initilization dosent play well with unions
        Block fill_blk;
            fill_blk.standard.data_len   = 0;
            fill_blk.standard.next_block = 0;

        for(; num_blocks > 0; num_blocks--){
            fout->write(reinterpret_cast<char*>(&fill_blk), sizeof(Block));
        }

        fout->close();
    }
}

void FATManager::open_fs(fs::path pth){
    diskPath = pth;
    if(!fs::exists(pth)){
        add_blocks(1);
    }
}

void FATManager::save_fs(fs::path pth){
    // everyhting is already saved
}

void FATManager::list_files(DISK_UNIT start_idx, DISK_UNIT num_to_list){
    Block fat = getBlockThrowable(diskPath, 0);

    for(auto it = begin(fat, 0); it != end() && num_to_list > 0; ++it){
        // std::cout << "meow" << std::endl;

        // if not at start of page
        if(start_idx > fat.standard.data_len){
            // seek to start of page
            start_idx -= fat.standard.data_len;

        } else {    
            // print FAT entries
            for(BLOCK_UNIT i = 0; i < fat.standard.data_len && num_to_list > 0; i++){
                
                if(fat.asFAT_filestart[i] != 0){ // if is valid block
                    num_to_list--;

                    char buf[sizeof(Block::asFileStart_name)+1];

                    snprintf(buf, sizeof(buf)-1, "%s", (*it).asFileStart_name);
                    std::cout << buf;

                    snprintf(buf, sizeof(buf)-1, "%s", (*it).asFileStart_owner);
                    std::cout << "\t\t\t\t" <<buf << std::endl;
                }
            }

            if(num_to_list == 0)
                break;
        }
    }
}

void FATManager::remove(std::string _name){
    auto finfo = find_by_name(_name);

    if(finfo.is_bad()){
        std::cout << "local file not found" << std::endl;
        return;
    }

    //replace old entry with the last of the list, reduce length counter
    finfo.FAT_block.asFAT_filestart[finfo.FAT_block_entry_number] = finfo.FAT_block.asFAT_filestart[finfo.FAT_block.standard.data_len-1];
    finfo.FAT_block.standard.data_len--;

    setBlock(diskPath, finfo.FAT_block_number, &finfo.FAT_block);
}

void FATManager::rename(std::string _old, std::string _new){
    auto finfo = find_by_name(_old);

    if(finfo.is_bad()){
        std::cout << "local file not found" << std::endl;
        return;
    }

    snprintf(finfo.headder_block.asFileStart_name, std::min(sizeof(Block::asFileStart_name), _new.size()), "%s", _new.c_str());

    setBlock(diskPath, finfo.headder_block_number, &finfo.headder_block);
}

void FATManager::add_from_host_fs(fs::path host_path){
    if(!fs::exists(host_path)){
        std::cout << "file does not exist on host: " << host_path.string() << std::endl;
        return;
    }

    Block blk;
        blk.standard.data_len   = MAX_DATA_SIZE;
        blk.standard.next_block = 0;
        
    // former block starts off as the header
    // set name of link
    snprintf(blk.asFileStart_name, std::min(sizeof(Block::asFileStart_name), host_path.filename().string().size()), "%s", host_path.filename().string().c_str());

    DISK_UNIT former_block_num = find_empty_blocks(1)[0];
    std::cout << "\tD:header, as block: " << std::to_string(former_block_num) << std::endl;

    std::cout << "adding fat entry" << std::endl;
    add_fat_entry(former_block_num, diskPath);

    if(auto fin = getifstream(diskPath)){
        while(!fin->eof()){
            { // point old block to new one then save the old 
                std::cout << "finding new empty block ... ";
                DISK_UNIT empty_blk = find_empty_blocks(1)[0];
                blk.standard.next_block = empty_blk;
                std::cout << " found block #" << std::to_string(empty_blk) << std::endl;

                std::cout << "\t writing block: " << std::to_string(former_block_num) << std::endl;
                setBlock(diskPath, former_block_num, &blk);

                former_block_num = empty_blk;
            }

            fin->read(
                    reinterpret_cast<char*>((&blk) + offsetof(Block, Block::data_start)),
                    sizeof(Block::data_start)
                );
            
            std::cout << "set size ";
            if(fin->eof()){
                blk.standard.data_len = MAX_DATA_SIZE;
            } else 
                blk.standard.data_len = MAX_DATA_SIZE - fin->tellg() % MAX_DATA_SIZE;
            std::cout << "to " << std::to_string(blk.standard.data_len);
            std::cout << " out of " << std::to_string(MAX_DATA_SIZE) << std::endl;
        }
        fin->close();

        //save the final block
        setBlock(diskPath, former_block_num, &blk);
        
    }
    std::cout << "a" << std::endl;
}

void FATManager::copy_to_host_fs(std::string _name){
    auto finfo = find_by_name(_name);
    if(finfo.is_bad()){
        std::cout << "local file not found" << std::endl;
        return;
    }

    if(auto fout = getofstream(diskPath.append("../" + std::to_string(finfo.headder_block_number))) ) {
        for(auto fat = begin(getBlockThrowable(diskPath, finfo.headder_block_number), finfo.headder_block_number); fat != end(); ++fat){
            fout->write(
                    reinterpret_cast<char const *>((*fat).data_start),
                    (*fat).standard.data_len
                );
        }
        
        fout->close();
    }

}

void FATManager::add_link(fs::path pth, std::string link){
    auto finfo = find_by_name(link);
    if(finfo.is_bad()){
        std::cout << "file not found" << std::endl;
        return;
    }

    // create header block for the link
    DISK_UNIT link_header_block = find_empty_blocks(1)[0];
    {
        Block blk;
            blk.standard.data_len   = MAX_FILE_ENTRIES;
            blk.standard.next_block = finfo.headder_block_number; // link to the original file
        
        // set name of link
        snprintf(blk.asFileStart_name, std::min(sizeof(Block::asFileStart_name), link.size()), "%s", link.c_str());

        setBlock(diskPath, link_header_block, &blk);
    }

    // add FAT entry for the new header block
    {   
        // add to origional FAT if possible
        auto fat_blk = getBlockThrowable(diskPath, finfo.FAT_block_number);
        if(fat_blk.standard.data_len != (sizeof(Block::asFAT_filestart) / sizeof(fat_blk.asFAT_filestart[0])) ) { // if not full
            
            fat_blk.asFAT_filestart[fat_blk.standard.data_len++] = link_header_block; // set last entry
            
            setBlock(diskPath, finfo.FAT_block_number, &fat_blk);
            return;
        }

        // otherwise, make a new FAT block to add it to
        DISK_UNIT new_fat_block_num = find_empty_blocks(1)[0];
        {
            Block blk;
                blk.standard.data_len = 1;
                blk.asFAT_filestart[0] = link_header_block;
            setBlock(diskPath, new_fat_block_num, &blk);
        }

        // link new FAT block to FAT list
        {
            // find end of list
            BLOCK_UNIT blk_num = finfo.FAT_block_number;
            for(auto fat = begin(getBlockThrowable(diskPath, blk_num), blk_num); fat != end(); ++fat){
                if((*fat).standard.next_block == 0){
                    // update the end of the list
                    // i raelly should make some macro for this
                    if(auto fout = getofstream(diskPath)){
                        fout->seekp(sizeof(Block) * blk_num + offsetof(Block, standard.next_block), std::ios::beg);
                        
                        Block::BlockStandard stdd = {
                            .next_block = 0,
                        }; 
                        fout->write(reinterpret_cast<char*>(&stdd.next_block), sizeof(Block::BlockStandard::next_block));

                        fout->close();
                    }
                    break;
                }

                blk_num = (*fat).standard.next_block;
            }
        }

    }
}

void FATManager::set_user(fs::path pth, std::string usr){
    auto finfo = find_by_name(pth.filename().string());
    if(finfo.is_bad()){
        std::cout << "file not found" << std::endl;
    }

    if(auto fout = getofstream(diskPath)){
        fout->seekp(sizeof(Block) * finfo.headder_block_number + offsetof(Block, asFileStart_owner), std::ios::beg);

        fout->write(reinterpret_cast<char const *>(usr.c_str()), std::min(sizeof(Block::asFileStart_owner), usr.size()));

        fout->close();
    }
}

void FATManager::remove_link(std::string link){
    auto finfo = find_by_name(link);
    if(finfo.is_bad()){
        std::cout << "link not found" << std::endl;
        return;
    }

    unset_block(finfo.headder_block_number);

    // remove FAT entry
    auto fat_blk = getBlockThrowable(diskPath, finfo.FAT_block_number);
    fat_blk.asFAT_filestart[finfo.FAT_block_entry_number] = fat_blk.asFAT_filestart[fat_blk.standard.data_len-1];   // reaplce entry with another in the pile
    fat_blk.standard.data_len--;
    setBlock(diskPath, finfo.FAT_block_number, &fat_blk);
}

FATManager::FATfileinfo FATManager::find_by_name(std::string name){
    FATfileinfo info = {
        .headder_block_number   = 0,    // indicates a bad value as the FAT cannot be a file
        .FAT_block_number       = 0,
        .FAT_block_entry_number = 0,
        .headder_block          = {},
        .FAT_block              = {},
    };

    for(auto fat = begin(getBlockThrowable(diskPath, 0), 0); fat != end(); ++fat){ // for FAT block
        info.FAT_block = *fat;

        for(info.FAT_block_entry_number = 0; info.FAT_block_entry_number < (*fat).standard.data_len; info.FAT_block_entry_number++){               // for file entry

            info.headder_block = getBlockThrowable(diskPath, (*fat).asFAT_filestart[info.FAT_block_entry_number]);  // get file header

            if(std::string(info.headder_block.asFileStart_name) == name){
                info.headder_block_number   = info.FAT_block.asFAT_filestart[info.FAT_block_entry_number];
                return info;
            }
        }        

        info.FAT_block_number = (*fat).standard.next_block;
    }

    return info;
}

DISK_UNIT FATManager::total_num_blocks(){
    if(auto fin = getifstream(diskPath)){
        fin->seekg(0, std::ios::end);
        return fin->tellg() / sizeof(Block);
    }

    return 1;
}

std::vector<DISK_UNIT> FATManager::find_empty_blocks(DISK_UNIT num_blocks){
    
    std::vector<DISK_UNIT> empty_blocks;

    // I LOVE ASSIGNEMNTS WITH ODD DUE DATES. I LOVE WONKING ROUND MY SCHEDULE SO MUCH!!
    // (thers bigger problems if your using this to seriously store files)
    // i apologize for the code you see here
    for(DISK_UNIT i = 1; i < total_num_blocks(); i++)
        empty_blocks.push_back(i);

    for(auto fat = begin(getBlockThrowable(diskPath, 0), 0); fat != end(); ++fat){ // for FAT block
        for(DISK_UNIT file_entry_num = 0; file_entry_num < (*fat).standard.data_len; file_entry_num++){ // for file entry
            std::cout << "\t removing file at block:" << std::to_string((*fat).asFAT_filestart[file_entry_num]);
            Block file_root = getBlockThrowable(diskPath, (*fat).asFAT_filestart[file_entry_num]);      // get file header
            std::cout << "\t , named: " << file_root.asFileStart_name << std::endl;

            //remove block from list
            if(std::find(empty_blocks.begin(), empty_blocks.end(), file_root.standard.next_block) != empty_blocks.end())
                empty_blocks.erase(std::find(empty_blocks.begin(), empty_blocks.end(), file_root.standard.next_block));

            for(auto fb = begin(file_root, (*fat).asFAT_filestart[file_entry_num]); fb != end();){ // for all blocks used by the file
                //remove block from list
                if(std::find(empty_blocks.begin(), empty_blocks.end(), (*fb).standard.next_block) != empty_blocks.end())
                    empty_blocks.erase(std::find(empty_blocks.begin(), empty_blocks.end(), (*fb).standard.next_block));

                ++fb;
            }
        }        
    }

    if(empty_blocks.size() < num_blocks){
        add_blocks(num_blocks - empty_blocks.size());

        DISK_UNIT i = total_num_blocks();
        for(; num_blocks > 0; num_blocks--)
            empty_blocks.push_back(i++);
    }

    return empty_blocks;
}

void FATManager::formatfs(){
    // traverse the linked list of all files, anything not in it gets it data_len set to 0
    // im goning to be stupid about this so heres a massive list of all used cells
    std::vector<DISK_UNIT> used_cells = {0};

    for(auto fat = begin(getBlockThrowable(diskPath, 0), 0); fat != end() && (*fat).standard.data_len != 0; ++fat){
        for(BLOCK_UNIT i = 0; i < (*fat).standard.data_len; i++){
            used_cells.push_back((*fat).asFAT_filestart[i]);
        }
    }

    // set the rest to 0
    if(auto fout = getofstream(diskPath)){
        for(DISK_UNIT i = 0; i < total_num_blocks(); i++){
            if(std::find(used_cells.begin(), used_cells.end(), i) == used_cells.end()){
                fout->seekp(sizeof(Block) * i + offsetof(Block, standard.data_len), std::ios::beg);
                BLOCK_UNIT zero = 0;
                fout->write(reinterpret_cast<char*>(&zero), sizeof(BLOCK_UNIT));
            }
        }

        fout->close();
    }
}

void FATManager::unset_block(DISK_UNIT blk_num){
    if(auto fout = getofstream(diskPath)){
        fout->seekp(sizeof(Block) * blk_num + offsetof(Block, standard), std::ios::beg);
        
        Block::BlockStandard unsetted = {
            .next_block = 0,
            .data_len = 0,
        }; 
        fout->write(reinterpret_cast<char*>(&unsetted), sizeof(unsetted));

        fout->close();
    }
}

void FATManager::add_fat_entry(DISK_UNIT block_num, fs::path diskPath){
    DISK_UNIT current_fat_block_num = 0;
    for(auto fat = begin(getBlockThrowable(diskPath, 0), 0); fat != end(); ++fat){
        std::cout << "cealckneclkn" << std::endl;

        if((*fat).standard.data_len < MAX_FILE_ENTRIES){ // if not full
            // add a entry
            Block former_fat = (*fat);
                former_fat.asFAT_filestart[former_fat.standard.data_len++] = block_num;
            setBlock(diskPath, current_fat_block_num, &former_fat);
            std::cout << "adding entry to not full FAT block" << std::endl;
            break;
        }

        if((*fat).standard.next_block == 0){ // if end of list
            std::cout << "making new FAT block" << std::endl;
            //make a new FAT block to add it to
            DISK_UNIT new_fat_block_num = find_empty_blocks(1)[0];
            {
                Block blk;
                    blk.standard.data_len = 1;
                    blk.asFAT_filestart[0] = block_num;
                setBlock(diskPath, new_fat_block_num, &blk);
            }

            // link new FAT block to FAT list
            {
                Block former_fat = (*fat);
                    former_fat.standard.next_block = new_fat_block_num;
                setBlock(diskPath, current_fat_block_num, &former_fat);
            }

            break;
        }


        current_fat_block_num = (*fat).standard.next_block;
    }
}
