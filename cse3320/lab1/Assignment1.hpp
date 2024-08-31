/*
    cse3320-001
    George Boone
    1002055713

    Assignment 1.

    compile with:
    $ g++ Assignment1.cpp --std=c++17

    using a 3rd paty header library called "Better Enums",
        im using it for its reflective enum capabilities
        http://aantron.github.io/better-enums/

    - using vim as the text editor
    
    BONUS
        - showing additional file information (size, last modified, read/execute)
    
    OTHER
        - command matching for complete and partial spellings

    refrenced code sources below
*/
/** All refrenced code
 * the parts of this projects code that refrence these works are marked accordingly

    A. listing files within a directory
        - authored by: "Leo", "Peter Parker"
        - link: https://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c


*/

#include "BetterEnums.hpp"
#include <vector>
#include <string>
#include <optional>
#include <filesystem>
namespace fs = std::filesystem;

#define MAX_FILE_COUNT  1024
#define MAX_FILE_NAME   2048
#define MAX_LINES       5

//basically reflective enums in c++.
//  im just using it to map enums to and from strings
BETTER_ENUM(COMMAND, int,
        Quit,
        Display,
        Edit,
        Run,
        Change_Directory,
        Sort_Pager,
        Previous_Page,
        Next_Page
    );
typedef COMMAND::_enumerated CMD;

/** handles dispalying stuff */
struct Pager{
    struct Ele{
        fs::directory_entry file;
        int id = -1;
    };

    int idx;

    /**to next page*/
    void next();
    /**to previous page*/
    void previous();
    /**print to console*/
    void print();
    /**reset idx and messages*/
    void reset();

    void sortBySize();

    void sortByDate();

    const std::vector<Ele>& getEles() const;
    void addEle(fs::directory_entry);
private:
    std::vector<Ele> eles;
};

//where messages are stored
Pager pager;

//path to the active directory
fs::path active_directory;

int main(int, char*[]);

/**Gets the next command and the trailing arguemtns*/
std::optional<std::pair<CMD, std::string>> nextCMD();

//got rid of the help option, made the ui less intutive
/**loads pager with list of avaliable operations */
void showHelpMsg(const std::string&);

/**prepares a path for use */
fs::path prepPath(const std::string& path);
