/*
    cse3320
    George Boone
    1002055713

    Assignment 1.
*/

#include <iostream>
#include <limits.h>
#include <format>
#include <filesystem>

#include "Assignment1.hpp"

int main(int argc, char* args[]){
    std::cout << "cse3320 Assignment 1, George Boone" << std::endl;
    std::cout << "[H] to page operations. [D] to display page. [P][N] for page navigation" << std::endl;

    //current operation
    std::pair<CMD, std::string> cmd;

    //handle arguements
    if(argc != 0){
        cmd = {CMD::Change_Directory, *args};
        goto cmdHandler;
    }

    while(true){
        {
            auto optional_cmd = nextCMD();

            if(optional_cmd)
                cmd = optional_cmd.value();
            else {
                std::cout << "meow" << std::endl;
                continue;
            }
        }

        cmdHandler:
        switch(cmd.first){
            case CMD::Help:
                showHelpMsg(cmd.second);
                break;

            case CMD::Display:
                pager.print(); 
                break;

            case CMD::Quit:
                exit(EXIT_SUCCESS);
                break;

            case CMD::Previous:
                pager.previous();
                pager.print();
                break;

            case CMD::Next:
                pager.next();
                pager.print();
                break;

            case CMD::Edit:
                if(cmd.second.empty())
                    showHelpMsg(cmd.second);
                else 
                    system(("open " + cmd.second).c_str());
                break;

            case CMD::Run:
                system((
                        "cd " + active_directory + ";"
                        + cmd.second
                    ).c_str());
                break;

            case CMD::Change_Directory: {
                if(std::filesystem::exists(""));
            } break;

            case CMD::Sort_Directory_listing: {
                //this just sorts what evers in messages, dosent acually know if its a file

            } break;

            case CMD::Move_to_Directory: {

            } break;

            case CMD::Remove_File: {

            } break;

            default:
                std::cout << "u broke it :( " << std::endl;
        }
    }
}

std::optional<std::pair<CMD,std::string>> nextCMD() {
    std::cout << active_directory << ">";

    std::string line;
    std::getline(std::cin, line);

    //defaults to [Help]
    if(line.length() == 0)
        return {{COMMAND::Help, line}};

    //find the strongest match

    //current match strength (im just counting matching chracters)
    int cur_match_str = -1;
    //matches with the same strength
    std::vector<CMD> same_strs;

    for(COMMAND command : COMMAND::_values()){
        char c = command._to_string()[0];
        
        //match strength of the current command
        int cur_str = -1;

        //capital and lowercase matching for all characters
        for(int i = 0; i < line.size(); i++){
            //if matches letter (capital or lower case)
            constexpr char diff = 'a' - 'A';
            bool isCharMatch = 
                    c == line[i] 
                ||  ((c >= 'a') ? 
                        (c - diff) 
                    :   (c + diff))
                     == line[i];

            if(isCharMatch)
                cur_str++;
            else break;
        }

        if(cur_str == cur_match_str)
            same_strs.push_back(command);
        else if(cur_str > cur_match_str){
            cur_match_str = cur_str;
            same_strs = {command};
        }
    }
    std::cout << "match strength : " << cur_match_str << std::endl;
    //if no commands match
    if(same_strs.empty()){
        return std::nullopt;
    }
    //if exactly 1 matching command
    else if(same_strs.size() == 1){
        //trim leading white space from arguements
        size_t nxt = line.find_first_not_of(" ", 1);
        
        if(nxt == std::string::npos)
            return {{same_strs[0], line}};

        return {{same_strs[0], line.substr(nxt)}};
    } 
    //mutiple commands matching
    else {
        pager.reset();
        for(CMD v : same_strs){
            COMMAND c = v;

            pager.messages.push_back(
                std::format("{:d}. {:c} : {:s}", 
                    c._to_index(),
                    c._to_string()[0],
                    c._to_string()
                )
            );
        }
        std::cout << "ambigous command: " << same_strs.size() << " matches" << std::endl;
        return {{CMD::Display, ""}};
    }
}

void showHelpMsg(const std::string& msg) {
    if(!msg.empty())
        std::cout << "Unknown Operation: \"" << msg << "\"" << std::endl;

    pager.reset();

    for(int i = 0; i < COMMAND::_size(); i++){
        COMMAND c = COMMAND::_from_index(i);

        pager.messages.push_back(
            std::format("{:d}: {:s}", 
                i,
                c._to_string()
            )
        );
    }
}

void Pager::next()
{
    if(idx + MAX_LINES < messages.size())
        idx += MAX_LINES;
}

void Pager::previous()
{   
    idx -= MAX_LINES;
    idx = std::max(0, idx);
}

void Pager::print()
{
    if(messages.empty())
        std::cout << "pager is empty" << std::endl;

    if(idx < 0)
        idx = 0;

    for(int off = 0; off < MAX_LINES; off++){
        size_t i = idx + off;

        if(i >= messages.size())
            return;

        std::cout << messages[i] << std::endl;
    }
}

void Pager::reset()
{
    messages.clear();
    idx = 0;
}
