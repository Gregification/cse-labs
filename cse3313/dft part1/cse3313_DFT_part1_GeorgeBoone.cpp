/**
 * cse3313 spring2025 DFT part 1
 * 
 * 1002055713 
 * George Boone
 * 
 * 
 * build :                  $ g++ -std=c++20 cse3313_DFT_part1_GeorgeBoone.cpp -o dft1.out
 * run w/ streaming:        $ ./dft1.out <dft/idft> <n-points> < file.csv
 * example 64 point dft:    $ ./dft1.out dft 64 < file.csv
 * example 55 point idft:   $ ./dft1.out idft 55 < file.csv
 * example output to clipboard:     $  ./dft1.out dft 64 < response.csv | xclip -selection clipboard
 * 
 * output example : [-1+2i,3+4i]
 * """
 *      -1,2
 *      3,4
 * """
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

typedef long double UNIT;
typedef complex<UNIT> COMPLEX;

vector<COMPLEX> parseInput(istream &input);
vector<COMPLEX> dft(vector<COMPLEX> sequence, size_t n);
vector<COMPLEX> idft(vector<COMPLEX> sequence, size_t n);

int main(int argc, char *argv[]) {
    if (argc != 3 || (string(argv[1]) != "dft" && string(argv[1]) != "idft") || !stoul(argv[2])) {
        cerr << "Usage: " << argv[0]  << " <dft/idft> <n-points>" << endl;
        cerr << "\tprogram must know to preform dft or idft and for how many points. pass it both arguements" << endl;
        return 1;
    }

    bool isDFT = string(argv[1]) == "dft";
    size_t n = stoul(argv[2]);

    vector<COMPLEX> seq = parseInput(cin);

    if(isDFT)
        seq = dft(seq, n);
    else
        seq = idft(seq, n);

    for (int i = 0; i < seq.size(); i++) {
        cout << seq[i].real() << "," << seq[i].imag() << endl;
    }
    return 0;
}

vector<COMPLEX> parseInput(istream &is){
    vector<COMPLEX> arr;

    // parse complex input to vector
    // example input : 0.98,-0.89655-0.47921i,0.55137+0.82518i,-0.097637-0.99133i,
    for(string str; getline(is, str, ',');) {
        COMPLEX temp;

        try{
            size_t numend;
            temp.real(stold(str, &numend));
            temp.imag(stold(str.substr(numend)));
        } catch(const exception&){}

        arr.push_back(temp);
    }
    return arr;
}

vector<COMPLEX> dft(vector<COMPLEX> s, size_t n){
    // f[k] =   sum<j=0 to n-1>(f[j] * e^(-i2<pi>jk/n))                              // base equation
    //      =>  sum<j=0 to n-1>(f[j] * (cos(-2<pi>jk/n) + i * sin(-2<pi>jk/n)) )     // expanded for computation
    // f[k] : frequency content
    // s[k] : sample sequence
    // expanded using : e^(ix) = cos(x) + i*sin(x)
    
    vector<COMPLEX> f(n); // f[k]

    s.resize(n);// handles zero padding

    for (UNIT k = 0; k < n; k++) {
        for (UNIT j = 0; j < n; j++) {
            f[k] += s[j] * COMPLEX(cos(-2 * M_PI * j * k / n), sin(-2 * M_PI * j * k / n));
        }
    }
    return f;
}

vector<COMPLEX> idft(vector<COMPLEX> f, size_t n){
    // s[k] =   sum<j=0 to n-1>(f[j] * e^(i2<pi>jk/n)) / n                              // base equation
    //      =>  sum<j=0 to n-1>(f[j] * (cos(2<pi>jk/n) + i * sin(2<pi>jk/n)) ) / n      // expanded for computation
    // s[k] : sample sequence
    // f[k] : frequency content
    // expanded using : e^(ix) = cos(x) + i*sin(x)
    
    vector<COMPLEX> s(n); // s[k]
    
    f.resize(n);// handles zero padding

    for (UNIT k = 0; k < n; k++) {
        for (UNIT j = 0; j < n; j++) {
            s[k] += f[j] * COMPLEX(cos(2 * M_PI * j * k / n), sin(2 * M_PI * j * k / n));
        }

        s[k] /= n;
    }
    return s;
}