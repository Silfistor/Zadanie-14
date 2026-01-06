#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>

#define MAX_TEXT  100
#define MAX_NAME  32
#define MAX_USERS 50
#define MAX_MSGS  200
#define SHM_KEY   0x12345

typedef struct 
{
    pid_t sender;
    char  name[MAX_NAME];
    char  text[MAX_TEXT];
} ChatMessage;

typedef struct 
{
    pthread_mutex_t mutex;
    int  user_count;
    char users[MAX_USERS][MAX_NAME];
    int  msg_count;
    ChatMessage msgs[MAX_MSGS];
} SharedChat;

static int shmid;
static SharedChat *chat;

void cleanup(int sig) 
{
    if (chat) 
    {
        pthread_mutex_destroy(&chat->mutex);
        shmdt(chat);
    }

    if (shmid >= 0) 
    {
        shmctl(shmid, IPC_RMID, NULL);
    }

    printf("\nServer stopped and memory removed.\n");
    exit(0);
}


int main() 
{
    signal(SIGINT, cleanup);

    shmid = shmget(SHM_KEY, sizeof(SharedChat), IPC_CREAT | IPC_EXCL | 0666);
    if (shmid < 0) 
    { 
        perror("shmget"); 
        exit(-1); 
    }

    chat = shmat(shmid, NULL, 0);
    if (chat == (void*) - 1) 
    { 
        perror("shmat"); 
        exit(-1); 
    }

    pthread_mutexattr_t ma;
    pthread_mutexattr_init(&ma);
    pthread_mutexattr_setpshared(&ma, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&chat->mutex, &ma);

    chat->user_count = 0;
    chat->msg_count  = 0;

    printf("Server ready. Ctrl+C to stop.\n");
    while (1) 
        pause();
}
