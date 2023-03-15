#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#ifdef _OPENMP
    #include <omp.h>
#endif

// swap the values of two pointers
void swap(int* a, int* b){
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Prints out the list to the N:th element.
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
        double lambda = 1/0.002; //precalculate division
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
    return(upper); 
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

void print_to_file(char *filename, int *list, int N){
    FILE *fp;
    fp = fopen(filename, "w");

    for (int i = 0; i < N; i++){
        fprintf(fp, "%d ", list[i]);
        if((i+1)%30==0)fprintf(fp,"\n");
    }
    fclose(fp);
}

void dist_into_buckets(int *list, int N_list, int N_buckets, int **buckets, int* bucket_count){
    // find max and min:
    int max_element = list[0];
    int min_element = list[0];
    for(int i = 1; i < N_list; i++){
        if(list[i] > max_element) max_element = list[i];
        else if(list[i] < min_element) min_element = list[i];
    }
    //printf("Biggest element: %d, smallest element: %d\n", max_element, min_element);

    // calculate factor

    int range = (max_element - min_element);
    double temp = (double)N_buckets/(range+1);
    //printf("Factor: %lf\n", temp);

    for (int i = 0; i < N_list; i++){
        int bucket_index = (int)((list[i]-min_element)*temp);
        buckets[bucket_index][bucket_count[bucket_index]] = list[i];
        bucket_count[bucket_index]++;
    }
}


int main(int argc, char *argv[]){
    if(argc != 6){
        printf("Error: Excpected 5 arguments...\nUsage: %s N_list N_buckets N_threads Dist_type print(1/0)\n", argv[0]);
        return -1;
    }
    if(strcmp(argv[4],"uniform") && strcmp(argv[4],"normal") && strcmp(argv[4],"exponential")){
        printf("Error: allowed distribution are 'uniform', 'normal', 'exponential'");
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
        printf("openmp found, running parallel\n");
        omp_set_num_threads(N_threads);
    }else{
        printf("openmp not found, running serial\n");
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

    double starttime, endtime, createtime, disttime, sorttime, putbacktime;
    starttime = omp_get_wtime();

    //Find some values from list :

    // find max and min:
    int max_element = list[0];
    int min_element = list[0];
    for(int i = 1; i < N_list; i++){
        if(list[i] > max_element) max_element = list[i];
        else if(list[i] < min_element) min_element = list[i];
    }
    //printf("Biggest element: %d, smallest element: %d\n", max_element, min_element);

    // calculate factor

    int range = (max_element - min_element);
    double temp = (double)N_buckets/(range+1);
    //printf("Factor: %lf\n", temp);

    // CREATE BUCKETS -----------------------------------------------------------------------------------------------
    int **buckets = malloc(N_buckets*sizeof(*buckets)); //buckets is array of int pointer pointers
    int *bucket_count = calloc(N_buckets, sizeof(*bucket_count)); //bucket_count keep tracks of how many elements are in each bucket

    // calc bucket count first
    for (int i = 0; i < N_list; i++){
        int bucket_index = (int)((list[i]-min_element)*temp);
        bucket_count[bucket_index]++;
    }


    for(int i = 0; i < N_buckets; i++){
        buckets[i] = malloc(bucket_count[i]*sizeof(*list)); // allocate only necessary memory!
        //reset bucket count
        bucket_count[i] = 0;
    }

    createtime = omp_get_wtime();
    

    // SORT ELEMENTS INTO BUCKETS ---------------------------------------------------------------------------------

    

    for (int i = 0; i < N_list; i++){
        int bucket_index = (int)((list[i]-min_element)*temp);
        buckets[bucket_index][bucket_count[bucket_index]] = list[i];
        bucket_count[bucket_index]++;
    }

    disttime = omp_get_wtime();
    

    
    // print out buckets to check work load balance
    /*
    for (int i = 0; i < N_buckets; i++)
    {
        printf("BUCKET #%d, Nr of elements: %d\n", i, bucket_count[i]);
    }
    */
    

    // SORT BUCKETS LOCALLY --------------------------------------------------------------------------------

    // some factor to decide wether to quicksort or insertion sort
    //int factor = (int)(N_list/N_buckets);
    int factor = 10; // seems like a good value
    int quicksorted = 0;

#pragma omp parallel for
    for (int i = 0; i < N_buckets; i++){

        // choose local sorting method depending on bucket size
        if(bucket_count[i] >= factor){
            quicksorted++;
            quicksort(buckets[i], 0, bucket_count[i]-1);
        }
        else{
            insertionsort(buckets[i], bucket_count[i]);
        }
    }
    printf("Used quicksort for %d buckets\n", quicksorted);

    sorttime = omp_get_wtime();
    
    


    // PUT ELEMENTS FROM BUCKET BACK INTO LIST IN ORDER --------------------------------------------------
    // this is parallelizable but need to remove list_index

    int list_index = 0;
    for (int i = 0; i < N_buckets; i++){
        for (int j = 0; j < bucket_count[i]; j++){
            list[list_index] = buckets[i][j];
            list_index++;
        }
        
    }

    putbacktime = omp_get_wtime();
    

    //end of sorting time

    endtime = omp_get_wtime();


    printf("TIME: Total: %lf s, Bucket creation: %lf s, Bucket Dist: %lf s, Local sorting: %lf s, Put back: %lf s \n", endtime - starttime, createtime - starttime, disttime - createtime, sorttime - disttime, putbacktime - sorttime);


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




}