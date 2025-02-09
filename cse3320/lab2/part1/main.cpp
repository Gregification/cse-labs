/**
 * cse3320 Assignment 2 Part 1
 * George Boone
 * 1002055713
 * 
 * - takes 1 cli argument: number of children processes, default 1
 * - tested on WSL Ubuntu 22.04.5
 * - time takes is on the last line outputted
 * - see macros on line 42
 * 
 * compile with:
 * $ g++ main.cpp --std=c++17
 * 
 * run with:
 * # WHERE "N" IS THE NUMBER OF CHILD PROCESSES
 * $ ./a.out N < data.csv > sorted.csv
 * 
 * process:
 *  1. init child processes. (all other child processes route output into child process 0)
 *  2. read all data from stdin
 *  3. round robin distribute data to child process
 *  4. wait for child process 0 to finish
 *  5. print sorted data to stdout
 * 
 * refrences:
 *  - UTA Professor Hiu Lu's cse3320 fall-2024 notes & resources
 *      - https://www.cs.binghamton.edu/~huilu/teaching/lecture5/examples/
*/

// spaghettie code. I avoided singals since Im not too familiar with it and its another 
// thing the class hasent gone over. instead used a deticated 'bad' value a end of the pipe.

// side note: 
// Whats the point of the class if we have to self teach entirely from 3rd party resources
// to learn anything. Even with how simple this assignment is we literally did not cover 
// anything remotely relevant to it. Hiu Lu's leacture slides are a great example of what to
// actually cover. Big gripe with the pacing and how the course is so non technical, even if 
// this was intended to be a blow off class we should at least learn something; from how this 
// class has gone so for far, it should be a 1100 level course. Ive see what this course
// could be and its sad to see what it is. 

// #define DEBUG
#define SHOW_TIMEING_RESULTS

#include <vector>       // std::vector
#include <string>       // std::string
#include <array>        // std::array
#include <iostream>     // std::cin, std::cout, std::cerr
#include <unistd.h>     // pipe, fork, pid_t
#include <csignal>      // signal
#include <sys/wait.h>   // waitpid
#include <chrono>       // std::chrono

#include "Entry.h"

int main(int argc, char* args[]) {
    std::vector<std::array<int, 2>> child_pfds;
    int main_pfds[2];       //useed for final steps to stdout

    if(pipe(main_pfds) == -1) {
        std::cerr << "error creating pipe" << std::endl;
        exit(1);
    }

    //------------------------------------------------------------------------------------
    // parse args
    //------------------------------------------------------------------------------------
    // 0: number of child processes

    // 0
    {
        unsigned int n_children = argc < 2 ? 1 : std::stoul(args[1]);

        if(n_children < 1){
            n_children = 1;
        }

        child_pfds.resize(n_children);
    }

    #ifdef DEBUG
    std::cout << "number of child processes: " << child_pfds.size() << std::endl;
    #endif

    //------------------------------------------------------------------------------------
    // pipe & child processes initialization
    //------------------------------------------------------------------------------------

    #ifdef DEBUG
    std::cout << "pipe & child processes initialization. " << std::endl;
    #endif

    for(int i = 0; i < child_pfds.size(); i++) {
        int pfds[2]; // pipe file descriptors
        pid_t pid;
        // create pipe
        if(pipe(pfds) == -1) {
            std::cerr << "error creating pipe" << std::endl;
            exit(1);    
        }

        // fork
        if((pid = fork()) < 0) {
            std::cerr << "error during fork" << std::endl;
            exit(1);
        }

        
        child_pfds[i][0] = pfds[0];
        child_pfds[i][1] = pfds[1];
        

        if(pid == 0) {
            // sorted collection
            std::vector<ENTRY> data;
            // read until receive bad data
            
            for(ENTRY e = {.id = 1}; e.id >= 0 && read(pfds[0], &e, sizeof(e))>= 0;) {

                #ifdef DEBUG
                std::cerr << "child " << i << " received " << e.id << std::endl;
                #endif

                if(e.id < 0) {
                    break;
                }

                // sorting logic, insertion
                for(int j = 0; j < data.size(); j++) {
                    if(data[j].id > e.id) {
                        data.insert(data.begin() + j, e);
                        goto cont;
                    }
                }

                data.push_back(e);

                cont:{}
            }

            #ifdef DEBUG
            std::cerr << "child " << i << " total data size: " << data.size() << std::endl;
            #endif

            // if is first child
            if(i == 0) {
                //route the output to the main stdout
                close(pfds[1]);               //close normal stdout
                dup2(main_pfds[1], pfds[1]);  //redirect to main stdout
            } else {
                close(pfds[1]);               //close normal stdin
                dup2(child_pfds[0][1], pfds[1]);  //redirect to first childs stdin
            }
            // output sorted entries
            for(ENTRY e : data) {
                //std::cerr << "child " << i << " sending " << e.genome << " " << e.id << std::endl;
                if(write(pfds[1], &e, sizeof(e)) <= 0) {
                    std::cerr << "child" << i << "error during write" << __LINE__<< std::endl;
                    exit(1);
                }
            }

            if(i == 0){
                //send bad data
                ENTRY e = {.id = -1};
                if(write(pfds[1], &e, sizeof(e)) <= 0) {
                    std::cerr << "child" << i << "error bad data during write" << __LINE__<< std::endl;
                    exit(1);
                }
            }

            exit(0);
        }
    }

    // only parent continues

    //------------------------------------------------------------------------------------
    // read data 
    //    - read from stdin
    //    - assume CSV as such : "[string],[long]"
    //------------------------------------------------------------------------------------
    #ifdef DEBUG
    std::cout << "read data." <<std::endl;
    #endif

    std::vector<ENTRY> data;
    data.reserve(10e3);

    for(std::string line; std::getline(std::cin, line);) {
        
        int delim = line.find(',');
        if(delim == -1) {
            #ifdef DEBUG
            std::cerr << "error skipping bad line: " << line << std::endl;
            #endif
            continue;
        }

        ENTRY e;
        // store string
        line.copy(e.genome, delim, 0);
        e.genome[delim] = '\0';

        // parse number
        e.id = std::stol(line.substr(delim + 1));


        // #ifdef DEBUG
        // static int oaecnoc = 0;
        // oaecnoc++;
        // if(oaecnoc < 5)
        // #endif

        data.push_back(e);
    }
    

    //------------------------------------------------------------------------------------
    // split data set among child processes
    //      - timer starts
    //------------------------------------------------------------------------------------
    #ifdef DEBUG
    std::cout << "split data set among child processes, num entries: " << data.size() << std::endl;
    #endif

    #ifdef SHOW_TIMEING_RESULTS
    long long start_micros = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    #endif

    for(int i = 0; i < data.size(); i++) {
        if(write(
            child_pfds[i % child_pfds.size()][1],
            &data[i],
            sizeof(data[i])
        ) <= 0) {
            std::cerr << "error during write" << std::endl;
            exit(1);
        }
    }

    #ifdef DEBUG
    std::cout << "close write end of pipes." <<std::endl;
    #endif

    // close write end of pipes
    for(int i = 1; i < child_pfds.size(); i++) {
        
        //send bad data to kill everything but the first child processes
        #ifdef DEBUG
        std::cerr << "bad data sent to child " << i << std::endl;
        #endif

        ENTRY e{.id = -2};
        if(write(
            child_pfds[i][1],
            &e,
            sizeof(data[i])
        ) <= 0) {
            std::cerr << "error during write" << std::endl;
            exit(1);
        }
        //close(child_pfds[i][1]);
        #ifdef DEBUG
        sleep(1);
        #endif
    }
    
    #ifdef DEBUG
    std::cout << "wait for sub child processes to finish." <<std::endl;
    #endif

    // wait for child processes to finish
    for(int i = 1; i < child_pfds.size(); i++) {
        #ifdef DEBUG
        std::cout << "waiting on child process " << i << "." << std::endl;
        #endif

        wait(NULL);

        #ifdef DEBUG
        std::cout << "child a process finished " << i << "." << std::endl;
        sleep(1);
        #endif
    }
    
    #ifdef DEBUG
    std::cout << "make first child finish." <<std::endl;
    #endif

    {
        ENTRY e{.id = -1};
        if(write(
            child_pfds[0][1],
            &e,
            sizeof(data[0])
        ) <= 0) {
            std::cerr << "error during write" << std::endl;
            exit(1);
        }
        
        #ifdef DEBUG
        sleep(1);
        #endif
    }


    #ifdef DEBUG
    std::cout << "data from first child." <<std::endl;
    #endif

    for(ENTRY e;  read(main_pfds[0], &e, sizeof(e)) > 0;) {
        if(e.id == -1)
            break;

        std::cout << e.genome << "," << e.id << std::endl;        
    }

    //------------------------------------------------------------------------------------
    // timer ends
    //------------------------------------------------------------------------------------
    #ifdef SHOW_TIMEING_RESULTS
    long long end_micros = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    end_micros -= start_micros;
    std::cout << "time elapsed (ms): " << end_micros / 1000;
    std::cout << "." << end_micros % 1000 << std::endl;
    #endif

    #ifdef DEBUG
    std::cout << "wait for first child processes to finish." <<std::endl;
    #endif
    wait(NULL);

    return 0;
}
