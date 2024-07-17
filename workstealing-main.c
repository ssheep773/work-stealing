#include "workstealing.c"

#define N_THREADS 8

deque_t *thread_queues;

atomic_bool done;

int fib_result = 0;
int tids[N_THREADS];
int empty_thread_queues[N_THREADS];

/* 執行 task 並且設有 while loop 是為了執行後續程式*/
void do_work(int id, work_t *work)
{
    while (work){
        printf("worker %d running item %p\n", id, work);
        work = (*(work->code)) (work);
    }
}

void *thread(void *payload)
{
    int id = *(int *) payload;
    deque_t *my_queue = &thread_queues[id];
    while (true) {        
        work_t *work = take(my_queue);
        // printf("Take %p\n",work);
        if (work != EMPTY) {            
            do_work(id, work);            
        } 
        else {
            work_t *stolen = EMPTY;
            int j = rand() % N_THREADS;
            for (int i = j; i < N_THREADS; ++i) {
                if (i == id)
                    continue;
                stolen = steal(&thread_queues[i]);
                if (stolen == ABORT) {
                    i--;
                    continue; /* Try again at the same i */
                } else if (stolen == EMPTY){
                    continue;
                }
                /* Found some work to do */
                break;
            }
            if (stolen == EMPTY) {
                if (atomic_load(&done))
                    break;
                continue;
            } else {
                do_work(id, stolen);
            }
        }
    }
    printf("worker %d finished\n", id);
    return NULL;
}

/* print task 的後繼程式 ，用於檢察下一個任務是否 ready */ 
work_t *join_work(work_t *work)
{
    int old_join_count = atomic_fetch_sub(&work->join_count, 1);
    if (old_join_count == 1)
        return work;
    
    return NULL;
}

work_t *sum(work_t *w);
work_t *done_task(work_t *w);


work_t *fib(work_t *w)
{
    work_t *cont = (work_t *) w->args[0]; 
    int *n = (int *) w->args[1];       
    
    if (*n < 2) {
        work_t *child_sum1 = malloc(sizeof(work_t) + 2 * sizeof(int *));
        child_sum1->code = &sum;
        child_sum1->join_count = 1;      
        int *result = malloc(sizeof(int));
        *result = 1;  
        child_sum1->args[0] = cont;
        child_sum1->args[1] = result;
        free(n);
        free(w);
        return join_work(child_sum1);
    }
    else{
        work_t *child_sum2 = malloc(sizeof(work_t) + 2 * sizeof(int *));
        child_sum2->code = &sum;         
        child_sum2->join_count = 2;      
        int *result = malloc(sizeof(int));
        *result=0;
        child_sum2->args[0] = cont;   
        child_sum2->args[1] = result;  

        /* spawn x */
        work_t *fibwork1 = malloc(sizeof(work_t) + 2 * sizeof(int *));
        fibwork1->code = &fib;        
        fibwork1->join_count = 0;      
        int *n1 = malloc(sizeof(int));
        *n1 = *n - 1;
        fibwork1->args[0] = child_sum2;
        fibwork1->args[1] = n1; 

        int j = rand() % N_THREADS;
        push(&thread_queues[j], fibwork1);

        /* spawn y */
        work_t *fibwork2 = malloc(sizeof(work_t) + 2 * sizeof(int *));
        fibwork2->code = &fib;       
        fibwork2->join_count = 0;      
        int *n2 = malloc(sizeof(int));
        *n2 = *n - 2;
        fibwork2->args[0] = child_sum2 ;
        fibwork2->args[1] = n2;     

        j = rand() % N_THREADS;       
        push(&thread_queues[j], fibwork2);

        printf("Did item %p => fib(%d), spawn %p and %p -> cont is sum() %p \n", w, *n, fibwork1, fibwork2, child_sum2);
        free(n); // 釋放 n
        free(w);
        return NULL;
    }
}

work_t *sum(work_t *w)
{
    work_t *sum_cont = (work_t *) w->args[0];
    int *result = (int *)sum_cont->args[1];

    int *cur_result = (int *)w->args[1];

    if (*result == -1){
        sum_cont->args[1] = cur_result;
        return done_task(sum_cont);
    }
    else{
        int before_result = *(int *)sum_cont->args[1];
        // int *result = (int *)sum_cont->args[1];
        *result = *result + *cur_result ;

        printf("Did item %p => sum(): %d + %d = %d -> cont is sum() %p\n", w, before_result, *cur_result, *result ,sum_cont);
        free(cur_result);
        free(w);
        return join_work(sum_cont);
    }

}

work_t *done_task(work_t *w)
{
    fib_result = *(int *)w->args[1];
    printf("Did item %p => done task Fib = %d \n", w, *(int *)w->args[1]);
    free(w->args[1]);
    free(w);
    atomic_store(&done, true);
    return NULL;
}

int main(int argc, char **argv)
{
    /* Check that top and bottom are 64-bit so they never overflow */
    static_assert(sizeof(atomic_size_t) == 8,
                  "Assume atomic_size_t is 8 byte wide");

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <fib_num>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int fib_num = atoi(argv[1]) - 1;
    if (fib_num < 0) {
        fprintf(stderr, "fib_num must be a non-negative integer\n");
        return EXIT_FAILURE;
    }

    pthread_t threads[N_THREADS];
    int tids[N_THREADS];
    thread_queues = malloc(N_THREADS * sizeof(deque_t));    
    atomic_store(&done, false);
    
    for (int i = 0; i < N_THREADS; ++i) {
        tids[i] = i;
        init(&thread_queues[i], 8);
    }

    work_t *done_work = malloc(sizeof(work_t) + 2 * sizeof(int *));
    done_work->code = &done_task;         
    done_work->join_count = 1;      
    int *final_result = malloc(sizeof(int));
    *final_result = -1;
    done_work->args[0] = NULL;   
    done_work->args[1] = final_result;   
        
    work_t *work = malloc(sizeof(work_t) + 2 * sizeof(int *));
    work->code = &fib;
    work->join_count = 0;
    int *n = malloc(sizeof(int));
    *n = fib_num;
    work->args[0] = done_work;              
    work->args[1] = n;              
    printf("done task item %p\n", done_work);
    push(&thread_queues[0], work);
    
    for (int i = 0; i < N_THREADS; ++i) {
        if (pthread_create(&threads[i], NULL, thread, &tids[i]) != 0) {
            perror("Failed to start the thread");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < N_THREADS; ++i) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Failed to join the thread");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < N_THREADS; ++i) {
        printf("workqueue %d size %zu\n", i, atomic_load(&thread_queues[i].array->size));
        free(thread_queues[i].array); // 釋放隊列的內存
    }
    free(thread_queues); // 釋放隊列數組
    printf("Finish task Fib(%d) = %d \n",fib_num+1, fib_result);

    return 0;
}

