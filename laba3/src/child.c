#include <fcntl.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define MAX_DATA_SIZE 4096

typedef struct {
    char buffer[MAX_DATA_SIZE];
    int data_size;
    int finished;
} shared_data_t;

int main(int argc, char* argv[]) {
    if (argc < 5) {
        const char msg[] = "error: insufficient arguments\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    
    char* shm_name = NULL;
    char* sem_data_name = NULL;
    char* sem_empty_name = NULL;
    int my_num = atoi(argv[4]);
    
    for (int i = 1; i < 4; i++) {
        if (!strncmp(argv[i], "SHM_NAME=", 9)) {
            shm_name = argv[i] + 9;
        } else if (!strncmp(argv[i], "SEM_DATA=", 9)) {
            sem_data_name = argv[i] + 9;
        } else if (!strncmp(argv[i], "SEM_EMPTY=", 10)) {
            sem_empty_name = argv[i] + 10;
        }
    }
    
    if (!shm_name || !sem_data_name || !sem_empty_name) {
        const char msg[] = "error: missing required arguments\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    
    int shm_fd = shm_open(shm_name, O_RDWR, 0600);
    if (shm_fd == -1) {
        const char msg[] = "error: failed to open shared memory\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    
    shared_data_t* shared_data = (shared_data_t*)mmap(
        NULL, sizeof(shared_data_t),
        PROT_READ | PROT_WRITE,
        MAP_SHARED, shm_fd, 0
    );
    
    if (shared_data == MAP_FAILED) {
        const char msg[] = "error: failed to map shared memory\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    
    sem_t* sem_data = sem_open(sem_data_name, O_RDWR);
    if (sem_data == SEM_FAILED) {
        const char msg[] = "error: failed to open data semaphore\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    
    sem_t* sem_empty = sem_open(sem_empty_name, O_RDWR);
    if (sem_empty == SEM_FAILED) {
        const char msg[] = "error: failed to open empty semaphore\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    
    char buf[MAX_DATA_SIZE];
    ssize_t bytes_read;
    int32_t written;
    int32_t left;
    int32_t right;
    
    while (!shared_data->finished) {
        sem_wait(sem_data);
        
        if (shared_data->finished) {
            break;
        }
        
        memcpy(buf, shared_data->buffer, shared_data->data_size);
        bytes_read = shared_data->data_size;
        
        left = 0;
        right = bytes_read - 2;
        while (left < right) {
            char tmp = buf[left];
            buf[left] = buf[right];
            buf[right] = tmp;
            ++left;
            --right;
        }
        
        written = write(STDOUT_FILENO, buf, bytes_read);
        if (written != bytes_read) {
            const char msg[] = "error: failed to echo\n";
            write(STDERR_FILENO, msg, sizeof(msg));
            exit(EXIT_FAILURE);
        }
        
        sem_post(sem_empty);
    }
    
    munmap(shared_data, sizeof(shared_data_t));
    close(shm_fd);
    sem_close(sem_data);
    sem_close(sem_empty);
    
    return 0;
}