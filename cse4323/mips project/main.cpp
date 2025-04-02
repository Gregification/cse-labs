/*
    cse4323 spring 2025
    mips project

    George Boone
    1002055713

    take a file as input arguement.
        $ g++ -std=c++20 -o main main.cpp
        $ ./main ./input.txt
*/

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <array>
#include <filesystem>

using namespace std;

#define UNIT unsigned long

// #define DEBUG

class MIPSProgram {
public:
    enum InstrucitonType{
        ADD,
        SUB,
        LW,
        SW,
        BEQ,
        BGT
    };

    struct Instruction {
        InstrucitonType type;
        UNIT args[3];
    };

    vector<Instruction> instrs;
    array<UNIT, 8> registers = {0, 0, 6, 4, 0, 0, 0, 0};
    map<UNIT, UNIT> memory;     // address -> value
    
    map<string, UNIT> labels;   // name -> location

    void parse(string filename){
        ifstream file(filename);
        
        string str;
        UNIT index = 0;

        // sotres unresolved labels form their pc. this could be better done with pointers
        map<string, vector<UNIT>> unresolvedLabels;

        #ifdef DEBUG
        cout << "parsing: " << filename << endl;
        #endif

        while(getline(file, str)){
            if(str[0] == '#' || str.empty()) continue;
            
            bool islabel = false;
            for(auto c : str){
                if(c == ':') islabel = true;
            }
            
            #ifdef DEBUG
            cout << "parsing " << (islabel ? "label" : "instruction") <<": " << str << endl;
            #endif

            if(islabel){
                string s = str.substr(0, str.find(":"));
                labels[s] = index;

                #ifdef DEBUG
                cout << "\tlabel: \"" << s << "\"" << endl;
                cout << "\tindex: " << index << endl;
                #endif
            }
            else 
            {
                string s = str.substr(0, str.find(" "));
                
                // from https://stackoverflow.com/questions/313970/how-to-convert-an-instance-of-stdstring-to-lower-case
                std::transform(s.begin(), s.end(), s.begin(),
                    [](unsigned char c){ return tolower(c); });

                Instruction istr;
                istr.args[2] = 0;
                
                if(s == "add")          istr.type = ADD;
                else if(s == "sub")     istr.type = SUB;
                else if(s == "lw")      istr.type = LW;
                else if(s == "sw")      istr.type = SW;
                else if(s == "beq")     istr.type = BEQ;
                else if(s == "bgt")     istr.type = BGT;
                else {
                    cout << "\tinvalid instruction: \"" << str << "\"" << endl;
                    continue;
                }

                #ifdef DEBUG
                cout << "\tinstr: " << istr.type << endl;
                #endif

                // tokenize by spaces
                vector<string> tokens;
                {
                    std::istringstream iss(str);
                    while (getline(iss, s, ' '))
                        tokens.push_back(s);
                }

                auto asRegister = [](string s)->UNIT{
                    
                    UNIT result = 0;
                    
                    if(s[0] == '$' && s[1] == 't') 
                    result = stoi(s.substr(2, 1));
                    else {
                        #ifdef DEBUG
                        cout << "invalid register: " << s << endl;
                        #endif
                    }
                    
                    #ifdef DEBUG
                    cout << "\targ parse as register: " << s << " -> " << result << endl;
                    #endif

                    return result;
                };
                auto asImmediate = [](string s)->UNIT{
                    UNIT result = stoi(s);

                    #ifdef DEBUG
                    cout << "\targ parse as immediate: " << s << " -> " << result << endl;
                    #endif

                    return result;
                };
                auto asLabel = [&](string s, UNIT* result, UNIT pc){                    
                    if(labels.contains(s)){
                        *result = labels[s];
                        cout << "\targ parse as label: " << s << " -> " << *result << endl;
                    } else {
                        
                        #ifdef DEBUG
                        cout << "\tunknown label: \"" << s << "\"" << endl;
                        #endif

                        unresolvedLabels[s].push_back(pc);
                    }

                    return result;
                };

                switch(istr.type){
                    case SUB: 
                    case ADD: 
                        istr.args[0] = asRegister(tokens[1]);
                        istr.args[1] = asRegister(tokens[2]);
                        istr.args[2] = asRegister(tokens[3]);
                        break;
                    case LW: 
                    case SW: 
                        istr.args[0] = asRegister(tokens[1]);
                        istr.args[1] = asImmediate(tokens[2]);
                        break;
                    case BEQ: 
                    case BGT: 
                        istr.args[0] = asRegister(tokens[1]);
                        istr.args[1] = asRegister(tokens[2]);
                        asLabel(tokens[3], istr.args + 2, index);
                        break;
                    default:
                        cout << "invalid instruction: " << str << endl;
                        return;
                }

                instrs.push_back(istr);

                #ifdef DEBUG
                cout << "\tindex: " << index << endl;
                #endif

                index++;
            }
        }
        file.close();

        // tie lables to their values
        for(auto it = unresolvedLabels.begin(); it != unresolvedLabels.end(); it++){
            string s = it->first;
            s.erase(
                std::remove_if(
                    s.begin(),
                    s.end(),
                    [](char c){
                        return !isalnum(c);
                    }),
                s.end()
            );
            
            if(labels.contains(s)){
                #ifdef DEBUG
                cout << "resolved label: \"" << s << "\" to " << labels[s] << " , #times: " << it->second.size() << endl;
                #endif
                for(auto v :  it->second){
                    instrs[v].args[2] = labels[s];
                }
                cout << endl;
            } else {
                cout << "unresolved label: \"" << s  << "\"" << endl;
            }
        }
    }

    void run(){
        UNIT pc = 0;
        while(pc < instrs.size()){

            #ifdef DEBUG
            // slap together print
            cout << "pc: " << pc << " | " << instrs[pc].type << " " << instrs[pc].args[0] << " " << instrs[pc].args[1] << " ";
            if(instrs[pc].type != SW && instrs[pc].type != LW)
                cout << instrs[pc].args[2] << "\t\t";
            else cout << "\t";
            cout << "{";
            for(int i = 0; i < registers.size(); i++) 
                cout << "\t'$t" << i << "': " << registers[i] << ", ";
            cout << "}" << endl;
            #endif

            // preform instruction
            switch(instrs[pc].type){
                case ADD:
                    registers[instrs[pc].args[0]] = registers[instrs[pc].args[1]] + registers[instrs[pc].args[2]];
                    pc++;
                    break;
                case SUB:
                    registers[instrs[pc].args[0]] = registers[instrs[pc].args[1]] - registers[instrs[pc].args[2]];
                    pc++;
                    break;
                case LW:
                    registers[instrs[pc].args[0]] = instrs[pc].args[1];
                    pc++;
                    break;
                case SW:
                    memory[instrs[pc].args[1]] = registers[instrs[pc].args[0]];
                    pc++;
                    break;
                case BEQ:
                    if(registers[instrs[pc].args[0]] == registers[instrs[pc].args[1]])
                        pc = instrs[pc].args[2];
                    else pc++;
                    break;
                case BGT:
                    if(registers[instrs[pc].args[0]] > registers[instrs[pc].args[1]])
                        pc = instrs[pc].args[2];
                    else pc++;
                    break;
                default:
                    cout << "invalid instruction: " << instrs[pc].type << endl;
                    return;
            }
        }
    }
};

int main(int argc, char *argv[]) {
    if(argc < 2){
        std::cout << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }
    
    // string file = "d:\cse-labs\cse4323\mips project\input2.txt";
    string file = argv[1];

    cout << "file: " << file << endl;
    if(!filesystem::exists(file)){
        cout << "file does not exist" << endl;
        return 0;
    }

    MIPSProgram prog;
    // prog.parse(argv[1]);
    prog.parse(file);
    prog.run();

    cout << endl;
    cout << endl;
    cout << endl;


    cout << "Final Register State: {";
    for(int i = 0; i < prog.registers.size(); i++) 
        cout << "'$t" << i << "': " << prog.registers[i] << ", ";
    cout << "}" << endl;

    cout << "Final Memory State: {";
    for(auto it = prog.memory.begin(); it != prog.memory.end(); it++) 
        cout << it->first << ": " << it->second << ", ";
    cout << "}" << endl;

    return 0;
};
