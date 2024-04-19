/**
 * cse3318-003 lab5
 * george boone
 * 1002055713
 * 
 * - must use GCC or Clang to compile. (switch statement depends on a feature)
 * - if the table is hard to read compile with "FANCY_PRINT" to 0
 * 
 * compile on OMEGA
    $   gcc -std=c99 -c lab5.c -o lab5.out;
    $ ./lab5.out < a.dat
*/
#define FANCY_PRINT 1

#include <stdlib.h>
#include <stdio.h>

#define BAD -1

typedef char cnt; //count units to be extra fancy

void checkAlloc(void*);
void printMat(cnt, cnt**);

int main(){
    cnt v;      //number of verticies

    cnt **mat;  //adjacency matrix

    //get input
    {   
        //get v
        {
            int d;
            scanf("%d", &d);
            if(d > 50){
                puts("more than 50 verticies! In the HW section \"Requirements 1.a\" specifies that \"V will not exceed 50\"");
                puts("the program will work with more but you have to change the typedef of \"cnt\"(line 16) form char to something larger (e.g:int)");
                exit(EXIT_FAILURE);
            }
            v = d;
        }

        mat = malloc(sizeof(cnt*) * v);
        checkAlloc(mat);
        for(int i = 0; i < v; i++){
            mat[i] = malloc(v * sizeof(cnt));
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
    
    //warshalls algo. implimentation
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

    puts("\nfinal table");
    printMat(v, mat);

    if(FANCY_PRINT){
        printf("\n\t\x1B[32m*%s",   "direct connection. ");
        printf("\n\t\x1B[1;34m*%s", "a short path.      ");
        printf("\n\t\x1B[1;33m*%s", "a medium path.     ");
        printf("\n\t\x1B[1;31m*%s", "a long path.       ");
        printf("\n\t\x1B[35m*%s",   "no path exists.    ");
        puts("");
    }

    return EXIT_SUCCESS;
}

void printMat(cnt v, cnt** mat){
    int mx, my;
    mx = my = v;

    printf("\x1B[37m%5s", "");
    for(int x = 0; x < mx; x++)
        printf(" %2d", x);
    printf("\n______");
    for(int x = 0; x < mx; x++)
        printf("___");
    puts("");

    for(int y = 0; y < my; y++){
        printf("%2d | ", y);
        for(int x = 0; x < mx; x++){
            cnt c = mat[y][x];

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
                    cnt i = 0;
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