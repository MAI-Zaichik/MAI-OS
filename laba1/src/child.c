#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    char buffer[4096];
    ssize_t bytesRead;
    int32_t bytesWritten;
    int32_t leftIdx;
    int32_t rightIdx;

    while ((bytesRead = read(STDIN_FILENO, buffer, sizeof(buffer)))) {
        if (bytesRead < 0) {
            const char errMsg[] = "child error: failed to read from stdin\n";
            write(STDERR_FILENO, errMsg, sizeof(errMsg));
            exit(EXIT_FAILURE);
        }
        leftIdx = 0;
        rightIdx = bytesRead - 2;
        while (leftIdx < rightIdx) {
            char temp = buffer[leftIdx];
            buffer[leftIdx] = buffer[rightIdx];
            buffer[rightIdx] = temp;
            ++leftIdx;
            --rightIdx;
        }
        bytesWritten = write(STDOUT_FILENO, buffer, bytesRead);
        if (bytesWritten != bytesRead) {
            const char errMsg[] = "child error: failed to echo\n";
            write(STDERR_FILENO, errMsg, sizeof(errMsg));
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}