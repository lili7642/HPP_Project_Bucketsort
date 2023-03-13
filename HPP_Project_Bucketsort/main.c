#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// Prints out the list to the N:th element.
void print_list(int* list, int N){
    for (int i = 0; i < N; i++)
    {
        printf("%d ", list[i]);
    }
    printf("\n");  
}

void fill_list(int *list, int N, char *dist){
    // uniform dist
    if(!(strcmp(dist, "uniform"))){
        printf("Uniform distribution\n");
        for(int i = 0; i < N; i++){
            list[i] = rand() % N; // makes each element a uniform random number between 0 and N (size of list)
        }
    }

    // normal dist

    // exponential dist
}



int main(int argc, char *argv[]){
    if(argc != 5){
        printf("Excpected list size argument.\nUsage: %s N_list N_buckets N_threads Dist_type\n", argv[0]);
        return -1;
    }

    int N_list = atoi(argv[1]);
    int N_buckets = atoi(argv[2]);
    int N_threads = atoi(argv[3]);
    char *dist_type = argv[4];

    // CREATE LIST
    int *list = malloc(N_list * sizeof(*list)); // malloc does not have to be typecasted?
                                                // also sizeof(*list) => type only declared in LHS
    if(!list){
        printf("Error: Memory allocation error\n");
        return -1;
    }


    // FILL LIST
    fill_list(list, N_list, dist_type);
    print_list(list, N_list);

    // CREATE BUCKETS
    int **buckets = malloc(N_buckets*sizeof(*buckets)); //buckets is array of int pointer pointers
    int *bucket_count = malloc(N_buckets*sizeof(*bucket_count)); //bucket_count keep tracks of how many elements are in each bucket
    for(int i = 0; i < N_buckets; i++){
        buckets[i] = malloc(N_list*sizeof(*list)); // memory to fit entire list allocated in each bucket
    }

    // SORT ELEMENTS INTO BUCKETS

    // find max and min:
    int max_element = list[0];
    int min_element = list[0];
    for(int i = 1; i < N_list; i++){
        if(list[i] > max_element) max_element = list[i];
    }

    // calculate factor

    // SORT BUCKETS LOCALLY

    // PUT ELEMENTS FROM BUCKET BACK INTO LIST IN ORDER

    // FREE EVERYTHING
    free(list);
    free(bucket_count);
    for(int i = 0; i < N_buckets; i++){
        free(buckets[i]);
    }
    free(buckets);




}