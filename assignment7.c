#include "assignment7.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>

#define SORT_THRESHOLD      40

typedef struct _sortParams {
    char** array;
    int left;
    int right;
} SortParams;

static int maximumThreads;              /* maximum # of threads to be used */
//int threadNumber;
//pthread_mutex_t threadTex;

/* This is an implementation of insert sort, which although it is */
/* n-squared, is faster at sorting short lists than quick sort,   */
/* due to its lack of recursive procedure call overhead.          */

static void insertSort(char** array, int left, int right) {
    int i, j;
    for (i = left + 1; i <= right; i++) {
        char* pivot = array[i];
        j = i - 1;
        while (j >= left && (strcmp(array[j],pivot) > 0)) {
            array[j + 1] = array[j];
            j--;
        }
        array[j + 1] = pivot;
    }
}

/* Recursive quick sort, but with a provision to use */
/* insert sort when the range gets small.            */

static void* quickSort(void* p) {
    SortParams* params = (SortParams*) p;
    char** array = params->array;
    int left = params->left;
    int right = params->right;
    int i = left, j = right;
    
    if (j - i > SORT_THRESHOLD) {           /* if the sort range is substantial, use quick sort */

        int m = (i + j) >> 1;               /* pick pivot as median of         */
        char* temp, *pivot;                 /* first, last and middle elements */
        if (strcmp(array[i],array[m]) > 0) {
            temp = array[i]; array[i] = array[m]; array[m] = temp;
        }
        if (strcmp(array[m],array[j]) > 0) {
            temp = array[m]; array[m] = array[j]; array[j] = temp;
            if (strcmp(array[i],array[m]) > 0) {
                temp = array[i]; array[i] = array[m]; array[m] = temp;
            }
        }
        pivot = array[m];

        for (;;) {
            while (strcmp(array[i],pivot) < 0) i++; /* move i down to first element greater than or equal to pivot */
            while (strcmp(array[j],pivot) > 0) j--; /* move j up to first element less than or equal to pivot      */
            if (i < j) {
                char* temp = array[i];      /* if i and j have not passed each other */
                array[i++] = array[j];      /* swap their respective elements and    */
                array[j--] = temp;          /* advance both i and j                  */
            } else if (i == j) {
                i++; j--;
            } else break;                   /* if i > j, this partitioning is done  */
        }
        
        if (maximumThreads > 0) {
            maximumThreads--;
            pthread_t thr;
            //threadNumber++;
            SortParams first;  first.array = array; first.left = left; first.right = j;
            if (pthread_create(&thr, NULL, quickSort, (void*)(&first)) != 0) {
                fprintf(stderr, "%s\n", strerror(errno));
                exit(0);
            }
            
            SortParams second; second.array = array; second.left = i; second.right = right;
            quickSort(&second); 

            if (pthread_join(thr, NULL) != 0) {
                fprintf(stderr, "%s\n", strerror(errno));
                exit(0);    
            }
            maximumThreads++;
            //threadNumber--;
        } else {
            SortParams first;  first.array = array; first.left = left; first.right = j;
            quickSort(&first);                  /* sort the left partition  */
            
            SortParams second; second.array = array; second.left = i; second.right = right;
            quickSort(&second);                 /* sort the right partition */
        }      
    } else insertSort(array,i,j);           /* for a small range use insert sort */
    return NULL;  
}

/* user interface routine to set the number of threads sortT is permitted to use */

void setSortThreads(int count) {
    maximumThreads = count;
    /*
    if (pthread_mutex_init(threadTex, NULL) != 0) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(0);
    }
    */
}

/* user callable sort procedure, sorts array of count strings, beginning at address array */

void sortThreaded(char** array, unsigned int count) {
    SortParams parameters;
    parameters.array = array; parameters.left = 0; parameters.right = count - 1;
    quickSort(&parameters);
    /*
    if (pthread_mutex_destroy(threadTex) != 0) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(0);
    }*/
}

/*
int main(int argc, char const *argv[]) {
    if (argc <= 1) {
        printf("Need to pass a file to sort\n");
        return 0;
    }

    FILE* fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("Could not find file: %s\n", argv[1]);
        exit(0);
    }

    int bufSize = 256;
    int arrSize = 21000;

    char buf[bufSize];
    //time_t timeTaken;
    struct timespec start, stop;
    double elapsed_time;
    char** wordArray = calloc(arrSize, sizeof(char*));
    int i = 0;

    //threadNumber = 0;
    setSortThreads(6);

    // insert into array
    while ((fscanf(fp, "%s", buf) > 0) && (i < arrSize)) {
        //printf("%s\n", buf);
        wordArray[i] = calloc(bufSize, sizeof(char));
        strcpy(wordArray[i], buf);
        i++;
    }
    fclose(fp);
    
    printf("Before:\n");
    for (int j = 0; j < i; j++) {
        printf("%s\n", wordArray[j]);
    }
    
    //timeTaken = time(NULL);
    clock_gettime(CLOCK_MONOTONIC, &start);

    // sort array
    sortThreaded(wordArray, i);

    clock_gettime(CLOCK_MONOTONIC, &stop);
    elapsed_time = (stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec)/1000.0;
    //printf("Time Taken: %ld\n", time(NULL) - timeTaken);
    printf("Time Taken: %f\n", elapsed_time);

    
    printf("After:\n");
    for (int j = 0; j < i; j++) {
        printf("%s\n", wordArray[j]);
    }

    for (int j = 0; j < i; j++) {
        free(wordArray[j]);
    }
    free(wordArray);

    return 0;
} */
