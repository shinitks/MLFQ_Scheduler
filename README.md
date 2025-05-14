# 🧠 MLFQ Scheduler Implementation
This project implements a Multilevel Feedback Queue (MLFQ) CPU scheduler using C and pthreads, simulating realistic scheduling scenarios across four different queues.

## 🧾 Overview
The MLFQ scheduler consists of four levels:

1. Q0 – Round Robin (RR) with 5ms time quantum

2. Q1 – First-Come-First-Served (FCFS), 5ms time allocation

3. Q2 – Priority Scheduling (priority = inverse of remaining time), 5ms allocation

4. Q3 – Shortest Job First (SJF), 5ms allocation

Processes move between these queues depending on their execution status. Unfinished processes are cycled back to Q0.

## ✨ Features
✅ Support for both default and custom process configurations

🧵 Thread-safe implementation using pthreads

📊 Detailed scheduling visualization and logging

📈 Calculates individual and average turnaround times

## ⚙️ Requirements
Linux/Unix system

GCC compiler

pthread library

## 🛠️ Compilation
To compile the program, run:

gcc mlfq.c -o mlfq -lpthread

▶️ Usage
Run the compiled executable:

./mlfq

You will be prompted to choose:

Default processes:

A: 300ms

B: 150ms

C: 250ms

D: 350ms

E: 450ms

Custom processes:

Specify number of processes (1–10)

For each process:

Single-character ID

Total CPU time required (in ms)

📋 Sample Output (Default Configuration)
yaml
Copy
Edit
======= Final Results =======
Process    Total Time    Turnaround Time
A          300           1290
B          150            730
C          250           1135
D          350           1395
E          450           1500
Average Turnaround Time: 1210.00

## 🔍 Implementation Details

Simulates CPU execution using usleep()

Thread-safe queues with mutex locks

Tracks state transitions across queues

Displays detailed scheduling steps and queue transitions

## 🧹 Cleanup
Automatic cleanup of memory and mutexes after execution

## 📝 Notes
Output may be verbose — ideal for understanding scheduling behavior

For accurate results, run on a system with minimal background processes

Default config should yield an average turnaround time of ~1210ms

## 📁 License
This project is open-source and available under the MIT License.
