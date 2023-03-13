#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#ifdef _OPENMP
    #include <omp.h>
#endif
// Prints out the list to the N:th element.
void print_list(int* list, int N){
    for (int i = 0; i < N; i++)
    {
        printf("%d ", list[i]);
    }
    printf("\n");  
}

void fill_list(int *list, int N, char *dist){
    srand(time(NULL));

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
}

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



int main(int argc, char *argv[]){
    if(argc != 5){
        printf("Error: Excpected 4 arguments...\nUsage: %s N_list N_buckets N_threads Dist_type\n", argv[0]);
        return -1;
    }

    int N_list = atoi(argv[1]);
    int N_buckets = atoi(argv[2]);
    int N_threads = atoi(argv[3]);
    char *dist_type = argv[4];

    #ifdef _OPENMP
        printf("openmp found, running parallel\n");
        omp_set_num_threads(N_threads);
    #else
        printf("openmp not found, running serial\n");
    #endif

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
    print_to_file("unsorted.txt", list, N_list);

    // begin timing algorithm

    double starttime, endtime, createtime, disttime, sorttime, putbacktime;
    starttime = omp_get_wtime();

    // CREATE BUCKETS -----------------------------------------------------------------------------------------------

    int **buckets = malloc(N_buckets*sizeof(*buckets)); //buckets is array of int pointer pointers
    int *bucket_count = malloc(N_buckets*sizeof(*bucket_count)); //bucket_count keep tracks of how many elements are in each bucket
    for(int i = 0; i < N_buckets; i++){
        buckets[i] = malloc(N_list*sizeof(*list)); // memory to fit entire list allocated in each bucket

        //initialise the arrays to avoid segmentation error
        for (int j = 0; j < N_list; j++){
            buckets[i][j] = 0;
        }
        bucket_count[i] = 0;
    }

    createtime = omp_get_wtime();
    printf("\nTIME TAKEN FOR CREATING BUCKETS: %lf seconds.\n\n", createtime - starttime);

    // SORT ELEMENTS INTO BUCKETS ---------------------------------------------------------------------------------

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

    disttime = omp_get_wtime();
    printf("\nTIME TAKEN FOR BUCKET DISTRIBUTION: %lf seconds.\n\n", disttime - createtime);

    /*
    // print out buckets to check work load balance

    for (int i = 0; i < N_buckets; i++){
        printf("BUCKET #%d: ", i);
        print_list(buckets[i], bucket_count[i]);
    }
    
    for (int i = 0; i < N_buckets; i++)
    {
        printf("BUCKET #%d, Nr of elements: %d\n", i, bucket_count[i]);
    }*/
    
    

    // SORT BUCKETS LOCALLY --------------------------------------------------------------------------------
    //printf("SORTING: \n");

#pragma omp parallel for
    for (int i = 0; i < N_buckets; i++){
        insertionsort(buckets[i], bucket_count[i]);
    }

    sorttime = omp_get_wtime();
    printf("\nTIME TAKEN FOR SORTING BUCKETS: %lf seconds.\n\n", sorttime - disttime);


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
    printf("\nTIME TAKEN FOR PUTTING BACK INTO LIST: %lf seconds.\n\n", putbacktime - sorttime);

    //end of sorting time

    endtime = omp_get_wtime();
    printf("\nTIME TAKEN IN TOTAL: %lf seconds.\n\n", endtime - starttime);


    //printf("SORTED LIST:\n");
    //print_list(list, N_list);

    // CHECK IF SORTED ----------------------------------------------------------------
    int flag = is_sorted(list, N_list);
    if(flag){
        printf("The list is sorted!\n");
        print_to_file("sorted.txt", list, N_list);
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