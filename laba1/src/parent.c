#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    const char CHILD_PROG_NAME[] = "child";
    const char OUT_DIR_NAME[] = "out";
    int pipeFirst[2];
    int pipeSecond[2];
    pid_t pidChild1;
    pid_t pidChild2;
    char childProgPath[1024];
    char outputDirPath[1024];

    {
        char exePath[1024];
        ssize_t pathLen = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
        if (pathLen == -1) {
            const char errMsg[] = "parent error: cannot get program path\n";
            write(STDERR_FILENO, errMsg, sizeof(errMsg));
            exit(EXIT_FAILURE);
        }

        while (exePath[pathLen] != '/') --pathLen;
        exePath[pathLen] = '\0';
        snprintf(childProgPath, sizeof(childProgPath) - 1, "%s/%s",
                 exePath, CHILD_PROG_NAME);

        while (exePath[pathLen] != '/') --pathLen;
        exePath[pathLen] = '\0';
        snprintf(outputDirPath, sizeof(outputDirPath) - 1, "%s/%s", exePath,
                 OUT_DIR_NAME);
    }

    if (pipe(pipeFirst) == -1) {
        const char errMsg[] = "parent error: failed to create first pipe\n";
        write(STDERR_FILENO, errMsg, sizeof(errMsg));
        exit(EXIT_FAILURE);
    }

    if (pipe(pipeSecond) == -1) {
        const char errMsg[] = "parent error: failed to create second pipe\n";
        write(STDERR_FILENO, errMsg, sizeof(errMsg));
        exit(EXIT_FAILURE);
    }

    char inputBuffer[4096];
    ssize_t bytesRead;

    bytesRead = read(STDIN_FILENO, inputBuffer, sizeof(inputBuffer));
    if (bytesRead < 0) {
        const char errMsg[] = "parent error: failed to read from stdin\n";
        write(STDERR_FILENO, errMsg, sizeof(errMsg));
        exit(EXIT_FAILURE);
    } else if (inputBuffer[0] == '\n' || bytesRead == 0) {
        const char errMsg[] = "parent error: invalid first file name\n";
        write(STDERR_FILENO, errMsg, sizeof(errMsg));
        exit(EXIT_FAILURE);
    }

    char filePath[1024];
    inputBuffer[bytesRead - 1] = '\0';
    snprintf(filePath, sizeof(filePath) - 1, "%s/%s", outputDirPath, inputBuffer);
    int32_t fileDesc1 = open(filePath, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fileDesc1 == -1) {
        const char errMsg[] = "parent error: failed to open first file\n";
        write(STDERR_FILENO, errMsg, sizeof(errMsg));
        exit(EXIT_FAILURE);
    }

    bytesRead = read(STDIN_FILENO, inputBuffer, sizeof(inputBuffer));
    if (bytesRead < 0) {
        const char errMsg[] = "parent error: failed to read from stdin\n";
        write(STDERR_FILENO, errMsg, sizeof(errMsg));
        exit(EXIT_FAILURE);
    } else if (inputBuffer[0] == '\n') {
        const char errMsg[] = "parent error: invalid second file name\n";
        write(STDERR_FILENO, errMsg, sizeof(errMsg));
        exit(EXIT_FAILURE);
    }

    inputBuffer[bytesRead - 1] = '\0';
    snprintf(filePath, sizeof(filePath) - 1, "%s/%s", outputDirPath, inputBuffer);
    int32_t fileDesc2 = open(filePath, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fileDesc2 == -1) {
        const char errMsg[] = "parent error: failed to open second file\n";
        write(STDERR_FILENO, errMsg, sizeof(errMsg));
        exit(EXIT_FAILURE);
    }

    pidChild1 = fork();
    if (pidChild1 == -1) {
        const char errMsg[] = "parent error: failed to spawn first child\n";
        write(STDERR_FILENO, errMsg, sizeof(errMsg));
        exit(EXIT_FAILURE);
    }
    if (pidChild1 == 0) {
        dup2(fileDesc1, STDOUT_FILENO);
        close(fileDesc1);

        close(pipeFirst[1]);
        dup2(pipeFirst[0], STDIN_FILENO);
        close(pipeFirst[0]);
        close(pipeSecond[0]);
        close(pipeSecond[1]);

        char* const args[] = {"child", NULL};
        execv(childProgPath, args);
        const char errMsg[] = "parent error: failed to execv for child1\n";
        write(STDERR_FILENO, errMsg, sizeof(errMsg));
        exit(EXIT_FAILURE);
    }

    pidChild2 = fork();
    if (pidChild2 == -1) {
        const char errMsg[] = "parent error: failed to spawn second child\n";
        write(STDERR_FILENO, errMsg, sizeof(errMsg));
        exit(EXIT_FAILURE);
    }
    if (pidChild2 == 0) {
        dup2(fileDesc2, STDOUT_FILENO);
        close(fileDesc2);

        close(pipeSecond[1]);
        dup2(pipeSecond[0], STDIN_FILENO);
        close(pipeSecond[0]);
        close(pipeFirst[0]);
        close(pipeFirst[1]);

        char* const args[] = {"child", NULL};
        execv(childProgPath, args);
        const char errMsg[] = "parent error: failed to execv for child2\n";
        write(STDERR_FILENO, errMsg, sizeof(errMsg));
        exit(EXIT_FAILURE);
    }

    while (bytesRead = read(STDIN_FILENO, inputBuffer, sizeof(inputBuffer))) {
        if (bytesRead < 0) {
            const char errMsg[] = "parent error: failed to read from stdin\n";
            write(STDERR_FILENO, errMsg, sizeof(errMsg));
            exit(EXIT_FAILURE);
        } else if (inputBuffer[0] == '\n') {
            break;
        }
        if (bytesRead > 11) {
            write(pipeSecond[1], inputBuffer, bytesRead);
        } else {
            write(pipeFirst[1], inputBuffer, bytesRead);
        }
    }
    close(pipeSecond[1]);
    close(pipeFirst[1]);
    waitpid(pidChild1, NULL, 0);
    waitpid(pidChild2, NULL, 0);
    return 0;
}