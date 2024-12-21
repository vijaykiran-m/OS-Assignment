#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#define MAX_MSG_SIZE 10
#define MAX_PASSENGERS 10
#define MAX_CARGO_ITEMS 100


struct Plane {
    long plane_id;
    int total_weight;
    int type;
    int num_passengers;
    int departure_airport;
    int arrival_airport;
};

struct message {
    long mtype;
    int plane_details[MAX_MSG_SIZE];
};

void create_passenger_processes(int num_passengers, int pipefd[][2]) {
    int weight;

    for (int i = 0; i < num_passengers; ++i) {
        if (pipe(pipefd[i]) == -1) {
            perror("Pipe failed");
            exit(EXIT_FAILURE);
        }

        pid_t pid;
        if ((pid = fork()) < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
       
        else if (pid == 0) { // Child process
            close(pipefd[i][0]); // Close read end of pipe
            printf("Enter Weight of Your Luggage for Passenger %d: ", i+1);
            scanf("%d", &weight);
            write(pipefd[i][1], &weight, sizeof(weight));
            printf("Enter Your Body Weight for Passenger %d: ", i+1);
            scanf("%d", &weight);
            write(pipefd[i][1], &weight, sizeof(weight));
            exit(EXIT_SUCCESS);
        } else {
            close(pipefd[i][1]); // Close write end of pipe
            wait(NULL); // Wait for child process to complete before creating next one
        }
    }
}

int main() {
    struct Plane plane;
    int pipefd[MAX_PASSENGERS][2];
    int weight;
    int plane_details[6];

    // message queue intialization
    key_t key;
    int msgqid;
    struct message msg;

    // Create a message queue
    key = ftok(".", 'a');
    msgqid = msgget(key, 0644 | IPC_CREAT);
    if (msgqid == -1) {
        perror("msgget");
        return 1;
    }


    printf("Enter Plane ID: ");
    scanf("%ld", &plane.plane_id);
    msg.plane_details[2] = plane.plane_id;

    printf("Enter Type of Plane (0 for cargo, 1 for passenger): ");
    scanf("%d", &plane.type);
    msg.plane_details[4] = plane.type;

    if (plane.type == 1) { // Passenger plane
        printf("Enter Number of Occupied Seats: ");
        scanf("%d", &plane.num_passengers);
        if (plane.num_passengers > MAX_PASSENGERS) {
            printf("Number of passengers exceeds maximum limit\n");
            exit(EXIT_FAILURE);
        }
        create_passenger_processes(plane.num_passengers, pipefd);
        // Calculate total weight for passenger plane
        plane.total_weight = 0;
        for (int i = 0; i < plane.num_passengers; i++) {
            read(pipefd[i][0], &weight, sizeof(weight)); // Read luggage weight
            plane.total_weight += weight;
            read(pipefd[i][0], &weight, sizeof(weight)); // Read body weight
            plane.total_weight += weight;
        }
        plane.total_weight += (75 * 7); // Crew weight: 7 members, average weight 75kg

        // assigning plane details -
        msg.plane_details[5] = plane.num_passengers;
        msg.plane_details[3] = plane.total_weight;
    }
   
    else if (plane.type == 0) { // Cargo plane
        printf("Enter Number of Cargo Items: ");
        scanf("%d", &plane.num_passengers);
        if (plane.num_passengers > MAX_CARGO_ITEMS) {
            printf("Number of cargo items exceeds maximum limit\n");
            exit(EXIT_FAILURE);
        }
        printf("Enter Average Weight of Cargo Items: ");
        scanf("%d", &weight);
        plane.total_weight += plane.num_passengers * weight;
        plane.total_weight = plane.total_weight + 150;
        msg.plane_details[3] = plane.total_weight;
    }
   
    // printf("%d",plane.total_weight);

    printf("Enter Airport Number for Departure: ");
    scanf("%d", &plane.departure_airport);
    msg.plane_details[1] = plane.departure_airport;

    printf("Enter Airport Number for Arrival: ");
    scanf("%d", &plane.arrival_airport);
    msg.plane_details[0] = plane.arrival_airport;

    

    msg.mtype = 11;

    // Send the message
    if (msgsnd(msgqid, &msg, sizeof(msg), 0) == -1) {
        perror("msgsnd");
        return 1;
    }
    long x=plane.plane_id+20;
    while(1){
        if(msgrcv(msgqid, &msg, sizeof(msg), x, IPC_NOWAIT) != -1) {
            printf("Plane %ld has successfully traveled from Airport %d to Airport %d!\n",
           plane.plane_id, plane.departure_airport, plane.arrival_airport);
           break;
        }
        sleep(1);
    }
    // Display message upon completion
    

    return 0;
}
