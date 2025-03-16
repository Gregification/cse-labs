/**
 * cse3320 Assignment 3
 * George Boone
 * 1002055713
 * 
 * build
        $ CMake -S [dir with CMakeLists.txt] -B [build output dir]
        $ CMake --build [build output dir]
        $ ./Assigment3
    alternatively use g++ and tag all the ".cpp" files
 */

#include <iostream>
#include "FAT.hpp"

int main(){
    std::cout << "cse3320 fall2024 assignment-3" << std::endl;

    FATManager fatm;

    fatm.open_fs("default.assignemnt3");
    // for(int j = 0; j < 4; j++){
    //     std::cout << j << std::endl;
    //     for(auto i : fatm.find_empty_blocks(j+1)) 
    //         std::cout << "\t" << i << std::endl;
    // }

    int cmd_num = 0;
    goto run_cmd;

    while(true){
        std::cout << "> ";
        
        {
            cmd_num = 0;
            std::string cmd;
            std::getline(std::cin, cmd);

            if(cmd == "q")
                exit(0);
            else if(cmd == "i") {
                std::cout << "'disk' file:          " << fatm.currentfs().string()                                          << std::endl;
                std::cout << "num blocks:           " << fatm.total_num_blocks()                                            << std::endl;
                std::cout << "\tblock total size:   " << std::to_string(sizeof(Block))                                      << std::endl;
                std::cout << "\tblock overhead:     " << std::to_string(sizeof(Block::standard))                            << std::endl;
                std::cout << "\tblock data size:    " << std::to_string(sizeof(Block::data_start))                          << std::endl;
                std::cout << "\tblock efficiency:   " << std::to_string((float)sizeof(Block::data_start) / sizeof(Block))   << std::endl;
                continue;
            }

            try{
                cmd_num = std::stoi(cmd);
            } catch(...){}
        }

        run_cmd:;
        switch(cmd_num){
            default:
                std::cout << "unknown command" << std::endl;
                break;

            case 0:
                std::cout << "q quit" << std::endl;
                std::cout << "i info" << std::endl;
                std::cout << "0 help" << std::endl;
                std::cout << "1 createfs" << std::endl;
                std::cout << "2 formatfs" << std::endl;
                std::cout << "3 openfs" << std::endl;
                std::cout << "4 list" << std::endl;
                std::cout << "5 remove" << std::endl;
                std::cout << "6 rename" << std::endl;
                std::cout << "7 put" << std::endl;
                std::cout << "8 get" << std::endl;
                std::cout << "9 user" << std::endl;
                std::cout << "10 link" << std::endl;
                std::cout << "11 unlink" << std::endl;
                break;

            case 1: {
                std::cout << "1 createfs" << std::endl;
                std::cout << "name: ";
                std::string cmd;
                std::getline(std::cin, cmd);
                fatm.open_fs({cmd});
                fatm.add_blocks(50);
            } break;

            case 2: {
                std::cout << "2 formatfs" << std::endl;
            }break;

            case 3: {
                std::cout << "3 openfs" << std::endl;
                std::cout << "name: ";
                std::string cmd;
                std::getline(std::cin, cmd);
                fatm.open_fs({cmd});
            } break;

            case 4: {
                std::cout << "4 list" << std::endl;
                fatm.list_files(0, 50);
            }break;

            case 5:{
                std::cout << "5 remove" << std::endl;
                std::cout << "local file: ";
                std::string cmd;
                std::getline(std::cin, cmd);
                fatm.remove(cmd);
            }break;

            case 6:{
                std::cout << "6 rename" << std::endl;
                std::string old, _new;
                std::cout << "local file target: ";
                std::getline(std::cin, old);
                std::cout << "new name: ";
                std::getline(std::cin, _new);
                fatm.rename(old, _new);
            }break;

            case 7:{
                std::cout << "7 put" << std::endl;
                std::cout << "host file: ";
                std::string cmd;
                std::getline(std::cin, cmd);
                fatm.add_from_host_fs({cmd});
                std::cout << "b" << std::endl;
            }break;

            case 8:{
                std::cout << "8 get" << std::endl;
                std::cout << "local file: ";
                std::string cmd;
                std::getline(std::cin, cmd);
                fatm.copy_to_host_fs({cmd});
            }break;

            case 9:{
                std::cout << "9 user" << std::endl;
                std::string file, name;
                std::cout << "local file: ";
                std::getline(std::cin, file);
                std::cout << "name: ";
                std::getline(std::cin, name);
                fatm.set_user({file}, {name});
            }break;

            case 10:{
                std::cout << "10 link" << std::endl;
            }break;

            case 11:{
                std::cout << "11 unlink" << std::endl;
            }break;                
        }
    }
}
