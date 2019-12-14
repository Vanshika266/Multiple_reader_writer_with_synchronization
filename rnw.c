#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/un.h>
#include <semaphore.h>
#include <stdio.h>
#include<unistd.h>
#include<pthread.h>

int len;
int shared_msg[256];
int num_read;
int num_write;
int front;
int end;
pthread_t readers[256];
pthread_t writers[256];
int readv[256];
int start=0;

int value_writer[256];
int work_reader[256];
int work_writer[256];
int mode_writer[256];
int mode_reader[256];
int pos_r[256];
int pos_w[256];

sem_t wlocks[256];
sem_t rlocks[256];
sem_t Dequeue;
sem_t Enqueue;
int counts[256];
sem_t current;


void *writing(void *client_id1)
{
    int client_id = *((int *)(client_id1));
    do{
    sem_wait(&current);
    sem_wait(&wlocks[client_id]);

    sleep(2);

    printf("%d\n",work_writer[client_id]);
    printf("%d\n",value_writer[client_id]);
    shared_msg[work_writer[client_id]] = value_writer[client_id];
    printf("Writer %d writing at index %d \n",client_id,work_writer[client_id]);

    sem_post(&wlocks[client_id]);
    sem_post(&current);

    }
    while(1==1);

 //   sleep(2);


}

void *addNum(void *client_id1)
{
    do{
    int client_id = *((int *)(client_id1));
    sem_wait(&current);
    sem_wait(&Enqueue);
    sem_wait(&wlocks[client_id]);

    sleep(2);
    shared_msg[work_writer[len]] = value_writer[client_id];
    len=len+1;
    end=end+1;
    printf("Writer %d enqueued value at index %d \n",client_id,work_writer[client_id]);

    sem_post(&wlocks[client_id]);
    sem_post(&Enqueue);
    sem_post(&current);
    }
    while(1);

}

void *delNum(void *client_id1)
{
    do{
    int client_id = *((int *)(client_id1));

    sem_wait(&Dequeue);
    sem_wait(&wlocks[work_reader[client_id]]);

    sleep(2);
    shared_msg[work_reader[client_id]]=-1;
    printf("Value dequeued by reader %d on index %d \n",client_id,work_reader[client_id]);

    sem_post(&wlocks[work_reader[client_id]]);
    sem_post(&Dequeue);
    }
    while(1);
}





void *reading(void *client_id1)
{

     int client_id = *((int *)(client_id1));
     do{
     sem_wait(&rlocks[client_id]);

     sleep(2);
     counts[client_id]++;
     if(counts[client_id]==1)
     {
        sem_wait(&wlocks[client_id]);
     }
     sem_post(&rlocks[client_id]);

     printf("Reading is performed by reader %d on index %d\n",client_id,work_reader[client_id]);
     printf("VALUE : %d \n",shared_msg[work_reader[client_id]]);


     readv[start] = shared_msg[work_reader[client_id]];
     start++;
     sem_wait(&rlocks[client_id]);
     counts[client_id]--;
     if(counts[client_id]==0)
     {
         sem_post(&wlocks[client_id]);
     }
     sem_post(&rlocks[client_id]);
     }
     while(1);
}

   //  printf("")





int main()
{
    printf("Enter the length of queue: \n");
    scanf("%d",&len);
    front=0;
    end=len-1;
    for(int i=front;i<256;i++)
    {
        if(i<=100)
        {
            shared_msg[i]=i;
        }
        else
        {
            shared_msg[i]=-1;
        }
    }

    printf("Shared memory inititalized\n");
    printf("Enter Number Of Writers: \n");
    scanf("%d",&num_write);
  /*  if(num_write<0 || num_write>255)
    {
        printf("Enter Valid number! \n");
        scanf("%d\n",&num_write);
    }*/
    printf("Enter Number of Readers: \n");
    scanf("%d",&num_read);
    if(num_read<0 || num_read>255)
    {
        printf("Enter Valid number! \n");
        scanf("%d\n",&num_read);
    }


    for(int j=0;j<num_write;j++)
    {
        pos_w[j]=j;
        printf("Choose the option for Writer %d\n ",j);
        printf("Choose 1 to Enqueue in the shared queue \n");
        printf("Choose 2 to Write to the shared queue \n");
        scanf("%d",&mode_writer[j]);
        printf("%d",mode_writer[j]);
        if(mode_writer[j]!=1 && mode_writer[j]!=2)
        {
            printf("Enter a valid option \n");
            scanf("%d",&mode_writer[j]);
        }
        if(mode_writer[j]==2)
        {
            printf("Enter an index to write \n");
            scanf("%d",&work_writer[j]);
            while(work_writer[j]<front || work_writer[j]>=end)
            {
                printf("Enter a valid index \n");
                scanf("%d",&work_writer[j]);
            }
        }
        printf("Enter value: \n");
        scanf("%d",value_writer);
        sleep(2);

    }

    printf("\n");
    for(int j=0;j<num_read;j++)
    {
        pos_r[j]=j;
        printf("Choose the option for Reader %d\n ",j);
        printf("Choose 1 Dequeue from the shared queue \n");
        printf("Choose 2 Read from the shared queue \n");
        scanf("%d",&mode_reader[j]);
        if(mode_reader[j]!=1 && mode_reader[j]!=2)
        {
            printf("Enter a valid option \n");
            scanf("%d",&mode_reader[j]);
        }
        if(mode_reader[j]==2)
        {
            printf("Enter an index to read \n");
            scanf("%d",&work_reader[j]);
            while(work_reader[j]<front || work_reader[j]>=end)
            {
                printf("Enter a valid index \n");
                scanf("%d",&work_reader[j]);
            }
        }
    }


    for(int h=0;h<256;h++)
    {
        counts[h] = 0;
    }

    for(int u=0;u<256;u++)
    {
        sem_init(&rlocks[u],0,1);
    }
    for(int u=0;u<256;u++)
    {
        sem_init(&wlocks[u],0,1);
    }

    sem_init(&Dequeue,0,1);
    sem_init(&Enqueue,0,1);

    sem_init(&current,0,1);

    for(int q=0;q<num_write;q++)
    {
        if(mode_writer[q]==1)
        {
            pthread_create( &writers[q], NULL, addNum,&q);
        }
    }
    for(int q2=0;q2<num_write;q2++)
    {
        if(mode_writer[q2]==2)
        {
            pthread_create( &writers[q2], NULL, writing,&q2);
        }
    }

    sleep(3);

    for(int q1=0;q1<num_read;q1++)
    {
        if(mode_writer[q1]==1)
        {
            pthread_create( &readers[q1], NULL, delNum,&q1);
        }
    }

    for(int q3=0;q3<num_read;q3++)
    {
        if(mode_reader[q3]==2)
        {
            pthread_create( &readers[q3], NULL, reading,&q3);
        }
    }

    for(int y=0;y<num_write;y++)
    {
        pthread_join(writers[y],NULL);
    }
    for(int y=0;y<num_read;y++)
    {
        pthread_join(readers[y],NULL);
    }

}


/* resources:
    * lecture slides

    */





