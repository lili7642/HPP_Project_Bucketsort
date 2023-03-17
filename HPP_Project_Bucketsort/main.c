#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#ifdef _OPENMP
    #include <omp.h>
#endif

// swap the values of two pointers, used only for quicksorting as full implementation did not increase performance
void swap(int* a, int* b){
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Prints out the list to the N:th element, used for testing puroposes
void print_list(int* list, int N){
    for (int i = 0; i < N; i++)
    {
        printf("%d ", list[i]);
    }
    printf("\n");  
}

void fill_list(int *list, int N, char *dist){
    srand(time(NULL)); // set unique seed

    // uniform dist
    if(!(strcmp(dist, "uniform"))){
        printf("Uniform distribution\n");
        for(int i = 0; i < N; i++){
            list[i] = rand() % N; // makes each element a uniform random number between 0 and N (size of list)
        }
    }

    // normal dist
    else if(!(strcmp(dist, "normal"))){
        printf("Normal distribution\n");

        // some sort of box muller transform

        int mean = N/2;
        int std = N/12;

        double u, v, z;

        for (int i = 0; i < N; i++){
            u = (double)rand()/RAND_MAX; // two values between 0 and 1
            v = (double)rand()/RAND_MAX;

            z = sqrt(-2*log(u))*cos(2*3.1415*v);
            list[i] = z*std + mean;
        }
    }

    // exponential dist
    else if(!(strcmp(dist, "exponential"))){
        printf("Exponential distribution\n");

        // Using inverse density function transform

        double lambda = 1/0.002; //precalculate division, value chosen arbirtarily
        double u; 
        for (int i = 0; i < N; i++){
            u = (double)rand()/RAND_MAX;
            list[i] = -log(1-u)*lambda;
        }
        

    }
}

// SORTING METHODS --------------------------------------------------------------------------------------------
// insertion sort from wikipedia
void insertionsort( int *list, int N){
    int i, j, val;
    for (i = 1; i < N; i++){
        val = list[i];
        j = i-1;
        while( j >= 0 && list[j] > val){
            list[j+1] = list[j];
            j--;
        }
        list[j+1] = val;
    }
    
}

// quicksort stuff

// algorithm to chose a pivot index for quicksorting
int pivot(int *list, int lower, int upper){
    return(upper); // last element will suffice as pivot element however it will suffer for exponential distribution
}

// swap all elements in a list over a pivot element
int partition(int *list, int lower, int upper){
    int piv_index = pivot(list, lower, upper);
    int piv = list[piv_index];

    swap(&list[upper], &list[piv_index]);

    int i = lower - 1;
    for (int j = lower; j < upper; j++){
        if (list[j] < piv){
            i++;
            swap(&list[i], &list[j]);
        }
    }

    swap(&list[i + 1], &list[upper]); // swap the pivot element to its correct position
    return(i + 1); // all elements smaller than the pivot element will be to the left of i+1
}

// QUICK SORT
void quicksort(int* list, int lower, int upper){
    // only sort if list is longer than one element
    if(lower < upper){
        int piv_index = partition(list, lower, upper); // find pivot element
        quicksort(list, lower, piv_index - 1);
        quicksort(list, piv_index + 1, upper);
    }
}

// --------------------------------------------------------------------------------------------------------------

// check if a list is sorted in ascending order
int is_sorted(int *list, int size){
    int flag = 1, i = 0;
    while(flag == 1 && i+1 < size){
        if(list[i]>list[i+1]) flag = 0;
        i++;
    }
    if(!flag){
        printf("\n\n UNSORTED ELEMENTS IN INDEX %d and %d:\n%d and %d.\n", i, i+1, list[i], list[i+1]);
    }
    return flag;
}

// Function for printing out sorted and unsorted list to a txt file
void print_to_file(char *filename, int *list, int N){
    FILE *fp;
    fp = fopen(filename, "w");

    for (int i = 0; i < N; i++){
        fprintf(fp, "%d ", list[i]);
        if((i+1)%30==0)fprintf(fp,"\n");
    }
    fclose(fp);
}

int main(int argc, char *argv[]){
    if(argc != 6){
        printf("Error: Excpected 5 arguments...\nUsage: %s N_list N_buckets N_threads Distribution(uniform/normal/exponential) print(1/0)\n", argv[0]);
        return -1;
    }
    if(strcmp(argv[4],"uniform") && strcmp(argv[4],"normal") && strcmp(argv[4],"exponential")){
        printf("Error: allowed distribution are 'uniform', 'normal', 'exponential'\n");
        return -1;
    }

    int N_list = atoi(argv[1]);
    int N_buckets = atoi(argv[2]);
    int N_threads = atoi(argv[3]);
    char *dist_type = argv[4];
    int printflag = atoi(argv[5]);

    if(printflag != 1 && printflag != 0){
        printf("Error: Invalid print value. Allowed values: 1/0\n");
        return -1;
    }

    if(N_threads > 1){
        printf("Running in parallel on %d threads\n", N_threads);
        omp_set_num_threads(N_threads);
    }else{
        printf("Running in serial on 1 thread\n");
        omp_set_num_threads(1);
    }

    // CREATE LIST --------------------------------------------------------------------------------------------------
    int *list = malloc(N_list * sizeof(*list)); // malloc does not have to be typecasted?
                                                // also sizeof(*list) => type only declared in LHS
    if(!list){
        printf("Error: Memory allocation error\n");
        return -1;
    }


    // FILL LIST -----------------------------------------------------------------------------------------------------

    fill_list(list, N_list, dist_type);
    //print_list(list, N_list);

    if(printflag){
        print_to_file("unsorted.txt", list, N_list);
    }

    // begin timing algorithm
    double starttime, endtime, createtime, disttime, sorttime;
    starttime = omp_get_wtime();

    // SORTING STARTPOINT -------------------------------------------------------------------------------------------------

    // find max and min:
    int max_element = list[0];
    int min_element = list[0];
#pragma omp parallel for reduction(max:max_element) reduction(min:min_element)
    for(int i = 1; i < N_list; i++){
        if(list[i] > max_element) max_element = list[i];
        else if(list[i] < min_element) min_element = list[i];
    }
    // calculate variables for hash function
    int range = (max_element - min_element);
    double temp = (double)N_buckets/(range+1);

    // CREATE BUCKETS -----------------------------------------------------------------------------------------------

    int **buckets = malloc(N_buckets*sizeof(*buckets)); //buckets is array of int pointer pointers
    int *bucket_count = calloc(N_buckets, sizeof(*bucket_count)); //bucket_count keep tracks of how many elements are in each bucket

    int *list_element_bucket_count = malloc(N_list * sizeof(*list));
    
    // calc bucket count first, needs to be in serial probably. 
    // This loop is responsible for 75% of time consumption
    int bucket_index;
    for (int i = 0; i < N_list; i++){
        bucket_index = (int)((list[i]-min_element)*temp); // find bucket index using the hashing variable
        list_element_bucket_count[i] = bucket_count[bucket_index];
        bucket_count[bucket_index]++; // count how many elements are going in each bucket
    }

    for(int i = 0; i < N_buckets; i++){
        buckets[i] = malloc(bucket_count[i]*sizeof(*list)); // allocate only necessary memory!
    }

    createtime = omp_get_wtime();
    
    // SORT ELEMENTS INTO BUCKETS ---------------------------------------------------------------------------------

    #pragma omp parallel for
    for (int i = 0; i < N_list; i++){ // loop through buckets
        bucket_index = (int)((list[i]-min_element)*temp);
        buckets[bucket_index][list_element_bucket_count[i]] = list[i]; // insert value in its correct bucket
    }
    //this is no longer needed
    free(list_element_bucket_count);


    disttime = omp_get_wtime();   

    // SORT BUCKETS LOCALLY --------------------------------------------------------------------------------

    // some factor to decide wether to quicksort or insertion sort
    int factor = 10; // seems like a good value
    factor = 0;

#pragma omp parallel for schedule(dynamic) // schedule dynamic makes sense
    for (int i = 0; i < N_buckets; i++){

        // choose local sorting method depending on bucket size
        if(bucket_count[i] >= factor){
            quicksort(buckets[i], 0, bucket_count[i]-1); // perform quicksort on bucket
        }
        else{
            insertionsort(buckets[i], bucket_count[i]); // perform insertion sort on bucket
        }
    }

    sorttime = omp_get_wtime();
    
    // PUT ELEMENTS FROM BUCKET BACK INTO LIST IN ORDER --------------------------------------------------
    
    int *bucket_accum = malloc((N_buckets)*sizeof(*bucket_accum)); // cumulative list

    // Calculate accumulative sum
    bucket_accum[0] = 0;
    for(int i = 1; i<N_buckets; i++){
        bucket_accum[i] = bucket_count[i-1] + bucket_accum[i-1]; //bucket_accum begins with 0 and is 1 element behind bucket_count
    }                                                            //essentially counting how many elements there are before each bucket
    #pragma omp parallel for
    for(int i = 0; i < N_buckets; i++){
        for(int j = 0; j < bucket_count[i]; j++){
            list[bucket_accum[i] + j] = buckets[i][j]; //find correct list index using cumulative sum
        }
    }
    
    
    //end of sorting time

    endtime = omp_get_wtime();

    printf("TIME: Total: %lf s, Bucket creation: %lf s, Bucket Dist: %lf s, Local sorting: %lf s, Put back: %lf s \n", endtime - starttime, createtime - starttime, disttime - createtime, sorttime - disttime, endtime - sorttime);

    //printf("SORTED LIST:\n");
    //print_list(list, N_list);

    // CHECK IF SORTED ----------------------------------------------------------------
    int flag = is_sorted(list, N_list);
    if(flag){
        printf("The list is sorted!\n");
        if(printflag){
            print_to_file("sorted.txt", list, N_list);
        }
    }else{
        printf("The list is not sorted...\n");
    }

    // FREE EVERYTHING --------------------------------------------------------------------
    free(list);
    free(bucket_count);
    for(int i = 0; i < N_buckets; i++){
        free(buckets[i]);
    }
    free(buckets);
    free(bucket_accum);

    printf("\n");
}