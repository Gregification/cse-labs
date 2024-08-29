/*
    cse3320-001
    George Boone
    1002055713

    Assignment 1.

    compile using
    $ g++ Assignment1.cpp --std=c++20

    using a 3rd paty header library called "Better Enums",
        im using it for its reflective enum capabilities
        http://aantron.github.io/better-enums/

    refrenced code sources below
*/
/** All refrenced code
 * the parts of this projects code that refrence these works are marked accordingly

    1. listing files within a directory
        - authored by: "Leo", "Peter Parker"
        - link: https://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c


*/

#include "BetterEnums.hpp"
#include <vector>
#include <string>

#define MAX_FILE_COUNT  1024
#define MAX_FILE_NAME   2048
#define MAX_LINES       5

BETTER_ENUM(COMMAND, int,
		Help,
        Quit,
        Display,
        Edit,
        Run,
        Change_Directory,
        Sort_Directory_listing,
        Move_to_Directory,
        Remove_File,
        Previous,
        Next
    );
typedef COMMAND::_enumerated CMD;

/** handles dispalying messages */
struct Pager{
    std::vector<std::string> messages;
    int idx;

    /**to next page*/
    void next();
    /**to previous page*/
    void previous();
    /**print to console*/
    void print();
    /**reset idx and messages*/
    void reset();
};

//where messages are stored
Pager pager;

//full path to the active directory
std::string active_directory;

int main(int, char*[]);

/**Gets the next command and the trailing arguemtns*/
std::optional<std::pair<CMD, std::string>> nextCMD();

/**loads pager with list of avaliable operations */
void showHelpMsg(const std::string&);
