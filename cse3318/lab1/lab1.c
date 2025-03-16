/**
 * cse3318-003 lab1
 * george  boone 
 * 1002055713
 * 
 * compile on OMEGA
    $ gcc ./lab1.c -std=c99
*/

/* tested with powershell & msys64
    curl https://ranger.uta.edu/~weems/NOTES3318/LAB/LAB1SPR24/lab1spr24a.dat | Set-Content test1.data   ;
    curl https://ranger.uta.edu/~weems/NOTES3318/LAB/LAB1SPR24/lab1spr24a.out | Set-Content test1.out    ;
    gcc .\lab1.c;
    Get-Content test1.data | ./a.exe | Set-Content res.txt ;
    echo "`n`n nothing should be after this line `n";
    Compare-Object (Get-Content .\res.txt) (Get-Content test1.out); #should return nothing -> it matches
*/

#include <stdio.h>
#include <stdlib.h>

int cmpIdx0(const void*, const void*);
int cmpIdx1(const void*, const void*);
int distinct(int, int m, int[][m]);

int main(){

    /////////////////////
    // inputting
    /////////////////////

    int n;
    scanf("%d", &n);

    //[i][0] -> value
    //[i][1] -> orgional index
    int arr[n][2];
    for(int i = 0; i < n; i++){
        scanf("%d", &arr[i]);
        arr[i][1] = i;
    }


    /////////////////////
    // the actual logic
    /////////////////////

    //sort by value (des)
    qsort(arr, n, sizeof(void*), cmpIdx0);

    //filter out duplicates
    n = distinct(n, 2, arr);

    //sort by orgional index (ase)
    qsort(arr, n, sizeof(void*), cmpIdx1);


    /////////////////////
    // outputting
    /////////////////////

    printf("%d\n",n);
    for(int i = 0; i < n; i++)
        printf("%d\n", arr[i][0]);
        // printf("%2d\t : %5d\n", arr[i][0], arr[i][1]);

    return EXIT_SUCCESS;
}

/** acending order */
int cmpIdx0(const void* a, const void* b){
    return *(int*)a - *(int*)b; //compairs 0'th index of the arrays
}

/** acending order */
int cmpIdx1(const void* a, const void* b){
    return ((int*)a)[1] - ((int*)b)[1]; //compairs 1'th index of the arrays
}

/**
 * removes duplicates in the array, shifts everyhting towards the 0th position.
 * array space is retained but overwritten.
 * retains the last duplicate.
 * 
 * @param arr array to trim
 * @param n  length of array
 * @returns length of new array
*/
int distinct(int n, int m, int arr[][m]){
    int push = 0;   //last pushed index
    int pick = 1;

    for(; pick < n; pick++){
        
        //if value already copied
        if(*arr[pick] == *arr[push]){
            if(arr[push][1] < arr[pick][1]) //update to last index of orgional order
                arr[push][1] = arr[pick][1];
            continue;
        }
        
        push += 1;

        //cant figure out how to just repoint the array so just hard copy it.
        arr[push][0] = arr[pick][0];
        arr[push][1] = arr[pick][1];
    }
    
    return push+1;
}