/**
 * cse3318-003 lab4
 * george boone
 * 1002055713
 * 
<<<<<<< HEAD
 * compile on OMEGA, runs using the file redirect thing "<"
 * executable is "lab4.out"
    $ gcc -c lab4.c -o l4.o; gcc -c lab4driver.c -o l4d.o; gcc -o lab4.out l4.o l4d.o;
    $ ./lab4.out < a.dat

    table print format
    parent nodes on left, child nodes on right

    [item, subtree-size]    
                |_[left node]
                |       |_[left of left node]
                |       |_[end]             <--- sentinal node
                |_[right node]
                |       |_[end]
                |       |_[right of right node]
=======
 * compile on OMEGA
    $   gcc -std=c99 -c lab4.c -o l4.o;
        gcc -std=c99 -c lab4driver.c -o l4d.o;
        gcc -std=c99 -o lab4.out l4.o l4d.o;
    $ ./lab4.out < a.dat
>>>>>>> 95cfc9e78f026f747951f2fcf6e9e31a49943a95
*/

#include "lab4.h"

#define DEBUG_PARSER 0
#define isSentinal(node_ptr) (node_ptr->N < 0)
#define MAX_NODE_STR_LEN 21 //max of 21 digits in 64bit base 10

int NULLitem = INT_MIN;
link sentinel;

<<<<<<< HEAD
//spaghetti code
char **ptstr;
=======
//global pointer to input string
char *ptstr = 0;
>>>>>>> 95cfc9e78f026f747951f2fcf6e9e31a49943a95

void STinit(){          // Initialize tree with just a sentinel
    sentinel = createSTnode();
    sentinel->N = -1;
}

link createSTnode(){
    link node = malloc(sizeof(struct STnode));
    struct STnode tmp = {
            .r = sentinel,
            .l = sentinel,
            .item = NULLitem
        };
    memcpy(node, &tmp, sizeof(struct STnode));
    return node;
}

void STterminate(){
    //lazy
    //find a leaf -> delete that leaf -> repeat

    while(!isSentinal(sentinel->l)){//if tree has nodes
        link 
            former = sentinel,
            node = sentinel->l;

        //go to child
        while(!isSentinal(node)){
            int 
                leftValid = !isSentinal(node->l),
                rightValid = !isSentinal(node->r);

            if(!leftValid && !rightValid){
                if(node == former->l)
                    former->l = sentinel;
                else former->r = sentinel;

                free(node);
                break;
            }

            former = node;
            if(leftValid)
                node = node->l;
            else
                node = node->r;
        }
    }

    free(sentinel);
    if(ptstr) free(ptstr);

    exit(0);
}

Item STsearch(Key v){   // Find node for a key
    link ret = sentinel->l;

    while(!isSentinal(ret)){
        if(eq(ret->item, v)) 
            return ret->item;

        int isLess = less(ret->item, v);

        if(!isSentinal(ret->l) && isLess)
            ret = ret->l;
        else if(!isSentinal(ret->r) && !isLess)
            ret = ret->r;
        else break;
    }

    return NULLitem;
}

Item STselect(int k){   // Treat tree as flattened into an ordered array
    link node = sentinel->l;

    while(!isSentinal(node)){
        int apparentRank = (isSentinal(node->l) ? 0 : node->l->N) + 1; //[left ST count] + 1
        if(apparentRank == k) return node->item;
        
        if(k < apparentRank){
            node = node->l;
        }else {
            node = node->r;
            k -= apparentRank;
        }
    }

    return NULLitem;
}

int STinvSelect(Key v){ // Inverse of STselect
    link node = sentinel->l;

    while(!isSentinal(node)){
        if(eq(node->item, v))
            return (isSentinal(node->l) ? 0 : node->l->N) + 1;;

        if(less(v, node->item))
            node = node->l;
        else 
            node = node->r;
    }
    
    return -1;
}

void STinsert(Item item){     // Insert an item.  No uniqueness check
    link 
        node    = sentinel->l,
        former  = sentinel,
        new     = createSTnode();

    int isLess;

    new->N      = 1;
    new->item   = item;

    //find the leaf that it should get put to
    while(!isSentinal(node)){
        former = node;
        node->N++;

        isLess = less(item, node->item);
        if(isLess)  node = node->l;
        else        node = node->r;
    }

    if(isLess)
        former->l = new;
    else
        former->r = new;
}

//return true if tree is invalid
int STverifyProperties(){    // Ensure that tree isn't damaged
    return STverifyPropertiesHelper(sentinel->l, INT_MIN, INT_MAX);
}
int STverifyPropertiesHelper(link node, Key min, Key max){
    if(isSentinal(node))
        return 0;

    if(less(node->item, min) || less(max, node->item))
        return 1;

    int size = 1;
    if(!isSentinal(node->l)) size += node->l->N;
    if(!isSentinal(node->r)) size += node->r->N;

    if(size != node->N) return 1;

    return 
        STverifyPropertiesHelper(node->l, min, node->item) ||
        STverifyPropertiesHelper(node->r, node->item, max);
}
int STcalcSTsize(link root){
    if(isSentinal(root)) return 0;

    int size = 1;
    size += STcalcSTsize(root->l);
    size += STcalcSTsize(root->r);
    root->N = size;

    return size;
}

void STprintTree(){           // Dumps out tree
    STprintSubTree(sentinel->l, 0);
}
void STprintSubTree(link root, int indent){
    for(int i = 0; i < indent; i++)
        printf("%12s", "|");
    
    if(isSentinal(root)){
        puts("_[end]");
        return;
    }

    printf("_[%3d, %2d]\n", root->item, root->N);
    STprintSubTree(root->r, indent+1);
    STprintSubTree(root->l, indent+1);
}

char* STserialize(){    // Flattens current tree into a pre-order string
    if(isSentinal(sentinel->l))
        return "";
    
    //size of node (assuming 64bit int) is at most 20 chars, "+1" for the sign. (MAX_NODE_STR_LEN = 21)
    //at worst each node gets 1 period tagged on so "+N"  on top of that
    //+1 for null terminating
    //ends up as N * MAX_NODE_STR_LEN + N, simpilified
    //complete overkill it semes, ~15kb(reserved) vs ~8kb(used) - valgrind
    char* ret = calloc(sentinel->l->N * (MAX_NODE_STR_LEN + 1) + 1, sizeof(char));

    STserializeHelper(sentinel->l, ret);

    return ret;
}
void STserializeHelper(link node, char* str){
    if(isSentinal(node)){
        strcat(str, ".");
        return;
    }
    char arr[MAX_NODE_STR_LEN];
    int l = sprintf(arr, "%d",node->item);
    strcat(str, node->item > 0 ? "+" : "");
    strcat(str, arr);
    STserializeHelper(node->l, str);
    STserializeHelper(node->r, str);
}

void STdeserialize(char *str){     // Parses string to current tree
    ptstr = str; //save it to a global so it can be freed internaly later

    //prime tree with root element
    link root = createSTnode();
        root->item = getValFromStr(&str);
        root->N = 1;
    sentinel->l = root;

    //populate tree
    STdeserializeHelper(&str, root);
    
    //correct the subtree sizes. (populating the tree dosent set it)
    STcalcSTsize(root);

    //tests
    if(strlen(str) != 0)        //was entire string used?
        puts("\x1B[33m[WARNING] extra characters in input! \x1B[39m ");
    if(STverifyProperties())    //checks subtree sizes and if children are within expected ranges
        puts("\x1B[33m[WARNING] invalid tree (either node order or subtree size)\x1B[39m ");
}
void STdeserializeHelper(char **pts, link root){
    if(!root) return;

    //for each possible child
    for(int i = 0; i < 2; i++){
        switch((*pts)[0]){
            default: {
                printf("bad input \"%c\" in %s\n", (*pts)[0], *pts);
                STterminate();
            }

            //skip assigning current child
            case '.'    :
                (*pts) += 1;
                if(DEBUG_PARSER) puts("next");
                continue;
            
            //valid cases
            case '+'    :
            case '-'    :
            case '\0'   :
                break;
        }


        //inserting node
        link* too;
        switch(i){ //get the current child being assigned
            case 0: too = &(root->l);  break;
            case 1: too = &(root->r);  break;

            default:{//uh oh
                if(DEBUG_PARSER) printf("[ERROR] undefined child index: %d\n quitting...", i);
                STterminate();
            }
        }

        *too = createSTnode();
        (*too)->N = 1;
        (*too)->item = getValFromStr(pts);

        //recurse to child
        STdeserializeHelper(pts, *too);
    }
}
Item getValFromStr(char **pts){ //gets first value & incriments
    int off = 1;
    char* str = *pts;

    while(str[off] <= '9' && str[off] >= '0') off++;

    switch(str[off]){
        case '+'    :
        case '-'    :
        case '.'    :
        case '\0'   :
            break;
        default: {
            printf("bad input \"%c\" in %s\n", str[off], str);
            STterminate();
        }
    }

    char og = str[off];
    str[off] = '\0';
    Item ret = atoi(str);
    str[off] = og;

    if(DEBUG_PARSER) printf("%d\n\t%s\n\t%s\n", ret, str, str+off);

    *pts += off;

    return ret;
}


int getLive(){  // Number of nodes in tree with active keys.
    if(isSentinal(sentinel->l))
        return 0;
    
    return sentinel->l->N;
}
