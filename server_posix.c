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
    // Сегмент разделяемой памяти
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    ftruncate(shm_fd, SIZE);
    ptr = mmap(0, SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    printf("[SERVER] Сегмент создан. Жду подключения клиента...\n");
    sleep(2); 
    sprintf(ptr, "Hi!");
    printf("[SERVER] Отправлено: %s\n", ptr);
    while (strncmp(ptr, "Hi!", 3) == 0) {
        sleep(1);
    }
    printf("[SERVER] Получено: %s\n", ptr);
    
    munmap(ptr, SIZE);
    close(shm_fd);
    shm_unlink(SHM_NAME);
    return 0;
}
