#include "matrix.h"

int check_args (int argc, char* argv[], int* err_num) {
    if(argc != 2) {
        *err_num = -incorrect_args;
	    return 0;
	}
	
	char* num_end;
	int biggest = strtol(argv[1], &num_end, 0);
	if (errno == ERANGE) {
        *err_num = -incorrect_args;
	    return 0;
	}

	if (num_end != NULL && *num_end) {
        *err_num = -incorrect_args;
	    return 0;
	}
	
	if (biggest <= 0) {
        *err_num = -incorrect_args;
	    return 0;
	}

    return biggest;
}

void print_error (int err_num) {
    if (err_num == ok) return;
    fprintf (stderr, "Error has occured during programm execution!\n");

    switch (err_num) {
        case -incorrect_args:
            fprintf (stderr, "Incorrect arguments!\n\n");
            break;

        case -bad_alloc:
            fprintf (stderr, "Couldn't allocate the memory!\n\n");
            break;

        case -bad_join:
            fprintf (stderr, "Couldn't execute pthread_join!\n\n");
            break;
        
        case -bad_cr_thread:
            fprintf (stderr, "Couldn't create thread!\n\n");
            break;

        case -bad_set:
            fprintf (stderr, "Set not valid!\n\n");
            break;

        default:
            fprintf(stderr, "Undefined error!\n\n");
            break;
    }

    return;
}

int empty_threads_create (int empty_threads_quant, pthread_t** empty_thread, int threads_quant) {
    if (empty_threads_quant <= 0) return 0;
    *empty_thread = (pthread_t*) calloc (empty_threads_quant, sizeof (pthread_t));
    if (*empty_thread == NULL) return -bad_alloc;

    Thread_info* empty_threads_info = (Thread_info*) calloc (empty_threads_quant, sizeof (Thread_info));
    if (empty_threads_info == NULL) return -bad_alloc;

    for (int i = 0; i < empty_threads_quant; i++) {
        empty_threads_info[i].thread_num = i;
        empty_threads_info[i].m = 0;

        if (pthread_create ((*empty_thread) + i, NULL, start_thread, empty_threads_info + i) != 0)
            return -bad_cr_thread;
    }

    return empty_threads_quant;
}

void* start_thread (void* data) {
    Thread_info* thread_info = (Thread_info*) data;
    cpu_set_t cpu_set;

    CPU_ZERO (&cpu_set);
    CPU_SET (thread_info -> thread_num, &cpu_set);

    pthread_setaffinity_np (pthread_self (), sizeof (cpu_set), &cpu_set);

    thread_info -> res = count_x (thread_info);

    return NULL;
}

int create_threads (int thread_quant, pthread_t* thread, Thread_info* threads_info, Matr_info* matr_data) {
    int CPU_quant = get_nprocs ();

    pthread_t* empty_threads = NULL;
    int empty_threads_quant = empty_threads_create (CPU_quant - thread_quant, &empty_threads, thread_quant);
    if (empty_threads < 0) return empty_threads_quant;

    for (int i = 0; i < thread_quant; i++) {
        threads_info[i].thread_num = (empty_threads_quant + i) % CPU_quant;
        fill_thread_info (matr_data, threads_info[i].data);
        threads_info[i].m = matr_data -> m;
        threads_info[i].opr_num = i;

        if (pthread_create (thread + i, NULL, start_thread, threads_info + i) != 0) return -bad_cr_thread;
    }

    for (int i = 0; i < empty_threads_quant; i++) {
        if (pthread_join (empty_threads[i], NULL) != 0) return -bad_join;
    }

    return thread_quant;
}

void fill_thread_info (Matr_info* matr_data, int* data) {
    if (matr_data -> m * matr_data -> m + matr_data -> m > (PAGE_SIZE - NON_DATA_SIZE) / sizeof (int)) {
        fprintf (stderr, "Too big size for matrix!\n");
        exit (1);
    }
    for (int i = 0; i < matr_data -> m; i++) {
        data[i] = matr_data -> array[i];
    }
    for (int i = 0; i < matr_data ->m * matr_data -> m; i++) {
        data[i + matr_data -> m] = matr_data -> matr[i];
    }
}

int count_res (int threads_quant, pthread_t* thread, Thread_info* threads_info, double* res) {
    for (int i = 0; i < threads_quant; i++) {
        if (pthread_join (thread[i], NULL) != 0) return -bad_join;

        res[i] += threads_info[i].res;
    }

    return 0;
}

double count_x (Thread_info* thread_info) {
    double res = 0;

    int* matr = (int*) calloc (thread_info->m*thread_info->m, sizeof(int));
    for (int i = 0; i < thread_info ->m * thread_info -> m; i++) {
        matr[i] = thread_info -> data[i + thread_info -> m];
    }
    //print_matr (matr, thread_info -> m, thread_info -> opr_num);

    res = count_matr (matr, thread_info -> m);

    for (int i = 0; i < thread_info ->m * thread_info -> m; i++) {
        matr[i] = thread_info -> data[i + thread_info -> m];
    }
    for (int i = 0; i < thread_info -> m; i++) {
        matr[i*thread_info -> m + thread_info -> opr_num] = thread_info->data[i];
    }
    //print_matr (matr, thread_info -> m, thread_info -> opr_num);

    res = count_matr (matr, thread_info -> m) / res;

    return res;
}

double count_matr (int* matr, int m) {
    double res = 0;
    double multi = 1;

    //print_matr (matr, m, 0);
    //printf ("Pointer to matr in count_matr: %p\n", matr);


    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) {
            multi *= matr[((i + j) % 3) * m + j];
            //printf ("%d", matr[((i + j) % 3) * m + j]);
            //if (j != m - 1) printf (" * ");
        }
        //if (i != m - 1) printf (" + \n");

        res += multi;
        //printf ("   cur res: %f\n", multi);
        multi = 1;
    }
    //printf (" - ");

    for (int i = m - 1; i >= 0; i--) {
        for (int j = 0; j < m; j++) {
            multi *= matr[((i + j*2) % 3) * m + j];
            //printf ("%d", matr[((i + j*2) % 3) * m + j]);
            //if (j != m - 1) printf (" * ");
        }
        //if (i != 0) printf (" - \n");

        res -= multi;
        //printf ("cur res: %f\n", multi);
        multi = 1;
    }
    //printf ("\n");
    //printf ("Res: %f", res);

    return res;
}

void print_matr (int* matr, int m, int opr_num) {
    //printf ("Opr_num is: %d", opr_num);
    for (int i = 0; i < m*m; i++) {
        if (i % m == 0) printf ("\n");
        //printf ("Opr_num is: %d - ", opr_num);
        printf ("%d ", matr[i]);
    }
    printf ("\n");
}

void fill_Matr_info (int m, Matr_info* matr_data) {
    matr_data -> m = m;

    printf ("Input matr[m][m]:\n");
    matr_data -> matr = (int*) calloc (m*m, sizeof (int));
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) {
            //matr_data -> matr[i*m + j] = i*j + i;
            scanf ("%d", matr_data -> matr + i*m + j);
        }
    }

    //print_matr (matr_data -> matr, m, 0);

    printf ("Input array[m]:\n");
    matr_data -> array = (int*) calloc (m, sizeof (int));
    for (int i = 0; i < m; i++) {
        //matr_data -> array[i] = i*i;
        scanf ("%d", &matr_data -> array[i]);
    }
}

int main (int argc, char* argv[]) {
    int thread_quant = 0;
    int m = 0;
    printf ("Input matrix [m*m]. Input m:");
    scanf ("%d", &m);
    
    if (m <= 0) {
        printf ("Incorrect size!\n");
        return 1;
    }

    Matr_info* matr_data = (Matr_info*) calloc (1, sizeof (Matr_info));
    fill_Matr_info (m, matr_data);    
    //print_matr (matr_data -> matr, m, 0);

    int err_num = 0;

    thread_quant = m;

    pthread_t* threads = (pthread_t*) calloc (thread_quant, sizeof (pthread_t));
    if (threads == NULL) {
        print_error (bad_alloc);
        return -bad_alloc;
    }

    Thread_info* threads_info = (Thread_info*) calloc (thread_quant, sizeof (Thread_info));
    if (threads_info == NULL) {
        print_error (bad_alloc);
        return -bad_alloc;
    }

    err_num = create_threads (thread_quant, threads, threads_info, matr_data);
    if (err_num != thread_quant) {
        print_error (err_num);
        return err_num;
    }

    double* res = (double*) calloc (m, sizeof (double));
    
    err_num = count_res (thread_quant, threads, threads_info, res);
    if (err_num == -bad_join) {
        print_error (-bad_join);
        return -bad_join;
    }

    printf ("Res is:\n");
    for (int i = 0; i < m; i++) {
        printf ("%f\n", res[i]);
    }
    

    free (threads);
    free (threads_info);
    free (matr_data -> matr);
    free (matr_data -> array);
    free (res);

    //printf ("Pointer to matr in main: %p\n", matr_data -> matr);
    //count_matr (matr_data -> matr, m);

    //free (matr_data -> matr);
    //free (matr_data -> array);

    return 0;
}