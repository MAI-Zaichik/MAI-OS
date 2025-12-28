#include <fcntl.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#define MAX_DATA_SIZE 4096

typedef struct {
    char buffer[MAX_DATA_SIZE];
    int data_size;
    int finished;
} shared_data_t;

void read_filename(char* buf, size_t size) {
    char c;
    int i = 0;
    while (i < size - 1) {
        ssize_t result = read(STDIN_FILENO, &c, 1);
        if (result <= 0) {
            const char msg[] = "error: failed to read from stdin\n";
            write(STDERR_FILENO, msg, sizeof(msg));
            exit(EXIT_FAILURE);
        }
        if (c == '\n') break;
        buf[i] = c;
        i++;
    }
    if (!i) {
        const char msg[] = "error: invalid file name\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    buf[i] = 0;
}

int main() {
    const char CHILD_PROGRAM_NAME[] = "child";
    const char OUTPUT_FOLDER_NAME[] = "out";
    pid_t child1;
    pid_t child2;
    char main_binary_path[1024];
    char output_path[1024];
    
    srand(time(NULL));
    int rand_num = rand() % 10000;
    
    char shm_name[64];
    char sem_child1_name[64];
    char sem_child2_name[64];
    char sem_empty_name[64];
    
    snprintf(shm_name, sizeof(shm_name), "/shm%d", rand_num);
    snprintf(sem_child1_name, sizeof(sem_child1_name), "/sem1%d", rand_num);
    snprintf(sem_child2_name, sizeof(sem_child2_name), "/sem2%d", rand_num);
    snprintf(sem_empty_name, sizeof(sem_empty_name), "/sem_empty%d", rand_num);
    
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0600);
    if (shm_fd == -1) {
        const char msg[] = "error: failed to create shared memory\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    
    if (ftruncate(shm_fd, sizeof(shared_data_t)) == -1) {
        const char msg[] = "error: failed to resize shared memory\n";
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
    
    memset(shared_data, 0, sizeof(shared_data_t));
    
    sem_t* sem_child1 = sem_open(sem_child1_name, O_CREAT | O_RDWR, 0600, 0);
    if (sem_child1 == SEM_FAILED) {
        const char msg[] = "error: failed to create semaphore for child1\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    
    sem_t* sem_child2 = sem_open(sem_child2_name, O_CREAT | O_RDWR, 0600, 0);
    if (sem_child2 == SEM_FAILED) {
        const char msg[] = "error: failed to create semaphore for child2\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    
    sem_t* sem_empty = sem_open(sem_empty_name, O_CREAT | O_RDWR, 0600, 1);
    if (sem_empty == SEM_FAILED) {
        const char msg[] = "error: failed to create empty semaphore\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    
    char exe_path[1024];
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (len == -1) {
        const char msg[] = "error: failed to read full program path\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    
    while (exe_path[len] != '/') --len;
    exe_path[len] = '\0';
    snprintf(main_binary_path, sizeof(main_binary_path) - 1, "%s/%s",
             exe_path, CHILD_PROGRAM_NAME);
    
    while (exe_path[len] != '/') --len;
    exe_path[len] = '\0';
    snprintf(output_path, sizeof(output_path) - 1, "%s/%s", exe_path,
             OUTPUT_FOLDER_NAME);
    
    char buf[4096];
    ssize_t bytes_read;
    
    bytes_read = read(STDIN_FILENO, buf, sizeof(buf));
    if (bytes_read < 0) {
        const char msg[] = "error: failed to read from stdin\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    } else if (buf[0] == '\n' || bytes_read == 0) {
        const char msg[] = "error: invalid first file name\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    
    char file_path[1024];
    buf[bytes_read - 1] = '\0';
    snprintf(file_path, sizeof(file_path) - 1, "%s/%s", output_path, buf);
    int32_t file1 = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (file1 == -1) {
        const char msg[] = "error: failed to open first file\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    
    bytes_read = read(STDIN_FILENO, buf, sizeof(buf));
    if (bytes_read < 0) {
        const char msg[] = "error: failed to read from stdin\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    } else if (buf[0] == '\n') {
        const char msg[] = "error: invalid second file name\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    
    buf[bytes_read - 1] = '\0';
    snprintf(file_path, sizeof(file_path) - 1, "%s/%s", output_path, buf);
    int32_t file2 = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (file2 == -1) {
        const char msg[] = "error: failed to open second file\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    
    child1 = fork();
    if (child1 == -1) {
        const char msg[] = "error: failed to spawn new process\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    if (child1 == 0) {
        dup2(file1, STDOUT_FILENO);
        close(file1);
        
        char shm_name_str[256];
        char sem_data_str[256];
        char sem_empty_str[256];
        snprintf(shm_name_str, sizeof(shm_name_str), "SHM_NAME=%s", shm_name);
        snprintf(sem_data_str, sizeof(sem_data_str), "SEM_DATA=%s", sem_child1_name);
        snprintf(sem_empty_str, sizeof(sem_empty_str), "SEM_EMPTY=%s", sem_empty_name);
        
        char* const args[] = {"child", shm_name_str, sem_data_str,
                              sem_empty_str, "1", NULL};
        execv(main_binary_path, args);
        
        const char msg[] = "error: failed to execv\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    
    child2 = fork();
    if (child2 == -1) {
        const char msg[] = "error: failed to spawn new process\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    if (child2 == 0) {
        dup2(file2, STDOUT_FILENO);
        close(file2);
        
        char shm_name_str[256];
        char sem_data_str[256];
        char sem_empty_str[256];
        snprintf(shm_name_str, sizeof(shm_name_str), "SHM_NAME=%s", shm_name);
        snprintf(sem_data_str, sizeof(sem_data_str), "SEM_DATA=%s", sem_child2_name);
        snprintf(sem_empty_str, sizeof(sem_empty_str), "SEM_EMPTY=%s", sem_empty_name);
        
        char* const args[] = {"child", shm_name_str, sem_data_str,
                              sem_empty_str, "2", NULL};
        execv(main_binary_path, args);
        
        const char msg[] = "error: failed to execv\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    
    while (bytes_read = read(STDIN_FILENO, buf, sizeof(buf))) {
        if (bytes_read < 0) {
            const char msg[] = "error: failed to read from stdin\n";
            write(STDERR_FILENO, msg, sizeof(msg));
            exit(EXIT_FAILURE);
        } else if (buf[0] == '\n') {
            break;
        }
        
        sem_wait(sem_empty);
        memcpy(shared_data->buffer, buf, bytes_read);
        shared_data->data_size = bytes_read;
        
        if (bytes_read > 11) {
            sem_post(sem_child2);
        } else {
            sem_post(sem_child1);
        }
    }
    
    shared_data->finished = 1;
    sem_post(sem_child1);
    sem_post(sem_child2);
    
    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);
    
    munmap(shared_data, sizeof(shared_data_t));
    close(shm_fd);
    shm_unlink(shm_name);
    
    sem_close(sem_child1);
    sem_close(sem_child2);
    sem_close(sem_empty);
    
    sem_unlink(sem_child1_name);
    sem_unlink(sem_child2_name);
    sem_unlink(sem_empty_name);
    
    return 0;
}