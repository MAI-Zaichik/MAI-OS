#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

#define TOTAL_POINTS 100

typedef struct {
    int thread_id;
    int max_threads;
    long long points_per_thread;
    double radius;
    unsigned int seed;
    long long hits;
} ThreadArgs;

static void *work(void *_args) {
    ThreadArgs *args = (ThreadArgs*)_args;
    long long local_hits = 0;
    
    for (long long i = 0; i < args->points_per_thread; i++) {
        double x = (double)rand_r(&args->seed) / RAND_MAX * 2 * args->radius - args->radius;
        double y = (double)rand_r(&args->seed) / RAND_MAX * 2 * args->radius - args->radius;
        
        if (x*x + y*y <= args->radius * args->radius) {
            local_hits++;
        }
    }
    
    args->hits = local_hits;
    return NULL;
}

static double get_time_diff(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        const char msg[] = "Usage: ./circle_area <radius> <max_threads>\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    double radius = atof(argv[1]);
    int max_threads = atoi(argv[2]);

    if (radius <= 0 || max_threads <= 0) {
        const char msg[] = "error: radius and max_threads must be positive\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    char buf[256];

    // Последовательно
    struct timespec start_seq, end_seq;
    if (clock_gettime(CLOCK_MONOTONIC, &start_seq) != 0) {
        const char msg[] = "error: cant get start time\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    unsigned int seed_seq = (unsigned int)time(NULL);
    long long hits_seq = 0;
    
    for (long long i = 0; i < TOTAL_POINTS; i++) {
        double x = (double)rand_r(&seed_seq) / RAND_MAX * 2 * radius - radius;
        double y = (double)rand_r(&seed_seq) / RAND_MAX * 2 * radius - radius;
        
        if (x*x + y*y <= radius * radius) {
            hits_seq++;
        }
    }
    
    double square_area = 4 * radius * radius;
    double seq_area = (double)hits_seq / TOTAL_POINTS * square_area;

    if (clock_gettime(CLOCK_MONOTONIC, &end_seq) != 0) {
        const char msg[] = "error: cant get end time\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    double seq_time = get_time_diff(start_seq, end_seq);
    double exact_area = M_PI * radius * radius;

    snprintf(buf, sizeof(buf), "Posledovatelno:\n");
    write(STDOUT_FILENO, buf, strlen(buf));
    snprintf(buf, sizeof(buf), "Time: %.6f sec\n", seq_time);
    write(STDOUT_FILENO, buf, strlen(buf));


    // Паралелльно
    struct timespec start_par, end_par;
    if (clock_gettime(CLOCK_MONOTONIC, &start_par) != 0) {
        const char msg[] = "error: cant get start time\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    long long points_per_thread = TOTAL_POINTS / max_threads;
    long long extra_points = TOTAL_POINTS % max_threads;

    pthread_t *threads = (pthread_t*)malloc(max_threads * sizeof(pthread_t));
    ThreadArgs *thread_args = (ThreadArgs*)malloc(max_threads * sizeof(ThreadArgs));

    for (int i = 0; i < max_threads; ++i) {
        thread_args[i] = (ThreadArgs){
            .thread_id = i,
            .max_threads = max_threads,
            .points_per_thread = points_per_thread,
            .radius = radius,
            .seed = (unsigned int)(time(NULL) ^ (i * 12345)),
            .hits = 0,
        };
        
        if (i == 0) {
            thread_args[i].points_per_thread += extra_points;
        }
        
        pthread_create(&threads[i], NULL, work, &thread_args[i]);
    }

    for (int i = 0; i < max_threads; ++i) {
        pthread_join(threads[i], NULL);
    }

    if (clock_gettime(CLOCK_MONOTONIC, &end_par) != 0) {
        const char msg[] = "error: cant get end time\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    double par_time = get_time_diff(start_par, end_par);
    
    long long total_hits = 0;
    for (int i = 0; i < max_threads; ++i) {
        total_hits += thread_args[i].hits;
    }
    
    double par_area = (double)total_hits / TOTAL_POINTS * square_area;


    snprintf(buf, sizeof(buf), "Parallelno:\n");
    write(STDOUT_FILENO, buf, strlen(buf));
    
    snprintf(buf, sizeof(buf), "Threads: %d\n", max_threads);
    write(STDOUT_FILENO, buf, strlen(buf));
    
    snprintf(buf, sizeof(buf), "Time: %.6f sec\n", par_time);
    write(STDOUT_FILENO, buf, strlen(buf));

    
    free(thread_args);
    free(threads);

    return 0;
}