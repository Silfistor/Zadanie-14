#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <ncurses.h>

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

static SharedChat *chat;
static char myname[MAX_NAME];
static WINDOW *chatlog,*userlist,*inputline;

void draw_users() 
{
    werase(userlist);
    pthread_mutex_lock(&chat->mutex);
    for (int i=0;i<chat->user_count;i++)
        mvwprintw(userlist,i,0,"%s",chat->users[i]);
    pthread_mutex_unlock(&chat->mutex);
    wrefresh(userlist);
}

void print_new(int *last) 
{
    pthread_mutex_lock(&chat->mutex);
    while (*last < chat->msg_count) 
    {
        ChatMessage *m = &chat->msgs[*last % MAX_MSGS];
        wprintw(chatlog,"[%s]: %s\n", m->name, m->text);
        (*last)++;
    }
    pthread_mutex_unlock(&chat->mutex);
    wrefresh(chatlog);
}

void *receiver(void *arg) 
{
    int last = 0;
    for (;;) 
    {
        print_new(&last);
        draw_users();
        sleep(1);
    }
    return NULL;
}

int main() 
{
    int shmid = shmget(SHM_KEY, sizeof(SharedChat), 0666);
    if (shmid < 0) 
    { 
        fprintf(stderr,"Run server first\n"); 
        return -1; 
    }

    chat = shmat(shmid, NULL, 0);
    if (chat == (void*) - 1) 
    { 
        perror("shmat"); 
        return 1; 
    }

    printf("Name: "); fflush(stdout);
    fgets(myname, MAX_NAME, stdin);
    myname[strcspn(myname,"\n")] = 0;

    pthread_mutex_lock(&chat->mutex);
    if (chat->user_count < MAX_USERS)
        strncpy(chat->users[chat->user_count++], myname, MAX_NAME);

    ChatMessage sys = { getpid(), "SYSTEM", "" };
    snprintf(sys.text, MAX_TEXT, "%s joined", myname);
    chat->msgs[chat->msg_count++ % MAX_MSGS] = sys;
    pthread_mutex_unlock(&chat->mutex);

    initscr(); 
    cbreak(); 
    noecho(); 
    curs_set(1);
    int chat_h = LINES-3, chat_w = COLS*3/4;

    WINDOW *chatbox=newwin(chat_h,chat_w,0,0); 
    box(chatbox,0,0);

    WINDOW *userbox=newwin(chat_h,COLS-chat_w,0,chat_w); 
    box(userbox,0,0);

    WINDOW *inputbox=newwin(3,COLS,LINES-3,0); 
    box(inputbox,0,0);

    wrefresh(chatbox); 
    wrefresh(userbox); 
    wrefresh(inputbox);

    chatlog=derwin(chatbox,chat_h-2,chat_w-2,1,1); 
    scrollok(chatlog,TRUE);

    userlist=derwin(userbox,chat_h-2,COLS-chat_w-2,1,1);
    inputline=derwin(inputbox,1,COLS-4,1,2);

    pthread_t tid; pthread_create(&tid,NULL,receiver,NULL);

    char buf[MAX_TEXT];
    for (;;) 
    {
        werase(inputline); 
        wmove(inputline,0,0); 
        wrefresh(inputline);
        echo(); 
        wgetnstr(inputline,buf,MAX_TEXT-1); 
        noecho();

        if (!strcmp(buf,"/quit")) 
            break;

        pthread_mutex_lock(&chat->mutex);
        ChatMessage m = { getpid(), "", "" };

        strncpy(m.name,myname,MAX_NAME);
        strncpy(m.text,buf,MAX_TEXT);

        chat->msgs[chat->msg_count++ % MAX_MSGS] = m;
        pthread_mutex_unlock(&chat->mutex);
    }

    pthread_mutex_lock(&chat->mutex);
    for (int i=0;i<chat->user_count;i++)
    {
        if (!strcmp(chat->users[i],myname)) 
        {
            for (int j=i;j<chat->user_count-1;j++)
                strcpy(chat->users[j], chat->users[j+1]);
            chat->user_count--;
            break;
        }
    }
    ChatMessage bye = { getpid(),"SYSTEM","" };
    snprintf(bye.text,MAX_TEXT,"%s left",myname);
    chat->msgs[chat->msg_count++ % MAX_MSGS] = bye;
    pthread_mutex_unlock(&chat->mutex);

    endwin();
    return 0;
}
