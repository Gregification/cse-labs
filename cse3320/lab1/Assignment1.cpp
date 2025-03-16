/*
    cse3320
    George Boone
    1002055713

    Assignment 1.
*/

#include <iostream>
#include <limits.h>
#include <functional>

#include "Assignment1.hpp"

int main(int argc, char* args[]){
    std::cout << "cse3320 Assignment 1, George Boone" << std::endl;
    std::cout << "\t- operations are not case sensitive, and a space can subsitute \"_\" when typing commands" << std::endl;
    std::cout << "\t- put space after first letter when calling command or else will try to match command name and not arguements:" << std::endl;
    std::cout << "\t- Quit              : exits" << std::endl;
    std::cout << "\t- Display           : displays current page" << std::endl;
    std::cout << "\t- Previous_Page     : displays previous page" << std::endl;
    std::cout << "\t- Next_Page         : displays next page" << std::endl;
    std::cout << "\t- Edit              : open in text editor" << std::endl;
    std::cout << "\t- Run               : runs in current shell" << std::endl;
    std::cout << "\t- Change_Directory  : change directory" << std::endl;

    //current operation
    std::pair<CMD, std::string> cmd;

    //handle arguements
    if(argc > 1){
        cmd = {CMD::Change_Directory, args[1]};
        goto cmdHandler;
    }

    //main loop
    //  theres some copy and pasting of identical code but its still readeable,
    //     trying to use functionals would just get overly messy
    pager.reset();
    while(true){
        //prompt next command from user
        {
            auto optional_cmd = nextCMD();

            if(optional_cmd)
                cmd = optional_cmd.value();
            else 
                continue;
        }

        cmdHandler:
        switch(cmd.first){
            case CMD::Display:
                pager.print(); 
                break;

            case CMD::Quit:
                exit(EXIT_SUCCESS);
                break;

            case CMD::Previous_Page:
                pager.previous();
                pager.print();
                break;

            case CMD::Next_Page:
                pager.next();
                pager.print();
                break;

            case CMD::Edit:{
                if(cmd.second.empty()){
                    puts("\e[0;31mexpected a file");
                    break;
                }

                fs::path pth = prepPath(cmd.second);

                if(fs::exists(pth)){    
                    if(fs::is_directory(pth)){
                        puts("\e[0;31mexpected a file");
                        break;
                    }
                    system(("vi " + pth.string()).c_str());
                } else 
                    puts("\e[0;31mfile does not exist");

            }break;

            case CMD::Run:{
                if(cmd.second.empty()){
                    puts("\e[0;31mexpected a file");
                    break;
                }

                fs::path pth = prepPath(cmd.second);

                if(fs::exists(pth)){    
                    if(fs::is_directory(pth)){
                        puts("\e[0;31mexpected a file");
                        break;
                    }
                    system((pth.string()).c_str());
                } else 
                    puts("\e[0;31mfile does not exist");   
            }break;

            case CMD::Change_Directory: {
                if(cmd.second.empty()){
                    puts("\e[0;31mno directory specified");
                    break;
                }

                fs::path pth = prepPath(cmd.second);

                if(fs::exists(pth)){    
                    if(fs::is_directory(pth))
                        active_directory = pth;
                    else{
                        puts("\e[0;31mnot a directory");
                        puts(cmd.second.c_str());
                        break;
                    }

                    pager.reset();

                    int i = MAX_FILE_COUNT;

                    //credit to refrence A for the for loop condition
                    for(fs::directory_entry e : std::filesystem::directory_iterator(active_directory)){
                        if(i-- <= 0) break;
                    
                        pager.addEle(e);
                    }

                } else {
                    puts("\e[0;31mdirectory does not exist");
                    break;
                }

            } break;

            case CMD::Sort_Pager:{
                std::string in = cmd.second;
                
                if(in.empty()){
                    std::cout << "size or date?(s/d) :";
                    std::getline(std::cin, in);
                }

                if(in.empty() || in[0] == 's')
                    pager.sortBySize();
                else if(in[0] == 'd')
                    pager.sortByDate();
                else
                    std::cout << "\e[0;31minvalid input" << std::endl;

            }break;

            default:
                std::cout << "\e[0;31mu broke it :( " << std::endl;
        }
    }
}

std::optional<std::pair<CMD,std::string>> nextCMD() {
    std::cout << "\e[0;32m" << active_directory << "\e[1;37m>";

    std::string line;
    std::getline(std::cin, line);

    //defaults to [Display]
    if(line.length() == 0)
        return {{COMMAND::Display, line}};

    //find the strongest match

    //current match strength (im just counting matching chracters)
    int cur_match_str = -1;

    //matches with the same strength
    std::vector<CMD> same_strs;

    //check strength of each command
    for(COMMAND command : COMMAND::_values()){
        //match strength of the current command
        int cur_str = -1;

        //looking for consecutive matches
        //  case insensitive matching for all characters
        //  spaces and underscores are interchangeable
        for(int i = 0; i < line.size(); i++){
            const char c = command._to_string()[i];

            constexpr char diff = 'a' - 'A';
            const bool isCharMatch = 
                    c == line[i] 
                ||  ((c >= 'a') ? 
                        (c - diff) 
                    :   (c + diff))
                     == line[i];

            if(isCharMatch 
                    || (c == '_' && line[i] == ' ') 
                    || (c == ' ' && line[i] == '_')
                )
                cur_str++;
            else break;
        }
        
        //if match strength tied, and is not a bad match
        if(cur_str == cur_match_str && cur_str != -1)
            same_strs.push_back(command);
        //if match strength high than anything seen
        else if(cur_str > cur_match_str){
            //set as selected command
            cur_match_str = cur_str;
            //remove other possible matches
            same_strs = {command};
        }
    }
    
    //if no commands match
    if(same_strs.empty()){
        std::cout << "\e[0;31mno matching command" << std::endl;
        return std::nullopt;
    }
    //if exactly 1 matching command
    else if(same_strs.size() == 1){
        //trim command
        line = line.substr(cur_match_str+1);
        
        //return args, remove leading [space]'s if any

        size_t nxt = line.find_first_not_of(" ", 0);

        if(nxt == std::string::npos)
            return {{same_strs[0], line}};

        return {{same_strs[0], line.substr(nxt)}};
    } 
    //mutiple commands matching
    else {
        std::cout << "\e[0;31mambigous command: " << same_strs.size() << " matches" << std::endl;

        //print all matching commands
        for(CMD v : same_strs){
            COMMAND c = v;

            std::cout << "\t" << c._to_string() << std::endl;
        }

        return {{CMD::Display, ""}};
    }
}

void Pager::next()
{
    idx = std::min(idx + MAX_LINES, (int)(eles.size() - MAX_LINES * .5));
}

void Pager::previous()
{  
    idx = std::max(idx - MAX_LINES, 0);
}

void Pager::print()
{
    if(eles.empty())
        std::cout << "\e[1;33m pager is empty" << std::endl;

    //clamp to valid size
    if(idx > eles.size())
        idx = eles.size();
    if(idx < 0)
        idx = 0;

    //print messages
    for(int off = 0; off < MAX_LINES; off++){
        size_t i = idx + off;

        if(i >= eles.size())
            return;

        try{
            if(eles[i].file.is_directory())
                 std::cout << "\e[0;33m";
            else
                std::cout << "\e[0;37m";
        } catch(...){
            std::cout << "\e[0;37m";
        }

        std::string size;
        try {
            size = std::to_string(eles[i].file.file_size());
        }catch(...){
            size = "dir";
        }

        auto perms = eles[i].file.status().permissions();

        std::cout
            << eles[i].id << ". "
            << eles[i].file.path().filename() 
            << "\t" << size
            << "\t" << eles[i].file.last_write_time().time_since_epoch().count()
            << "\t" << ((int)(perms & fs::perms::owner_read) != 0 ? "R" : "")
            << ((int)(perms & fs::perms::owner_write) != 0 ? "W" : "")
            << ((int)(perms & fs::perms::owner_exec) != 0 ? "E" : "")
            << std::endl;
    }
}

void Pager::reset()
{
    eles.clear();
    idx = 0;
}

void Pager::sortBySize()
{
    const static std::function<bool(const Ele&, const Ele&)> cmp = [](const Ele& a, const Ele& b){
        if(a.file.is_directory())
            return false;
        if(b.file.is_directory())
            return true;
        return a.file.file_size() < b.file.file_size();
    };
    std::sort(eles.begin(), eles.end(), cmp);
}

void Pager::sortByDate()
{
    const static std::function<bool(const Ele&, const Ele&)> cmp = [](const Ele& a, const Ele& b){
        return a.file.last_write_time() < b.file.last_write_time();
    };
    std::sort(eles.begin(), eles.end(), cmp);
}

void Pager::addEle(fs::directory_entry f)
{
    Ele e = {f, (int)eles.size()};
    eles.push_back(e);
}

fs::path prepPath(const std::string& str) {
    fs::path pth = str;

    if(pth.is_relative())
        pth = active_directory / pth;
    
    return pth;
}
