/**
 * cse3318-003 lab5
 * george boone
 * 1002055713
 * 
 * - must use GCC or Clang to compile. (switch statement depends on a feature)
 * - if the table is hard to read, toggle "FANCY_PRINT" and recompile
 * 
 * runs in O(v^3 + 2 * v^2)
 *      > v^3 : Warshall's algorithm
 *      > v^2 : finding cycles
 *      > v^2 : print path to leader
 *      ? FANCY_PRINT being enabled changes run time to O(v^6)
 * 
 * compile on OMEGA
    $   gcc -std=c99 lab5.c -o lab5.out;
    $ ./lab5.out < a.dat
*/
#define FANCY_PRINT 1

#include <stdlib.h>
#include <stdio.h>

#define BAD 52  //must be a "large 'infinite' value", (V shouldnt exceed 50) 

typedef char unt; //count units to be extra fancy

void checkAlloc(void*);
void printMat(unt, unt**);
void findCycles(unt*, unt, unt, int);
void printPath(unt, unt);

unt v;      //number of verticies
unt **mat;  //adjacency matrix
unt* cycl;  //cycle tracker

int main(){
    //get input
    {   
        //get v
        {
            int d;
            scanf("%d", &d);
            if(d > 50){
                puts("more than 50 verticies! HW section \"Requirements 1.a\" specifies that \"V will not exceed 50\"");
                puts("the program will work with more but you have to change the typedef of \"unt\"(line 16) form char to something larger (e.g:int) and change the BAD place holder to something else");
                exit(EXIT_FAILURE);
            }
            v = d;
        }

        mat = malloc(sizeof(unt*) * v);
        checkAlloc(mat);
        for(int i = 0; i < v; i++){
            mat[i] = malloc(v * sizeof(unt));
            checkAlloc(mat[i]);
        }

        for(int y = 0; y < v; y++)
            for(int x = 0; x < v; x++)
                mat[y][x] = BAD;

        for(int a = 0, b = 0;;){
            scanf("%d %d", &a, &b);
            if(a == -1) break;
            mat[a][b] = b;
        }
    }

    puts("initial table");
    printMat(v, mat);
    
    //warshalls algo O(v^3)
    {
        for(int x = 0; x < v; x++){
            for(int y = 0; y < v; y++){
                if(mat[y][x] == BAD) continue;

                for(int fx = 0; fx < v; fx++){
                    if(mat[y][fx] != BAD) continue;
                    if(mat[x][fx] == BAD) continue;
                    mat[y][fx] = mat[y][x];
                }
                
            }
            if(x+1 < v) {
                printf("\nafter processing column %d\n", x);
                printMat(v, mat);
            }
        }
    }

    puts("\nfinal table");
    printMat(v, mat);

    if(FANCY_PRINT){
        printf("\n\t\x1B[32m*%s",   "direct connection. ");
        printf("\n\t\x1B[1;34m*%s", "a short path.      ");
        printf("\n\t\x1B[1;33m*%s", "a medium path.     ");
        printf("\n\t\x1B[1;31m*%s", "a long path.       ");
        printf("\n\t\x1B[35m*%s",   "no path exists.    ");
        puts("\x1B[37m");
    }

    //find cycles, O(v^2)
    {
        cycl = malloc(v * sizeof(unt));
        checkAlloc(cycl);
        for(int i = 0; i < v; i++) cycl[i] = i; //assume all nodes are criticle

        for(int y = 0; y < v; y++){
            for(int x = 0; x < v; x++){

                if(mat[y][x] == BAD) continue;  //if A no connect to B
                if(mat[mat[y][x]][y] == BAD) continue; //if B no connect to A

                unt cb = cycl[mat[y][x]];//cycle of B

                //if cycles are already joined
                if(cb == cycl[y]) continue;

                //join the 2 cycles, use smallest id
                unt from = cycl[y], to = cb; 
                if(from < to){ unt tmp = to; to = from; from = tmp; }

                for(int i = 0; i < v; i++)
                    if(cycl[i] == from)
                        cycl[i] = to;
            }
        }
    }
    printf("\ncycles\n%5s", "");
    for(int x = 0; x < v; x++)
        printf("%3d", cycl[x]);
    puts("\n");


    //print cycle leaders and traces
    for(int i = 0; i < v; i++){
        if(cycl[i] == BAD) continue;

        if(cycl[i] == i){
            printf("%2d is a leader\n", i);

            for(int oi = 0; oi < v; oi++) {
                if(oi == i || cycl[oi] != cycl[i]) continue;

                printf("\t#%2d under leader(%2d)", oi,  cycl[i]);
                printf("\n\t\tpath to   : ");
                printPath(oi, cycl[i]);
                printf("\n\t\tpath from : ");
                printPath(cycl[i], oi);
                printf("\n");
                cycl[oi] = BAD;
            }
        }
        cycl[i] = BAD;
    }

    return EXIT_SUCCESS;
}

void findCycles(unt* cycl, unt start, unt target, int c){
    if(start == BAD || cycl[start]) return;
    if(mat[start][target] == BAD) return;

    cycl[start] = c;

    for(int x = 0; x < v; x++){
        findCycles(cycl,mat[start][x],  target, c);
    }
}

void printPath(unt from, unt to){
    printf("%3d", from);
    while(mat[from][to] != to){
        from = mat[from][to];
        printf("%3d", from);
    }
    printf("%3d", mat[from][to]);
}

void printMat(unt v, unt** mat){
    int mx, my;
    mx = my = v;

    printf("\x1B[37m");
    printf("\n%5s", "idx");
    for(int x = 0; x < mx; x++)
        printf(" %2d", x);
    printf("\n______");
    for(int x = 0; x < mx; x++)
        printf("___");
    puts("");

    for(int y = 0; y < my; y++){
        printf("%2d | ", y);
        for(int x = 0; x < mx; x++){
            unt c = mat[y][x];

            if(!FANCY_PRINT){
                if(c == BAD) printf("%3s", "-");
                else         printf("%3d", c);

                continue;
            }


            if(c == BAD)
                printf("\x1B[35m%3s", "-");
            else { 
                if(c == x)
                    printf("\x1B[32m");
                else {
                    unt i = 0;
                    for(int ny = y; mat[ny][x] != x; i++)
                        ny = mat[ny][x];
                    
                    i = i * 100.0 / v;

                    switch(i){
                        case 0  ... 14    : {printf("\x1B[1;34m"); break;}  //short
                        case 15 ... 30    : {printf("\x1B[1;33m"); break;}  //medium
                        case 31 ... 100   : {printf("\x1B[1;31m"); break;}  //long
                        default         : {printf("\x1B[37m"); break;}      //broke :(
                    }
                }

                printf("%3d", c);
            }
        }
        printf("\n\x1B[37m");
    }
}

void checkAlloc(void* ptr){
    if(!ptr){
        puts("bad alloc");
        exit(EXIT_FAILURE);
    }
}