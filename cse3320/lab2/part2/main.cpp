/**
 * cse3320 Assignment 2 Part 2
 * George Boone
 * 1002055713
 * 
 * - tested on WSL Ubuntu 22.04.5
 * - timing results are on the last line of output
 * - see macros on line 34
 * - min threads spawned : 1
 * - max threads spawned : '# of entries`
 * - didnt bother with mutexes, each thread just gets its own index range to work in.
 *      - the thread::join() function is used though.
 * 
 * compile with:
 * $ g++ main.cpp --std=c++17
 * 
 * run with:
 * # where "N" is the number of threads.
 * $ ./a.out N < data.csv > sorted.csv
 * 
 * process:
 *  1. read all data from stdin
 *  2. evenly split the data into N continious partitions
 *  3. sort each partition. one thread per partition.
 *  4. wait for all threads to finish
 *  5. merge the sorted partitions
 *  6. write sorted data to stdout
 * 
 * external resources:
 *   - "void move(...)" function to inplace reindex elements of a vector
 *          - neat way of doing it.
 *          - code snippet from  https://stackoverflow.com/questions/45447361/how-to-move-certain-elements-of-stdvector-to-a-new-index-within-the-vector
 *  
 */
// #define DEBUG
#define SHOW_TIMING_RESULTS

#include <functional>
#include <vector>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include "Entry.h"
#include <cmath>        // std::ceil

/**
 * reads data from stdin and stores it in _entries.
 *  - data format, CSV : "[char x26],[long]"
 *  - assumed to be valid data
 *  - skips bad lines
 */
void readData(std::vector<ENTRY>& _entries);

// neat way to move an element around within a vector
// from : https://stackoverflow.com/questions/45447361/how-to-move-certain-elements-of-stdvector-to-a-new-index-within-the-vector
template <typename t>
void move(std::vector<t>& v, size_t oldIndex, size_t newIndex) {
    if (oldIndex > newIndex)
        std::rotate(v.rend() - oldIndex - 1, v.rend() - oldIndex, v.rend() - newIndex);
    else        
        std::rotate(v.begin() + oldIndex, v.begin() + oldIndex + 1, v.begin() + newIndex + 1);
}

int main(int argc, char* args[]) {
    //--------------------------------------------------------------------------------------
    // init
    //--------------------------------------------------------------------------------------

    // user inupt data

    int targ_n_threads = 0;

    // internal data

    std::vector<ENTRY> entries;                         // data to be sorted

    typedef struct {
        size_t start, end;
    } Range;
    std::vector<std::pair<std::thread, Range>> threads; // threads & their ranges

    
    //--------------------------------------------------------------------------------------
    // parsing inputs
    //  - same code as used in part 1
    //--------------------------------------------------------------------------------------
    
    // handle cli args

    if (argc != 2) {
        std::cerr << "usage: " << args[0] << " N" << std::endl;
        std::cerr << "    N: target number of threads" << std::endl;
        exit(1);
    } else {
        targ_n_threads = std::stoul(args[1]);
        if (targ_n_threads < 1)
            targ_n_threads = 1;
        
        #ifdef DEBUG
        std::cerr << "target number of threads: " << targ_n_threads << std::endl;
        #endif
    }

    // read data

    #ifdef DEBUG
    std::cerr << "reading data" << std::endl;
    #endif
    readData(entries);

    
    //--------------------------------------------------------------------------------------
    // sorting
    //--------------------------------------------------------------------------------------
    #ifdef SHOW_TIMING_RESULTS
    long long start_micros = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    #endif

    {
        size_t step = std::ceil(entries.size() / targ_n_threads);
        if(step < 1) // incase the code before screws up
            step = 1;
        
        for (size_t i = 0; i < entries.size(); i += step) {
            Range r = {
                .start  = i,
                .end    = std::min(i + step, entries.size())
            };

            #ifdef DEBUG
            std::cerr << "spawning thread  " << threads.size() << " for range [" << r.start << ", " << r.end << ")" << std::endl;
            #endif

            threads.push_back({
                std::thread([&entries, r](){
                    // insertion sort the range
                    for(size_t i = r.start; i < r.end - 1; i++){
                        // find smallest, move to front
                        
                        size_t tj = i;
                        for(size_t j = i + 1; j < r.end; j++){
                            if(entries[j].id < entries[tj].id){
                                tj = j;
                            }
                        }

                        move(entries, tj, i);
                    }
                }),
                r
            });
        }
    }

    //--------------------------------------------------------------------------------------
    // merging
    //--------------------------------------------------------------------------------------    
    #ifdef DEBUG
    std::cerr << "waiting for thread " << 0 << "... " << std::endl;
    #endif
    threads[0].first.join();

    // merge the sorted partitions
    for(size_t i = 1; i < threads.size(); i++){
        #ifdef DEBUG
        std::cerr << "waiting for thread " << i << "... ";
        #endif
        threads[i].first.join();

        Range& r = threads[i].second;
        
        #ifdef DEBUG
        std::cerr << " merging results. "<< " range [" << r.start << ", " << r.end << ")" << std::endl;
        #endif
        
        //inplace insertion sort
        size_t 
            li = 0,
            ri = r.start;

        while(li < ri && ri < r.end){
            if(entries[ri].id < entries[li].id){
                move(entries, ri, li);
                ri++;
            } else {
                li++;
            }
        }
    }

    //--------------------------------------------------------------------------------------
    // output
    //--------------------------------------------------------------------------------------
    
    #ifdef DEBUG
    std::cerr << "outputting results" << std::endl;
    #endif
    for(size_t i = 0; i < entries.size(); i++){
        std::cout << entries[i].id << "," << entries[i].genome << std::endl;
    }

     // timing results

    #ifdef SHOW_TIMING_RESULTS
    {
        long long end_micros = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        end_micros -= start_micros;
        std::cout << "time elapsed (ms): " << end_micros / 1000;
        std::cout << "." << end_micros % 1000 << std::endl;
    }
    #endif
}

void readData(std::vector<ENTRY>& _entries){
    std::string line;

    while(std::getline(std::cin, line)) {

        int delim = line.find(',');
        if(delim == -1)
            continue;

        try{
            ENTRY e;

            line.copy(e.genome, delim, 0);
            e.genome[delim] = '\0';

            e.id = std::stol(line.substr(delim + 1));

            _entries.push_back(e);
        }
        catch(...){
            continue;
        }

    }
}