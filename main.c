#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-msc50-cpp"
/**
 * @name IOS Projekt 2
 * @author Pavel Kratochvil
 * login: xkrato61
 * version: V0.0
 * -std=gnu99 -Wall -Wextra -Werror -pedantic
 * -pthread
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

#define SEM1 "/xkrato61-ElfCounter"
#define SEM2 "/xkrato61-ReinCounter"
#define SEM3 "/xkrato61-LogSem"
#define SEM4 "/xkrato61-WorkLineSem"
#define SEM5 "/xkrato61-ReinReady"
#define SEM6 "/xkrato61-ReinReturned"
#define SEM7 "/xkrato61-ElfNeedHelp"
#define SEM8 "/xkrato61-Elf1Help"
#define SEM9 "/xkrato61-Elf2Help"
#define SEM10 "/xkrato61-GetHitched"

typedef struct SharedMem {
    int elf_n;
    int rein_n;
    int log_count;
    int rein_ready;
    int line_n;         // number of elves in line_to_workshop for workshop
    int workshop_sign;  // 0=work, 1=closed
}SharedMem_t;

sem_t *ElfCounter = NULL;
sem_t *ReinCounter = NULL;
sem_t *LogSem = NULL;
sem_t *WorkLineSem = NULL;
sem_t *ReinReady = NULL;
sem_t *ReinReturned = NULL;

sem_t *ElfNeedHelp = NULL;
sem_t *Elf1Help = NULL;
sem_t *Elf2Help = NULL;

sem_t *GetHitched = NULL;


int elf_n = 0;
int rein_n = 0;

static SharedMem_t *shared_mem = NULL;

unsigned int n_e, n_r, t_e, t_r;

void elf_fn() {

    // sleep time - working alone
    int sleep_time = rand() % (int)t_e + 1;
    usleep(sleep_time);

    // Need help from Santa
    sem_wait(WorkLineSem);
        shared_mem -> line_n = shared_mem -> line_n + 1;
        printf("%d: Elf %d: need help\n", shared_mem ->log_count, elf_n);
        printf("CURRENTLY %d IN LINE FOR WOOOOOOOORK\n", shared_mem->line_n);
    sem_post(WorkLineSem);

    // If the workshop is closed, take holidays
    if (shared_mem->workshop_sign) exit(0);

    switch (shared_mem->line_n) {
        case 1:
            // waits until Santa comes
            sem_wait(Elf1Help);

            // if workshop is open
            if (!shared_mem->workshop_sign) {

                // Elf is getting help from Santa
                sem_wait(LogSem);
                    shared_mem->log_count++;
                    printf("%d: Elf %d: get help\n", shared_mem->log_count, elf_n);
                sem_post(LogSem);
            }

            sem_wait(LogSem);
            shared_mem->log_count++;
            printf("%d: Elf %d: taking holidays\n", shared_mem->log_count, elf_n);
            sem_post(LogSem);

            exit(0);
        case 2:
            // waits until Santa comes
            sem_wait(Elf2Help);

            // if workshop is open
            if (!shared_mem->workshop_sign) {

                // Elf is getting help from Santa
                sem_wait(LogSem);
                    shared_mem->log_count++;
                    printf("%d: Elf %d: get help\n", shared_mem->log_count, elf_n);
                sem_post(LogSem);
            }

            sem_wait(LogSem);
                shared_mem->log_count++;
                printf("%d: Elf %d: taking holidays\n", shared_mem->log_count, elf_n);
            sem_post(LogSem);

            exit(0);
        case 3:
            // waits until Santa comes
            sem_wait(ElfNeedHelp);

            // if workshop is open
            if (!shared_mem->workshop_sign) {

                // Elf is getting help from Santa
                sem_wait(LogSem);
                    shared_mem->log_count++;
                    printf("%d: Elf %d: get help\n", shared_mem->log_count, elf_n);
                sem_post(LogSem);
            }

                sem_wait(LogSem);
                    shared_mem->log_count++;
                    printf("%d: Elf %d: taking holidays\n", shared_mem->log_count, elf_n);
                sem_post(LogSem);

            exit(0);
    }
}

void rein_fn() {

    // reindeer vacation
    int sleep_time = (int)t_r + (rand() % ((int)t_r/2) + 1);

    // reindeer vacation
    usleep(sleep_time);

    sem_wait(ReinReturned);

        // incrementing number of reindeer ready to get hitched
        shared_mem -> rein_ready++;

        // reindeer returned home from vacation message
        sem_wait(LogSem);
            shared_mem -> log_count++;
            printf("%d: RD %d: return home\n", shared_mem ->log_count, rein_n);
        sem_post(LogSem);

    sem_post(ReinReturned);

    // wait until all reindeer are back from holidays
    sem_wait(GetHitched);

    // reindeer message get hitched
    sem_wait(LogSem);
        shared_mem -> log_count++;
        printf("%d: RD %d: get hitched\n", shared_mem ->log_count, rein_n);
    sem_post(LogSem);

    // exiting hitched reindeer
    exit(0);
}

void santa_fn() {

    // Santa's init log
    sem_wait(LogSem);
        shared_mem -> log_count++;
        printf("%d: Santa: going to sleep.\n", shared_mem ->log_count);
    sem_post(LogSem);

    // helping elves until all reindeer are ready
    while (shared_mem->rein_ready != n_r+1) {

        //sem_wait(ReinReturned);
        //    if (shared_mem->rein_ready == n_r) {
        //        sem_post(GetHitched);
        //    }
        //sem_close(ReinReturned);


        sem_wait(WorkLineSem);
            // wake up Santa?
            if (shared_mem->line_n > 2) {
                // TODO: change this semaphore to a more general one for changing shared memory in a general way
                sem_wait(LogSem);
                printf("------Santas perspective: %d\n", shared_mem->line_n);
                shared_mem -> line_n = shared_mem -> line_n - 3;
                printf("-----Santas new perspective: %d\n", shared_mem->line_n);
                sem_post(LogSem);

                // Santa is helping elves log
                sem_wait(LogSem);
                    shared_mem -> log_count++;
                    printf("%d: Santa: helping elves\n", shared_mem ->log_count);
                sem_post(LogSem);

                // unlocking ElfNeedHelp, that unlocks help for 3 other elves
                sem_post(Elf1Help);
                sem_post(Elf2Help);
                sem_post(ElfNeedHelp);

            }
        sem_post(WorkLineSem);
    }

    sem_wait(LogSem);
        shared_mem -> log_count++;
        printf("%d: Santa: Christmas started.\n", shared_mem ->log_count);
    sem_post(LogSem);

    // TODO: hitch reindeer
    exit(0);
}

int init(){
    shared_mem = mmap(NULL, sizeof(SharedMem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    shared_mem -> elf_n = 0;            // counter for creation
    shared_mem -> rein_n = 0;           // counter for creation
    shared_mem -> log_count = 0;        // counter for logging
    shared_mem -> line_n = 0;           // number of elves in line for workshop
    shared_mem -> rein_ready = 0;       // number of reindeer ready to get hitched
    shared_mem -> workshop_sign = 0;    // workshop sign, init to 0=work

    if (shared_mem == MAP_FAILED){
        fprintf(stderr,"Shared memory allocation failed. \n");
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
    sem_unlink(SEM10);

    {
        // inits to UNLOCKED, unlocks only when a new elf is spawned
        ElfCounter = sem_open(SEM1, O_CREAT | O_EXCL, 0666, 1);
        if (ElfCounter == SEM_FAILED) {
            fprintf(stderr, "Semaphore no.1 construction failed.\n");
            return 1;
        }

        // inits to UNLOCKED, unlocks only when a new reindeer is spawned
        ReinCounter = sem_open(SEM2, O_CREAT | O_EXCL, 0666, 1);
        if (ReinCounter == SEM_FAILED) {
            fprintf(stderr, "Semaphore no.2 construction failed.\n");
            return 1;
        }

        // inits to UNLOCKED, unlocks during logging
        LogSem = sem_open(SEM3, O_CREAT | O_EXCL, 0666, 1);
        if (LogSem == SEM_FAILED) {
            fprintf(stderr, "Semaphore no.3 construction failed.\n");
            return 1;
        }

        // inits to UNLOCKED, unlocks only when an elf is joining the line to workshop
        WorkLineSem = sem_open(SEM4, O_CREAT | O_EXCL, 0666, 1);
        if (WorkLineSem == SEM_FAILED) {
            fprintf(stderr, "Semaphore no.4 construction failed.\n");
            return 1;
        }

        // inits to 0, when reindeer are ready switch to 1
        ReinReady = sem_open(SEM5, O_CREAT | O_EXCL, 0666, 0);
        if (ReinReady == SEM_FAILED) {
            fprintf(stderr, "Semaphore no.5 construction failed.\n");
            return 1;
        }

        // inits to UNLOCKED, unlocks only when a reindeer returned from vacation
        ReinReturned = sem_open(SEM6, O_CREAT | O_EXCL, 0666, 1);
        if (ReinReturned == SEM_FAILED) {
            fprintf(stderr, "Semaphore no.6 construction failed.\n");
            return 1;
        }

        // inits to LOCKED, unlocks only if Santa helps elves in the workshop
        ElfNeedHelp = sem_open(SEM7, O_CREAT | O_EXCL, 0666, 0);
        if (ElfNeedHelp == SEM_FAILED) {
            fprintf(stderr, "Semaphore no.7 construction failed.\n");
            return 1;
        }

        // inits to LOCKED, unlocks only if Santa helps elves in the workshop
        Elf1Help = sem_open(SEM8, O_CREAT | O_EXCL, 0666, 0);
        if (Elf1Help == SEM_FAILED) {
            fprintf(stderr, "Semaphore no.8 construction failed.\n");
            return 1;
        }

        // inits to LOCKED, unlocks only if Santa helps elves in the workshop
        Elf2Help = sem_open(SEM9, O_CREAT | O_EXCL, 0666, 0);
        if (Elf2Help == SEM_FAILED) {
            fprintf(stderr, "Semaphore no.9 construction failed.\n");
            return 1;
        }

        // inits to LOCKED, only santa can unlock a hitch a reindeer
        GetHitched = sem_open(SEM10, O_CREAT | O_EXCL, 0666, 0);
        if (GetHitched == SEM_FAILED) {
            fprintf(stderr, "Semaphore no.10 construction failed.\n");
            return 1;
        }

    }
    return 0;
}

void dest(){
    munmap(shared_mem, sizeof(SharedMem_t));
    sem_close(ElfCounter);
    sem_close(ReinCounter);
    sem_close(LogSem);
    sem_close(WorkLineSem);
    sem_close(ReinReady);
    sem_close(ReinReturned);
    sem_close(ElfNeedHelp);
    sem_close(Elf1Help);
    sem_close(Elf2Help);
    sem_close(GetHitched);
    // TODO: Unlink all semaphores
}

void print_help () {
    printf("Usage: ./proj2 NE NR TE TR\n");
    printf("        NE - the number of elves, 0 < NE < 1000\n");
    printf("        NR - the number of reindeer, 0 < NR < 20\n");
    printf("        TE - maximum time in ms the elf works on independently, 0 <= TE <= 1000\n");
    printf("        TR - maximum time in ms, after which the reindeer come from holidays to help, 0 <= RE <= 1000\n");
}

void incorrect_params_exit () {
    printf("Incorrect params format!\n\n");
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

unsigned int str_to_int (const char *string_val) {
    char *ptr;
    return (unsigned int)strtod(string_val, &ptr);
}

int correct_arg_count(int argc, char *argv[]) {
    if (argc == 1 || !strcmp(argv[1], "--help" )) {
        print_help();
        return 0;
    }

    if (argc != 5) {
        incorrect_params_exit();
        return 0;
    }
    return 1;
}

int check_get_params(char *argv[], unsigned int *n_ep, unsigned int *n_rp, unsigned int *t_ep, unsigned int *t_rp) {
    if (!is_int(argv[1]) || !is_int(argv[1]) || !is_int(argv[1]) || !is_int(argv[4])) {
        incorrect_params_exit();
        return 0;
    }
    // TODO: all are supposed to be >= 0
    *n_ep = str_to_int(argv[1]);
    *n_rp = str_to_int(argv[2]);
    *t_ep = str_to_int(argv[3]);
    *t_rp = str_to_int(argv[4]);
    return 1;
}

int main(int argc, char *argv[]) {

    // correct argument count check
    if (!correct_arg_count(argc, argv)) return 1;

    // checking argument validity
    if (!check_get_params(argv, &n_e, &n_r, &t_e, &t_r)) return 1;

    // Initializes semaphores
    init();

    // Satan creation
    pid_t parent = fork();
    if (parent == 0){
        santa_fn();
    }
    else {
        // eleves creation
        printf("number of elves: %d\n", n_e);
        for (int i = 0; i < n_e; i++) {
            pid_t pid = fork();
            if (pid == 0) {

                sem_wait(ElfCounter);
                    // inc the number of elves
                    shared_mem -> elf_n++;

                    // assign elf id
                    elf_n = shared_mem->elf_n;

                    // Elf's init log
                    sem_wait(LogSem);
                        shared_mem -> log_count++;
                        printf("%d: Elf %d: started\n", shared_mem ->log_count, elf_n);
                    sem_post(LogSem);

                sem_post(ElfCounter);

                // elf's work, holidays and exit
                elf_fn();
                exit(0);
            }
        }
        for (int i = 0; i < n_r; i++) {
            pid_t rein = fork();
            if (rein == 0) {
                sem_wait(ReinCounter);
                    shared_mem -> rein_n++;
                    rein_n = shared_mem->rein_n;
                    sem_wait(LogSem);
                        shared_mem -> log_count++;
                        printf("%d: RD %d: rstarted\n", shared_mem ->log_count, rein_n);
                    sem_post(LogSem);

                sem_post(ReinCounter);
                rein_fn();
                exit(0);
            }
        }
    }
    dest();
    return 0;
}