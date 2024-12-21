#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct Message {
    long mtype;
    int arr[10];
};

typedef struct {
    int capacity;
    pthread_mutex_t mtx;
} Runway;

int compare(const void *a, const void *b) {
    return *(int*)a - *(int*)b;
}

struct argument {
    struct Message m;
    Runway *r;
    int airno;
    int *copy;          //copy of runway in original format
    int count;          //number of runway
};

void *runner(void *arg) {
    struct argument *a = (struct argument *)arg;
    Runway *runways = a->r;
    int *copy = a->copy;
    int count = a->count;
    int airportnumber = a->airno;
    struct Message m = a->m;
    int plane=m.arr[1];


    int c = m.arr[2];
    int index = 0;
    for (int i = 0; i < count + 1; i++) {
        if (runways[i].capacity >= c) {
            index = i;
            break;
        }
    }
    int runwaynum;
    for(int i = 0; i < count + 1; i++) {
        if (copy[i] == runways[index].capacity) {
            runwaynum = i;
            break;
        }
    }
    if (m.arr[0] == 0) {
        pthread_mutex_lock(&runways[index].mtx);
        printf("taking off \n");
        sleep(3);
        printf("Plane %d has completed boarding and taken off from Runway No. %d of Airport No.%d\n", plane, runwaynum + 1, airportnumber);
        pthread_mutex_unlock(&runways[index].mtx);
        sleep(30);
        struct Message my;
        key_t key = ftok(".", 'a');
        int msgid = msgget(key ,0666|IPC_CREAT);
        my.arr[0]=1;
        my.arr[1]=m.arr[1];
        my.arr[2]=m.arr[2];
        my.mtype=12;
        my.arr[3]=m.arr[3];
        msgsnd(msgid, &my, sizeof(my),0);
    } else if (m.arr[0] == 1) {
        pthread_mutex_lock(&runways[index].mtx);
        printf("landing\n");
        sleep(2);
        printf("deboarding\n");
        sleep(3);
        printf("Plane %d has landed on Runway No. %d of Airport No. %d  and has completed deboarding\n", plane, runwaynum + 1, airportnumber);
        pthread_mutex_unlock(&runways[index].mtx);
        struct Message my;
        key_t key = ftok(".", 'a');
        int msgid = msgget(key ,0644|IPC_CREAT);
        my.arr[0]=1;
        my.arr[1]=m.arr[1];
        my.arr[2]=m.arr[2];
        my.mtype=13;
        my.arr[3]=m.arr[3];
        msgsnd(msgid, &my, sizeof(my),IPC_NOWAIT);
    }
    return NULL;
}

int main() {
    printf("Enter airport number: ");
    int airportnumber;
    scanf("%d", &airportnumber);

    printf("Enter number of runways: ");
    int count;
    scanf("%d", &count);

    Runway runways[count + 1]; 
    int run[count + 1];

    printf("Enter load capacities as space-separated integers: ");
    for(int i = 0; i < count; i++) {
        scanf("%d", &run[i]);
    }
    run[count] = 15000;
    int copy[count + 1];
    for(int i = 0; i < count + 1; i++) {
        copy[i] = run[i];
    }
    qsort(run, count + 1, sizeof(int), compare);

    for(int i = 0; i < count + 1; i++) {
        runways[i].capacity = run[i];
        pthread_mutex_init(&runways[i].mtx, NULL); 
    }
    pthread_t threads[100];
    int t = 0;
    struct Message mee;
    key_t key = ftok(".", 'a');
    int msgid = msgget(key ,0666|IPC_CREAT);
    if (msgid == -1) {
        perror("msgget");
        return 1; 
    }
    while(1) {
        if (mee.arr[0] == -1) {
            break;
        }
        while((msgrcv(msgid, &mee, sizeof(mee), airportnumber, IPC_NOWAIT ) == -1)) {
            sleep(1);
        }
        
        struct argument arg;
        arg.r = runways;
        arg.copy = run;
        arg.airno = airportnumber;
        arg.count = count + 1;
        arg.m = mee;
        if (pthread_create(&threads[t], NULL, runner, (void *)&arg) != 0) {
            fprintf(stderr, "Error creating thread\n");
            break;
        }
        t++;
    }
    for (int i = 0; i < t; i++) {
        pthread_join(threads[i], NULL);
    }
    struct Message m1;
    m1.mtype=airportnumber;
    m1.arr[0]=-1;
    m1.arr[1]=-1;
    m1.arr[2]=-1;
    m1.arr[3]=-1;
    if(msgsnd(msgid, &m1, sizeof(m1), IPC_NOWAIT) == -1) {
        perror("msgsnd");
        return 1;
    }
    printf("terminated");
    return 0;
}
