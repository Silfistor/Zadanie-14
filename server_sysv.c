#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <unistd.h>

#define KEY 1234
#define SIZE 256

int main() {
    int shmid;
    char *shm_ptr;

    // Сегмент
    shmid = shmget(KEY, SIZE, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }
    shm_ptr = shmat(shmid, NULL, 0);
    if (shm_ptr == (char*)-1) {
        perror("shmat");
        exit(1);
    }

    printf("[SERVER] Сегмент создан. Жду...\n");
    sleep(2);
    sprintf(shm_ptr, "Hi!");
    printf("[SERVER] Отправлено: Hi!\n");
    while (strcmp(shm_ptr, "Hi!") == 0)
        sleep(1);

    printf("[SERVER] Получено: %s\n", shm_ptr);
    shmdt(shm_ptr);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}
