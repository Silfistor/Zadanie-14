// client_shm_posix.c
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#define SHM_NAME "/chat_shm"
#define SIZE 256

int main() {
    int shm_fd;
    char *ptr;
    sleep(1);
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    ptr = mmap(0, SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    printf("[CLIENT] Получено: %s\n", ptr);
    sprintf(ptr, "Hello!");
    printf("[CLIENT] Ответ отправлен: Hello!\n");
    munmap(ptr, SIZE);
    close(shm_fd);
    return 0;
}
