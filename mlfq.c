#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_PROCESSES 10
#define TIME_QUANTUM 5

typedef struct {
    char id;
    int remaining_time;
    int total_time;
    int arrival_time;
    int completion_time;
    int turnaround_time;
    int priority;
    int current_queue;
    pthread_t thread;
    int is_running;
    int is_completed;
} Process;

typedef struct {
    Process *items[MAX_PROCESSES];
    int front;
    int rear;
    int count;
    pthread_mutex_t lock;
} Queue;

Process processes[MAX_PROCESSES];
int process_count = 0;
int current_time = 0;
pthread_mutex_t time_lock = PTHREAD_MUTEX_INITIALIZER;
int all_processes_completed = 0;

Queue q0, q1, q2, q3;

void initQueue(Queue *q) {
    q->front = 0;
    q->rear = -1;
    q->count = 0;
    pthread_mutex_init(&q->lock, NULL);
}

int isEmpty(Queue *q) {
    int empty;
    pthread_mutex_lock(&q->lock);
    empty = (q->count == 0);
    pthread_mutex_unlock(&q->lock);
    return empty;
}

void enqueue(Queue *q, Process *process) {
    pthread_mutex_lock(&q->lock);
    if (q->count < MAX_PROCESSES) {
        q->rear = (q->rear + 1) % MAX_PROCESSES;
        q->items[q->rear] = process;
        q->count++;
    }
    pthread_mutex_unlock(&q->lock);
}

Process *dequeue(Queue *q) {
    Process *process = NULL;
    pthread_mutex_lock(&q->lock);
    if (q->count > 0) {
        process = q->items[q->front];
        q->front = (q->front + 1) % MAX_PROCESSES;
        q->count--;
    }
    pthread_mutex_unlock(&q->lock);
    return process;
}

void printResults() {
    printf("\n======= Final Results =======\n");
    printf("Process\tTotal Time\tTurnaround Time\n");
    int total_turnaround = 0;

    for (int i = 0; i < process_count; i++) {
        printf("%c\t%d\t\t%d\n", processes[i].id, processes[i].total_time, processes[i].turnaround_time);
        total_turnaround += processes[i].turnaround_time;
    }

    double avg_turnaround = (double)total_turnaround / process_count;
    printf("\nAverage Turnaround Time: %.2f\n", avg_turnaround);
}

int areAllProcessesCompleted() {
    for (int i = 0; i < process_count; i++) {
        if (!processes[i].is_completed) {
            return 0;
        }
    }
    return 1;
}

void *process_thread(void *arg) {
    Process *process = (Process *)arg;

    while (!process->is_completed) {
        while (!process->is_running && !process->is_completed) {
            usleep(1000);
        }

        if (process->is_completed) {
            break;
        }

        int time_to_run = (process->remaining_time < TIME_QUANTUM) ? process->remaining_time : TIME_QUANTUM;

        printf("Process %c running for %d ms in Q%d\n",
               process->id, time_to_run, process->current_queue);

        usleep(time_to_run * 1000);

        pthread_mutex_lock(&time_lock);
        process->remaining_time -= time_to_run;
        current_time += time_to_run;

        if (process->remaining_time <= 0) {
            process->is_completed = 1;
            process->completion_time = current_time;
            process->turnaround_time = process->completion_time;
            printf("Process %c completed at time %d\n", process->id, current_time);
        }

        process->is_running = 0;
        pthread_mutex_unlock(&time_lock);
    }

    return NULL;
}

void *scheduler_thread(void *arg) {
   
   
    for (int i = 0; i < process_count; i++) {
        processes[i].current_queue = 0;
        enqueue(&q0, &processes[i]);
    }

    while (!all_processes_completed) {
      
      
        while (!isEmpty(&q0)) {
            Process *p = dequeue(&q0);
            if (p->is_completed) continue;

            p->is_running = 1;
            printf("Scheduling %c in Q0 (RR)\n", p->id);

         

            while (p->is_running) {
                usleep(1000);
            }

            if (!p->is_completed) {
                p->current_queue = 1;
                enqueue(&q1, p);
                printf("Process %c moved to Q1 with %d ms remaining\n", p->id, p->remaining_time);
            }
        }

        // Q1: FCFS
        while (!isEmpty(&q1)) {
            Process *p = dequeue(&q1);
            if (p->is_completed) continue;

            p->is_running = 1;
            printf("Scheduling %c in Q1 (FCFS)\n", p->id);

            while (p->is_running) {
                usleep(1000);
            }

            if (!p->is_completed) {
                p->current_queue = 2;
                enqueue(&q2, p);
                printf("Process %c moved to Q2 with %d ms remaining\n", p->id, p->remaining_time);
            }
        }

        // Q2: Priority (inverse of remaining time)
        Process *q2_sorted[MAX_PROCESSES];
        int q2_count = 0;
        
       
        while (!isEmpty(&q2)) {
            q2_sorted[q2_count++] = dequeue(&q2);
        }
        
        for (int i = 0; i < q2_count; i++) {
            for (int j = i+1; j < q2_count; j++) {
                if (q2_sorted[i]->remaining_time < q2_sorted[j]->remaining_time) {
                    Process *temp = q2_sorted[i];
                    q2_sorted[i] = q2_sorted[j];
                    q2_sorted[j] = temp;
                }
            }
        }

        for (int i = 0; i < q2_count; i++) {
            Process *p = q2_sorted[i];
            if (p->is_completed) continue;

            p->is_running = 1;
            printf("Scheduling %c in Q2 (Priority: %d)\n", p->id, p->remaining_time);

            while (p->is_running) {
                usleep(1000);
            }

            if (!p->is_completed) {
                p->current_queue = 3;
                enqueue(&q3, p);
                printf("Process %c moved to Q3 with %d ms remaining\n", p->id, p->remaining_time);
            }
        }

        // Q3: SJF
        Process *q3_sorted[MAX_PROCESSES];
        int q3_count = 0;
        
        while (!isEmpty(&q3)) {
            q3_sorted[q3_count++] = dequeue(&q3);
        }
        
        for (int i = 0; i < q3_count; i++) {
            for (int j = i+1; j < q3_count; j++) {
                if (q3_sorted[i]->remaining_time > q3_sorted[j]->remaining_time) {
                    Process *temp = q3_sorted[i];
                    q3_sorted[i] = q3_sorted[j];
                    q3_sorted[j] = temp;
                }
            }
        }

        for (int i = 0; i < q3_count; i++) {
            Process *p = q3_sorted[i];
            if (p->is_completed) continue;

            p->is_running = 1;
            printf("Scheduling %c in Q3 (SJF: %d)\n", p->id, p->remaining_time);

            while (p->is_running) {
                usleep(1000);
            }

            if (!p->is_completed) {
                p->current_queue = 0;
                enqueue(&q0, p);
                printf("Process %c cycled back to Q0 with %d ms remaining\n", p->id, p->remaining_time);
            }
        }

        all_processes_completed = areAllProcessesCompleted();
    }

    return NULL;
}

void initializeDefaultProcesses() {
    process_count = 5;
    processes[0] = (Process){'A', 300, 300, 0, 0, 0, 0, -1, 0, 0, 0};
    processes[1] = (Process){'B', 150, 150, 0, 0, 0, 0, -1, 0, 0, 0};
    processes[2] = (Process){'C', 250, 250, 0, 0, 0, 0, -1, 0, 0, 0};
    processes[3] = (Process){'D', 350, 350, 0, 0, 0, 0, -1, 0, 0, 0};
    processes[4] = (Process){'E', 450, 450, 0, 0, 0, 0, -1, 0, 0, 0};
}

void getUserInput() {
    printf("Enter number of processes (1-%d): ", MAX_PROCESSES);
    scanf("%d", &process_count);
    
    if (process_count < 1 || process_count > MAX_PROCESSES) {
        printf("Invalid number of processes. Using default value 5.\n");
        initializeDefaultProcesses();
        return;
    }
    
    for (int i = 0; i < process_count; i++) {
        printf("Enter details for process %d:\n", i+1);
        printf("Name of the process (single character): ");
        scanf(" %c", &processes[i].id);
        printf("Total CPU time needed: ");
        scanf("%d", &processes[i].total_time);
        
        processes[i].remaining_time = processes[i].total_time;
        processes[i].arrival_time = 0;
        processes[i].completion_time = 0;
        processes[i].turnaround_time = 0;
        processes[i].priority = 0;
        processes[i].current_queue = -1;
        processes[i].is_running = 0;
        processes[i].is_completed = 0;
    }
}

int main() {
    int choice;
    printf("MLFQ Scheduler\n");
    printf("1. Use default processes (A=300, B=150, C=250, D=350, E=450)\n");
    printf("2. Enter custom processes\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);
    
    if (choice == 1) {
        initializeDefaultProcesses();
    } else if (choice == 2) {
        getUserInput();
    } else {
        printf("Invalid choice. Using default processes.\n");
        initializeDefaultProcesses();
    }

    printf("\nStarting MLFQ Scheduler with %d processes:\n", process_count);
    printf("Process\tTotal Time\n");
    for (int i = 0; i < process_count; i++) {
        printf("%c\t%d\n", processes[i].id, processes[i].total_time);
    }

    initQueue(&q0);
    initQueue(&q1);
    initQueue(&q2);
    initQueue(&q3);

    pthread_t scheduler;
    pthread_create(&scheduler, NULL, scheduler_thread, NULL);

    for (int i = 0; i < process_count; i++) {
        pthread_create(&processes[i].thread, NULL, process_thread, &processes[i]);
    }

    pthread_join(scheduler, NULL);

    for (int i = 0; i < process_count; i++) {
        pthread_join(processes[i].thread, NULL);
    }

    printResults();

    pthread_mutex_destroy(&time_lock);
    pthread_mutex_destroy(&q0.lock);
    pthread_mutex_destroy(&q1.lock);
    pthread_mutex_destroy(&q2.lock);
    pthread_mutex_destroy(&q3.lock);

    return 0;
}
