/**
 * @name IOS Projekt 2 - Santa Claus synchronization problem
 * @author Pavel Kratochvil
 * login: xkrato61
 * version: V1.0
 * CFLAGS=-std=gnu99 -Wall -Wextra -Werror -pedantic
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#define SEM1 "/xkrato61-WaitToGetHitched"
#define SEM2 "/xkrato61-MemSem"
#define SEM3 "/xkrato61-ReinReady"
#define SEM4 "/xkrato61-SantaNeeded"
#define SEM5 "/xkrato61-AllReinHitched"
#define SEM6 "/xkrato61-ElfGotHelp"
#define SEM7 "/xkrato61-ProcessEnd"
#define SEM8 "/xkrato61-WorkshopEmpty"
#define SEM9 "/xkrato61-SantaDoneHelping"

typedef struct SharedMem {
    int process_ended_counter;
    int workshop_sign;   // 0=work, 1=closed
    int rein_hitched;    // hitched reindeer counter
    int rein_ready;      // ready reindeer counter
    int log_count;       // log counter
    int line_n;          // number of elves in line_to_workshop for workshop
    int rein_n;          // reindeer counter for assigning ids
    int elf_n;           // elf counter for assigning ids
}SharedMem_t;

sem_t *WaitToGetHitched = NULL;
sem_t *SantaDoneHelping = NULL;
sem_t *AllReinHitched = NULL;
sem_t *WorkshopEmpty = NULL;
sem_t *SantaNeeded = NULL;
sem_t *ElfGotHelp = NULL;
sem_t *ProcessEnd = NULL;
sem_t *ReinReady = NULL;
sem_t *MemSem = NULL;

int elf_id = 0;
int rein_id = 0;

static SharedMem_t *shared_mem = NULL;

FILE *fptr;

int n_e, n_r, t_e, t_r;

void elf_fn() {
    sem_wait(MemSem);
        // inc the number of elves
        shared_mem -> elf_n++;

        // assign elf id
        elf_id = shared_mem->elf_n;

        // Elf's init log
        shared_mem -> log_count++;
        printf("%d: Elf %d: started\n", shared_mem ->log_count, elf_id);
    sem_post(MemSem);

    // while the workshop is open
    while (1) {

        // sleep time - working alone
        int sleep_time = rand() % (int) t_e + 1;
        usleep(sleep_time);

        // Need help from Santa
        sem_wait(MemSem);
            shared_mem->log_count++;
            printf("%d: Elf %d: need help\n", shared_mem->log_count, elf_id);
        sem_post(MemSem);


        sem_wait(WorkshopEmpty);

        sem_wait(MemSem);
                // increment number of elves in line for work
                shared_mem->line_n++;

                if (shared_mem->line_n == 3) {
                    // try to wakeup Santa
                    sem_post(SantaNeeded);
                } else {
                    sem_post(WorkshopEmpty);
                }
        sem_post(MemSem);

        // if the workshop is closed, take holidays and break
            if (shared_mem->workshop_sign) {

                sem_wait(MemSem);
                    shared_mem->log_count++;
                    printf("%d: Elf %d: taking holidays\n", shared_mem->log_count, elf_id);
                sem_post(MemSem);

                sem_post(WorkshopEmpty);
                break;
            }

        // getting help from Santa
        sem_wait(ElfGotHelp);

            // if the workshop is closed, take holidays and break
            if (shared_mem->workshop_sign) {

                sem_wait(MemSem);
                    shared_mem->log_count++;
                    printf("%d: Elf %d: taking holidays\n", shared_mem->log_count, elf_id);
                sem_post(MemSem);

                sem_post(WorkshopEmpty);
                break;
            }

        // elf got help
        sem_wait(MemSem);
            shared_mem->line_n--;

            // the last one to leave let others to wait
            if (shared_mem->line_n == 0) {
                sem_post(WorkshopEmpty);
                sem_post(SantaDoneHelping);
            }
            printf("%d: Elf %d: get help\n", shared_mem->log_count, elf_id);
        sem_post(MemSem);

    } // end while(1)

    // increment exited process count
    sem_wait(MemSem);

        // increment of terminated processes
        shared_mem->process_ended_counter++;

        // the last process to finish unlock the main process to finish
        if (shared_mem->process_ended_counter == n_e + n_r + 1) {
            sem_post(ProcessEnd);
        }
    sem_post(MemSem);

    // exit after work
    exit(0);
} // end elf_fn()

void rein_fn() {

    sem_wait(MemSem);
        shared_mem->rein_n++;

        // assign reindeer id
        rein_id = shared_mem->rein_n;

        // reindeer init message
        shared_mem->log_count++;
        printf("%d: RD %d: rstarted\n", shared_mem->log_count, rein_id);
    sem_post(MemSem);

    // reindeer vacation
    int sleep_time = (int)t_r + (rand() % ((int)t_r/2) + 1);
    usleep(sleep_time);

    sem_wait(MemSem);
        // reindeer returned home
        shared_mem->rein_ready++;

        printf("%d: RD %d: return home\n", shared_mem->log_count, rein_id);

        // if all reindeer are home from vacation
        if (shared_mem->rein_ready == n_r) {
            sem_post(SantaNeeded);
        }
    sem_post(MemSem);

    sem_wait(WaitToGetHitched);
    sem_post(WaitToGetHitched);

    sem_wait(MemSem);

        shared_mem -> log_count++;
        printf("%d: RD %d: get hitched\n", shared_mem ->log_count, rein_id);

        // incrementing number of reindeer ready to get hitched
        shared_mem -> rein_hitched++;

        // wake up Santa if all rein are hitched
        if (shared_mem->rein_hitched == n_r) {
            sem_post(AllReinHitched);
        }

    sem_post(MemSem);

    // increment exited process count
    sem_wait(MemSem);
        // increment of terminated processes
        shared_mem->process_ended_counter++;
        // the last process to finish unlock the main process to finish
        if (shared_mem->process_ended_counter == n_e + n_r + 1) {
            sem_post(ProcessEnd);
        }
    sem_post(MemSem);

    // exiting hitched reindeer
    exit(0);
} // end rein_fn()

void santa_fn() {

    // Santa's init log
    sem_wait(MemSem);
        shared_mem -> log_count++;
        printf("%d: Santa: going to sleep.\n", shared_mem ->log_count);
    sem_post(MemSem);

    // helping elves until all reindeer are ready
    while (1) {

        // wait until needed
        sem_wait(SantaNeeded);

        sem_wait(MemSem);
        // all reindeer are ready
        if (shared_mem->rein_ready == n_r) {

            // close workshop
            shared_mem->workshop_sign = 1;

            // message from Santa closing workshop
            shared_mem->log_count++;
            printf("%d: Santa: closing workshop\n", shared_mem->log_count);

            sem_post(MemSem);
            break;
        } else {
            // helping three elves
            shared_mem->log_count++;
            printf("%d: Santa: helping elves\n", shared_mem->log_count);

            for (int i= 0; i < 3; i++) sem_post(ElfGotHelp);

            sem_post(MemSem);

            sem_wait(SantaDoneHelping);

            sem_wait(MemSem);
                shared_mem->log_count++;
                printf("%d: Santa: going to sleep.\n", shared_mem->log_count);
            sem_post(MemSem);
        } //end if (shared_mem->rein_ready == n_r)
    } // end while(1)

    for (int i= 0; i < 3; i++) sem_post(ElfGotHelp);

    // Santa starts hitching reindeer
    sem_post(WaitToGetHitched);

    // wait until all reindeer get hitched
    sem_wait(AllReinHitched);

    // Santa's christmas started message
    sem_wait(MemSem);
        shared_mem->log_count++;
        printf("%d: Santa: Christmas started.\n", shared_mem ->log_count);
    sem_post(MemSem);

    // increment exited process count
    sem_wait(MemSem);
    printf("Santa exited and %d processes remain\n", shared_mem->process_ended_counter);
        // increment of terminated processes
        shared_mem->process_ended_counter++;

        // the last process to finish unlock the main process to finish
        if (shared_mem->process_ended_counter == n_e + n_r + 1) {
            sem_post(ProcessEnd);
        }
    sem_post(MemSem);

    exit(0);
}

int init(){
    shared_mem = mmap(NULL, sizeof(SharedMem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    shared_mem -> elf_n = 0;                    // counter for creation
    shared_mem -> rein_n = 0;                   // counter for creation
    shared_mem -> log_count = 0;                // counter for logging
    shared_mem -> line_n = 0;                   // number of elves in line for workshop
    shared_mem -> rein_ready = 0;               // number of reindeer ready to get hitched
    shared_mem -> workshop_sign = 0;            // workshop sign, init to 0=work
    shared_mem -> rein_hitched = 0;             // counter of hitched reindeer
    shared_mem -> process_ended_counter = 0;    // counter of terminated processes

    if (shared_mem == MAP_FAILED){
        fprintf(stderr,"Shared memory allocation failed.\n");
        return 1;
    }

    // unliking previously unsuccessful semaphores
    sem_unlink(SEM1);
    sem_unlink(SEM2);
    sem_unlink(SEM3);
    sem_unlink(SEM4);
    sem_unlink(SEM5);
    sem_unlink(SEM6);
    sem_unlink(SEM7);
    sem_unlink(SEM8);
    sem_unlink(SEM9);

    {
        // inits to LOCKED, Santa unlocks reindeer to get hitched
        WaitToGetHitched = sem_open(SEM1, O_CREAT | O_EXCL, 0666, 0);
        if (WaitToGetHitched == SEM_FAILED) {
            fprintf(stderr, "Semaphore no.1 construction failed.\n");
            return 1;
        }

        // inits to UNLOCKED, unlocks during logging
        MemSem = sem_open(SEM2, O_CREAT | O_EXCL, 0666, 1);
        if (MemSem == SEM_FAILED) {
            fprintf(stderr, "Semaphore no.2 construction failed.\n");
            return 1;
        }

        // inits to LOCKED, unlocks when all reindeer are home
        ReinReady = sem_open(SEM3, O_CREAT | O_EXCL, 0666, 0);
        if (ReinReady == SEM_FAILED) {
            fprintf(stderr, "Semaphore no.3 construction failed.\n");
            return 1;
        }

        // inits to LOCKED, unlocks when Santa is needed by elves or reindeer
        SantaNeeded = sem_open(SEM4, O_CREAT | O_EXCL, 0666, 0);
        if (SantaNeeded == SEM_FAILED) {
            fprintf(stderr, "Semaphore no.4 construction failed.\n");
            return 1;
        }

        // inits to LOCKED, unlocks when all reindeer are hitched
        AllReinHitched = sem_open(SEM5, O_CREAT | O_EXCL, 0666, 0);
        if (AllReinHitched == SEM_FAILED) {
            fprintf(stderr, "Semaphore no.5 construction failed.\n");
            return 1;
        }

        // inits to LOCKED, unlocks when three elves get help in the workshop
        ElfGotHelp = sem_open(SEM6, O_CREAT | O_EXCL, 0666, 0);
        if (ElfGotHelp == SEM_FAILED) {
            fprintf(stderr, "Semaphore no.6 construction failed.\n");
            return 1;
        }

        // inits to LOCKED, unlocks when all processes terminate
        ProcessEnd = sem_open(SEM7, O_CREAT | O_EXCL, 0666, 0);
        if (ProcessEnd == SEM_FAILED) {
            fprintf(stderr, "Semaphore no.7 construction failed.\n");
            return 1;
        }

        // inits to UNLOCKED, locks when Workshop is full
        WorkshopEmpty = sem_open(SEM8, O_CREAT | O_EXCL, 0666, 1);
        if (WorkshopEmpty == SEM_FAILED) {
            fprintf(stderr, "Semaphore no.8 construction failed.\n");
            return 1;
        }

        // inits to LOCKED, unlocks when Santa finishes help
        SantaDoneHelping = sem_open(SEM9, O_CREAT | O_EXCL, 0666, 0);
        if (SantaDoneHelping == SEM_FAILED) {
            fprintf(stderr, "Semaphore no.9 construction failed.\n");
            return 1;
        }
    }

    fptr = fopen("proj2.out", "w+");
    if(fptr == NULL)
    {
        fprintf(stderr, "Unable to create file.\n");
        return 1;
    }

    return 0;
}

void dest(){
    // destroy shared memory
    munmap(shared_mem, sizeof(SharedMem_t));

    // closing semaphores
    sem_close(WaitToGetHitched);
    sem_close(MemSem);
    sem_close(ReinReady);
    sem_close(SantaNeeded);
    sem_close(AllReinHitched);
    sem_close(ElfGotHelp);
    sem_close(ProcessEnd);
    sem_close(WorkshopEmpty);
    sem_close(SantaDoneHelping);

    // close output file
    fclose(fptr);
}

void print_help () {
    printf("Usage: ./proj2 NE NR TE TR\n");
    printf("        NE - the number of elves, 0 < NE < 1000\n");
    printf("        NR - the number of reindeer, 0 < NR < 20\n");
    printf("        TE - maximum time in ms the elf works on independently, 0 <= TE <= 1000\n");
    printf("        TR - maximum time in ms, after which the reindeer come from holidays to help, 0 <= RE <= 1000\n");
}

void incorrect_params_exit () {
    fprintf(stderr, "Incorrect params format!\n");
    print_help();
}

int is_int (const char *string_val) {
    char *ptr;
    char *empty = "";

    double number_d = strtod(string_val, &ptr);
    if (*ptr != *empty) return 0;
    int number_i = (int)number_d;
    return number_d == number_i;
}

int str_to_int (const char *string_val) {
    char *ptr;
    return (int)strtod(string_val, &ptr);
}

int correct_arg_count(int argc, char *argv[]) {
    if (argc == 1 || !strcmp(argv[1], "--help" )) {
        print_help();
        return 1;
    }

    if (argc != 5) {
        incorrect_params_exit();
        return 1;
    }
    return 0;
}

int check_get_params(char *argv[], int *n_ep, int *n_rp, int *t_ep, int *t_rp) {
    if (!is_int(argv[1]) || !is_int(argv[2]) || !is_int(argv[3]) || !is_int(argv[4])) {
        return 1;
    }

    if (str_to_int(argv[1]) > 0 && str_to_int(argv[1]) < 1000 &&
            str_to_int(argv[2]) > 0 && str_to_int(argv[2]) < 20 &&
            str_to_int(argv[3]) >= 0 && str_to_int(argv[3]) <= 1000 &&
            str_to_int(argv[4]) >= 0 && str_to_int(argv[4]) <= 1000) {

        *n_ep = str_to_int(argv[1]);
        *n_rp = str_to_int(argv[2]);
        *t_ep = str_to_int(argv[3]);
        *t_rp = str_to_int(argv[4]);
        return 0;
    } else {
        return 1;
    }
}

int main(int argc, char *argv[]) {

    // argument count check
    if (correct_arg_count(argc, argv)) {
        return 1;
    }

    // argument validity check
    if (check_get_params(argv, &n_e, &n_r, &t_e, &t_r)) {
        incorrect_params_exit();
        return 1;
    }

    // Initializes semaphores
    init();

    // Santa creation
    pid_t parent = fork();
    if (parent == 0){
        santa_fn();
    }
    else {
        // elves creation
        for (int i = 0; i < n_e; i++) {
            pid_t pid = fork();
            if (pid == 0) {
                // elf's work, holidays and exit
                elf_fn();
            }
        }
        for (int i = 0; i < n_r; i++) {
            pid_t rein = fork();
            if (rein == 0) {
                // reindeer's vacation, hitching and exit
                rein_fn();
            }
        }
    }

    // main process waits for all to finish
    sem_wait(ProcessEnd);

    // destructor
    dest();
    return 0;
}
