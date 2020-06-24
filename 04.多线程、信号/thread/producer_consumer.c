#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
struct msg // 资源共享区
{
    struct msg *next;
    int num; /* data */
};

struct msg *head;
struct msg *mp;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static void *consumer(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&mutex);
        while (head == NULL)
            pthread_cond_wait(&cond, &mutex);
        mp = head;
        head = mp->next;
        pthread_mutex_unlock(&mutex);
        printf("----------consume----%d\n", mp->num);
        free(mp);
        mp = NULL;
        sleep(rand()%3);
    }
    pthread_exit(NULL);
}

static void *producer(void *arg)
{
    while (1)
    {
        mp = malloc(sizeof(struct msg));
        mp->num = rand() % 400 + 1;
        printf("******producted----%d\n", mp->num);

        pthread_mutex_lock(&mutex);
        mp->next = head;
        head = mp;
        pthread_mutex_unlock(&mutex);

        pthread_cond_broadcast(&cond);
        sleep(rand()%3);
    }
    pthread_exit(NULL);
}

int main()
{
    pthread_t con_id, pro_id;

    pthread_create(&con_id, NULL, consumer, NULL);
    pthread_create(&pro_id, NULL, producer, NULL);

    pthread_join(con_id, NULL);
    pthread_join(pro_id, NULL);

    exit(0);
}