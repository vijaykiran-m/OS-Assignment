#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#define MSG_QUEUE_KEY 12345 // Unique key to identify the message queue used for IPC
#define MSG_TYPE_TERMINATE 999 // A specific message type for termination signals

// Structure for message queue messages
struct msg_buffer {
    long mtype; // Message type, used by the receiving process to filter messages
    int arr[9]; // Content of the message
} msg;

int main() {
    int msgid;
    char input;

    struct msg_buffer msg ;
    key_t key= ftok(".", 'a');
    int msgqid = msgget(key, 0666 | IPC_CREAT);
    if (msgqid == -1) {
        perror("msgget");
        return 1;
    }
   
   
    int type = 15 ;
    while (1) {
        printf("Do you want the Air Traffic Control System to terminate? (Y for Yes and N for No)\n");
        scanf(" %c", &input);
       
        if (input == 'Y' || input == 'y') {
           
            msg.arr[8]=1;
            msg.mtype=type;
            // Send the termination message to the message queue for the ATC to receive
            if (msgsnd(msgqid, &msg, sizeof(msg),IPC_NOWAIT) == -1) {
                perror("Error sending termination message"); // Print error if message sending fails
                exit(1); // Exit the program with a failure status
            }

            printf("Termination message sent to the Air Traffic Controller.\n");
            break;
        } else if (input == 'N' || input == 'n') {
            continue;
        } else {
            printf("Invalid input. Please enter Y for Yes or N for No.\n");
        }
    }


    return 0;
}