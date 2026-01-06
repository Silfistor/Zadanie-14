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

    sleep(1);
    shmid = shmget(KEY, SIZE, 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }
    shm_ptr = shmat(shmid, NULL, 0);
    if (shm_ptr == (char*)-1) {
        perror("shmat");
        exit(1);
    }
    printf("[CLIENT] Получено: %s\n", shm_ptr);
    sprintf(shm_ptr, "Hello!");
    printf("[CLIENT] Отправлено: Hello!\n");

    shmdt(shm_ptr);
    return 0;
}
