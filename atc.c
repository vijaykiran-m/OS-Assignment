#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <string.h>

#define MAX_AIRPORTS 10
#define MAX_PLANES 100
#define MAX_MSG_SIZE 10
#define MSG_TYPE 1

// Message structure
struct message {
    long mtype;
    int array[MAX_MSG_SIZE];
};

struct Messageair {
    long mtype;
    int arr[10];
    
};


void log_flight(int plane_id, int departure_airport, int arrival_airport);

int main() {
    // Variables
    int num_airports;
    int msgqid;
    key_t key;
    struct message msg;
    struct Messageair msgtoSend; 
    struct Messageair msgtoRec;

    // Prompt for the number of airports
    printf("Enter the number of airports to be handled/managed: ");
    scanf("%d", &num_airports);

    // Validate the number of airports
    if (num_airports < 2 || num_airports > MAX_AIRPORTS) {
        printf("Invalid number of airports.\n");
        return 1;
    }

    // Create a message queue
    key = ftok(".", 'a');
    msgqid = msgget(key, 0644 | IPC_CREAT);
    
    if (msgqid == -1) {
        perror("msgget");
        return 1;
    }
    long type=11;
    long type2=12;
    long  type3=13;
    long type4=15;
    
    while(1){
    /*while((msgrcv(msgqid, &msg, sizeof(msg), type, 0) == -1) && (msgrcv(msgqid,&msg,sizeof(msg),type2,0) ==-1)){
        sleep(1);
    }*/
    if((msgrcv(msgqid, &msg, sizeof(msg), type, IPC_NOWAIT) != -1) || (msgrcv(msgqid,&msg,sizeof(msg),type2,IPC_NOWAIT) !=-1) || (msgrcv(msgqid,&msg,sizeof(msg),type3,IPC_NOWAIT) !=-1) || (msgrcv(msgqid,&msg,sizeof(msg),type4,IPC_NOWAIT) !=-1) ){
        if(msg.mtype==11){
        msgtoSend.mtype = (long)msg.array[1];
        msgtoSend.arr[0] = 0;
        msgtoSend.arr[1] = msg.array[2];
        msgtoSend.arr[2] = msg.array[3];
        msgtoSend.arr[3] = msg.array[0];
        log_flight(msg.array[2],msg.array[1],msg.array[0]);
        printf("done");

        if (msgsnd(msgqid, &msgtoSend, sizeof(msgtoSend), 0) == -1) {
            perror("msgsnd");
            return 1;
        }
        }
        if(msg.mtype==12){
            msgtoSend.arr[0] = msg.array[0];
            msgtoSend.arr[1] = msg.array[1];
            msgtoSend.mtype = msg.array[3];
            msgtoSend.arr[3] = -10;
            if (msgsnd(msgqid, &msgtoSend, sizeof(msgtoSend), 0) == -1) {
                perror("msgsnd");
                return 1;
            }
        }
        if(msg.mtype==13){
            msgtoSend.arr[1] = msg.array[1];
            printf("%d",msgtoSend.arr[1]);
            msg.mtype = (long)msgtoSend.arr[1]+20;
            if (msgsnd(msgqid, &msg, sizeof(msg), 0) == -1) {
                perror("msgsnd");
                return 1;
            }
        }
        if(msg.mtype==15){
            int air[num_airports];
            for(int i=1;i<=num_airports;i++){
                msgtoSend.mtype = (long)i;
                msgtoSend.arr[0] = -1;
                msgtoSend.arr[1] = -1;
                msgtoSend.arr[2] = -1;
                msgtoSend.arr[3] = -1;
                printf("done");

                if (msgsnd(msgqid, &msgtoSend, sizeof(msgtoSend), IPC_NOWAIT) == -1) {
                    perror("msgsnd");
                    return 1;
                }
            }
            while(1){
                if((msgrcv(msgqid, &msg, sizeof(msg), type4, IPC_NOWAIT) != -1)){
                    if(msg.array[0]==-1){
                        air[msg.mtype-1]=-1;
                    }
                    for(int i=0;i<num_airports;i++){
                        if(air[i]!=-1){
                            if(i==num_airports-1){
                                return 0;
                            }
                            break;
                        }
                    }
                }
               
            }
            return 0;
        }
    }
    }
    printf("terimnated");
    return 0;

}

void log_flight(int plane_id, int departure_airport, int arrival_airport) {
    FILE *fp = fopen("AirTrafficController.txt", "a");
    if (fp == NULL) {
        perror("fopen");
        return;
    }
    fprintf(fp, "Plane %d has departed from Airport %d and will land at Airport %d.\n", plane_id, departure_airport, arrival_airport);
    fclose(fp);
}