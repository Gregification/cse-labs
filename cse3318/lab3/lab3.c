/**
 * cse3318-003 lab3
 * george boone
 * 1002055713
 * 
 * worst case time : O(mn * nn * ln(n))
 * explanation
 *      mn : to fill the table
 *      nn / 3 * ln(n) : to back trace
 *          > n     : # of backtraces at most. there are alwayse n number of starting points to check. if n checks are used at least 1 is guarenteed to skip most of the rows but the limit still approaches n, regradless
 *          > n/3   : the number of possibilities to check. table can be 99% incrimenting in a diagional direction so that all traces (except 1) will have to check all rows, BUT other tarces would have reserved rows
 *          > logBase3(n)   : looks to find a unused spot for each row. there are on average a logarithmic amount of invalid cells
 * space complexity : [theta](mn + 2n)
 *      mn  : table size
 *      n   : input size
 *      n   : output size
 * 
 * compile on OMEGA, runs using the file redirect thing "<"
    $ gcc lab3.c -std=c99

    or ... c&p for test

    $ gcc lab3.c -std=c99 -o lab3.out; ./lab3.out < a.data
*/
#include <stdio.h>
#include <stdlib.h>

#define NUM_SOLUTIONS 3 //the algorithm should work to find any number of subsets. i think
#define COLOR_PRINT 1 //unix terminal colors. powershell does not like

void pritntable(int** table, int x, int y, int NAN);

int n;      //count
int* in;    //input
int* out;   //output
int**table; //backtrace table. [num input]x[target+1]
int target; //target sum for each subset. 1/3 of total sum

int solutionExists = 1; //error flag

int main(){

    ////////////////////////////////////////////////////////////////////
    // init
    ////////////////////////////////////////////////////////////////////
    scanf("%d", &n);

    in  = malloc(n * sizeof(int));
    out = malloc(n * sizeof(int));

    //read input & find target
    for(int i = 0; i < n; i++){
        scanf("%d",in + i);
        target += in[i];
    } 
    target /= NUM_SOLUTIONS;
    const int NAN = target+1; //this number or anything higher indicates a bad value

    //alloc table
    table = malloc(n * sizeof(int*));
    if(!table){ puts("failed to alloc table"); n = 0; goto exit; }
    
    //alloc table rows
    for(int i = 0; i < n; i++){ 
        table[i] = malloc((target+1) * sizeof(int));
        if(!table[i]){ printf("failed to alloc table row #%d out of %d", i, n); n = i;  goto exit; }
    }

    //test input
    {
        int sum = 0, isBad = 0;
        for(int i = 0; i < n; i++)
            sum += in[i];
        
        if(n < NUM_SOLUTIONS){
            printf("not enough elements. must be at least %d elements\n", NUM_SOLUTIONS);
            isBad++;
        }if(sum % NUM_SOLUTIONS != 0){
            printf("invalid input. total sum not divisible by %d\n", NUM_SOLUTIONS);
            isBad++;
        }if(in[n-1] > sum){
            puts("invalid input. contians a number thats too large");
            isBad++;
        }

        if(isBad) {
            solutionExists = 0;
            goto print;
        }
    }

    ////////////////////////////////////////////////////////////////////
    // actual algorithm
    ////////////////////////////////////////////////////////////////////

    /**
     * populate table 
     * > table is [num input]x[target+1]
     */

    for(int x = 0; x < n; x++){             //for each input
        for(int y = 0; y <= target; y++){   //for each up to target
            const int diff = y - in[x];
            
            if(diff < 0)          //match not possible
                table[x][y] = NAN;
            else if(diff == 0)   //exact match found
                table[x][y] = diff;
            else {                //cannot match using a single index
                //check if diff can be resolved using a former value
                //otherX, otherY
                int oX = 0;

                for(;oX <= x && table[oX][diff] >= NAN; oX++)
                    ;

                if(oX == x+1)
                    table[x][y] = NAN;
                else
                    table[x][y] = diff;
            }
        }
    }

    //test that at least 3 posible solutions exist
    {   
        int count = 0;
        for(int x = 0; x < n; x++){
            if(table[x][target] < NAN){
                count++;
                if(count == NUM_SOLUTIONS)
                    break;
            }
        }
        if(count < NUM_SOLUTIONS){
            printf("No solution. (%d out of %d possible subarrays)\n", count, NUM_SOLUTIONS);
            solutionExists = 0;
        }
    }

    /**
     * backtrace
     *  > greedy trace starting from highest input index.
     *  > finds all subsets through tracing, no process of elimination used
     */
    for(int i = 0; i < n; i++)
        out[i] = -1;

    int ssid = 0; //subset id
    int sum = 0;
    for(int solution = n-1; solution >= 0; solution--){         //for each possiable solution
        int y = target, x = solution;
        
        //if all solutions are found
        if(ssid == NUM_SOLUTIONS) break;

        //skip if space already used
        if(out[x] != -1) continue;
        
        sum = 0;        
        out[x] = ssid;  //mark head as being use by current subset

        while(x >= 0){
            sum += in[x];
            if(table[x][y] == 0){ //if is end of subarray
                
                //make sure sum is correct
                //sum is wrong when the trace stops early (no paths avalible from that position, but trace is not done)
                if(sum != target){//if sum is incorrect, current subset is wrong
                    //remove everything marked by current subset
                    for(int i = 0; i < n; i++)
                        if(out[i] == ssid) out[i] = -1;

                } else { //else, subset is valid
                    //the backtrace wont trace a zero so they're manually included into the first subset
                    if(ssid == 0)
                        for(int i = 0; i < n; i++)
                            if(out[i] == -1 && in[i] == 0)
                                out[i] = 0;

                    ssid++;
                }

                break;
            }

            //goto next layer
            y = table[x][y];

            //find index, that skips the most amount of rows, in the next layer
            for(x = n-1; x >= 0; x--)
                if(table[x][y] < NAN)   //if is a valid value
                    if(out[x] == -1)    //if is not already used
                        break;

            if(x == -1){ //if subset not possible
                //remove everything marked by current subset
                for(int i = 0; i < n; i++)
                    if(out[i] == ssid) out[i] = -1;

                break;
            }

            out[x] = ssid;
        }
    }
    
    //test that all positions are used
    if(solutionExists){
        for(int i = 0; i < n; i++)
            if(out[i] < 0 || out[i] >= NUM_SOLUTIONS){
                printf("No Solution, failed to account for input index %d\n", i);
                solutionExists = 0;
                break;
            }
    }

    ////////////////////////////////////////////////////////////////////
    // clean up
    ////////////////////////////////////////////////////////////////////
print:
    //print input
    printf("%d/%d = %d\n", target*3, NUM_SOLUTIONS, target);
    puts("input: i  value");
    for(int i = 0; i < n; i++)
        printf("%6s%2d%7d\n","", i, in[i]);
    puts("");

    //prints backtrace table
    if(target <= 10)
        pritntable(table, n, target+1, NAN);
    
    //print results if solution exists
    if(!solutionExists)
        goto exit;

    printf("%2s   ","i");
    for(int x = 0; x < NUM_SOLUTIONS; x++) printf("%3d  ", x);
    puts("");
    for(int y = 0; y < n; y++){
        printf("%2d | ", y);
        if(out[y] == -1) printf(" ? ");
        else
        for(int x = 0; x < NUM_SOLUTIONS; x++){
            if(out[y] == x && in[y] >= 0)
                printf("%4d ", in[y]);
            else
                printf("%5s","");
        }
        puts("");
    }

exit:
    free(in);
    free(out);

    for(int i = 0; i < n; i++)
        free(table[i]);

    free(table);

    return EXIT_SUCCESS;
}

void pritntable(int** table, int x, int y, int NAN) {
    printf("a cell is bad if its value X >= %d", NAN);
    printf("\nvalue");
    for(int i = 0; i < x; i++)
        printf("%3d  ", in[i]);
    printf("\n t\\i");
    for(int i = 0; i < x; i++)
        printf(" %3d ", i);
    puts("");
    
    for(int i = 0; i < y; i++){
        printf("%2d | ", i);

        for(int j = 0; j < x; j++){
            if(COLOR_PRINT){
                if(table[j][i] >= NAN){
                    printf("\x1B[31m%4s ", "-");
                    continue;
                }
                else if(table[j][i] == 0)
                    printf("\x1B[32m");
                else
                    printf("\x1B[33m");
            }

            printf("%4d ", table[j][i]);
        }
        
        if(COLOR_PRINT) printf("\x1B[37m");
        puts("");
    }
}