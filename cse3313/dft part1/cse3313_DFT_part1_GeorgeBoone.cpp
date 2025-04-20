/**
 * cse3313 spring2025 DFT part 1
 * 
 * 1002055713 
 * George Boone
 * 
 * build :              $ g++ -std=c++20 cse3313_DFT_part1_GeorgeBoone.cpp -o dft1.out
 * run w/ streaming:    $ ./dft1.out <dft/idft> <n-points> < file.csv
 * example 64 point dft: $ ./dft1.out dft 64 < file.csv
 * example 55 point idft: $ ./dft1.out idft 55 < file.csv
 * 
 * DFT and IDFT done using the summation equation
 */

#include <iostream>
#include <vector>
#include <complex>
#include <sstream>

// e^(ix) = cos(x) + i*sin(x)
// dft : x[k] = x * Wn^(nk) where Wn^k = e^(-j2<pi>/N * nk)

using namespace std;

typedef complex<long double> UNIT;

vector<UNIT> parseInput(istream &input);
vector<UNIT> dft(vector<UNIT> sequence, size_t n);
vector<UNIT> idft(vector<UNIT> sequence, size_t n);

int main(int argc, char *argv[]) {
    if (argc != 3 || (string(argv[1]) != "dft" && string(argv[1]) != "idft") || !stoul(argv[2])) {
        cerr << "Usage: " << argv[0]  << " <dft/idft> <n-points>" << endl;
        cerr << "\tprogram must know to preform dft or idft and for how many points. pass it both arguements" << endl;
        return 1;
    }

    bool isDFT = string(argv[1]) == "dft";
    size_t n = stoul(argv[2]);

    vector<UNIT> seq = parseInput(cin);

    if(isDFT)
        seq = dft(seq, n);
    else
        seq = idft(seq, n);

    for (int i = 0; i < seq.size(); i++) {
        cout << seq[i] << endl;
    }
    return 0;
}

vector<UNIT> parseInput(istream &is){
    vector<UNIT> arr;

    // parse complex input to vector
    // example input : 0.98,-0.89655-0.47921i,0.55137+0.82518i,-0.097637-0.99133i,
    for(string str; getline(is, str, ',');) {
        UNIT temp;

        size_t numend;
        temp.real(stold(str, &numend));
        try{
            temp.imag(stold(str.substr(numend)));
        } catch(const exception&){ }

        arr.push_back(temp);
    }
    return arr;
}

vector<UNIT> dft(vector<UNIT> s, size_t n){
    // f[k] =   sum<j=0 to n-1>(f[j] * e^(-i2<pi>jk/n))                              // base equation
    //      =>  sum<j=0 to n-1>(f[j] * (cos(-2<pi>jk/n) + i * sin(-2<pi>jk/n)) )    // expanded for computation
    // f[k] : frequency content
    // s[k] : sample sequence
    // expanded using : e^(ix) = cos(x) + i*sin(x)
    
    vector<UNIT> f(n); // f[k]

    s.resize(n);// handles zero padding

    for (int k = 0; k < n; k++) {
        for (int j = 0; j < n; j++) {
            f[k] += s[j] * UNIT(cos(-2 * M_PI * j * k / n), sin(-2 * M_PI * j * k / n));
        }
    }
    return f;
}

vector<UNIT> idft(vector<UNIT> f, size_t n){
    // s[k] =   sum<j=0 to n-1>(f[j] * e^(i2<pi>jk/n)) / n                              // base equation
    //      =>  sum<j=0 to n-1>(f[j] * (cos(2<pi>jk/n) + i * sin(2<pi>jk/n)) ) / n      // expanded for computation
    // s[k] : sample sequence
    // f[k] : frequency content
    // expanded using : e^(ix) = cos(x) + i*sin(x)
    
    vector<UNIT> s(n); // s[k]
    
    f.resize(n);// handles zero padding

    for (int k = 0; k < n; k++) {
        for (int j = 0; j < n; j++) {
            s[k] += f[j] * UNIT(cos(2 * M_PI * j * k / n), sin(2 * M_PI * j * k / n));
        }

        s[k] /= n;
    }
    return s;
}