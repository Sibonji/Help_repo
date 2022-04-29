#ifndef INTEGRAL_H
#define INTEGRAL_H

#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include <errno.h>
#include <stdlib.h>
#include <sched.h>
#include <math.h>
#include <string.h>

#define PAGE_SIZE 4096
#define NON_DATA_SIZE 3 * sizeof (int) + sizeof (double)

typedef struct _Thread_info {
    int m;
    int thread_num;
    double res;
    int opr_num;
    int data[(PAGE_SIZE - NON_DATA_SIZE) / sizeof (int)];
} Thread_info;

typedef struct _Matr_info {
    int* matr;
    int* array;
    int m;
} Matr_info;

enum errors {
    ok,

    incorrect_args,
    bad_alloc,
    bad_cr_thread,
    bad_set,
    bad_join
};

int check_args (int argc, char* argv[], int* err_num);
void print_error (int err_num);
int create_threads (int thread_quant, pthread_t* threads, Thread_info* threads_info, Matr_info* matr_data);
int empty_threads_create (int empty_threads_quant, pthread_t** empty_thread, int threads_quant);
void* start_thread (void* data);
int count_res (int threads_quant, pthread_t* thread, Thread_info* threads_info, double* res);
double count_matr (int* matr, int m);
double count_x (Thread_info* thread_info);
void fill_Matr_info (int m, Matr_info* matr_data);
void fill_thread_info (Matr_info* matr_data, int* data);
void print_matr (int* matr, int m, int opr_num);

#endif