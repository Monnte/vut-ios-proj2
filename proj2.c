/**
 * @file proj2.c
 * @author Peter Zdravecký
 * @brief The Faneuil Hall problem
 * @version 0.1
 * @date 2020-04-14
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include "proj2.h"

Memory *memory = NULL;
FILE *file;

/*  Semaphores */
sem_t *sem = NULL;
sem_t *noJudge = NULL;
sem_t *mutex = NULL;
sem_t *confirmed = NULL;
sem_t *allSigned = NULL;
sem_t *parent = NULL;

int main(int argc, char *argv[])
{
    //----------------------------//
    //       Arguments Check      //
    //----------------------------//
    int param[5];
    int errcode = argumentParser(argc, argv, param);
    if (errcode != 0)
    {
        switch (errcode)
        {
        case 1:
            printErr("Too many or too less arguemnts.");
            break;
        case 2:
            printErr("Only numbers are allowed as arguments.");
            break;
        case 3:
            printErr("Arguments value exceeds boundaries of input max/min values.");
            break;

        default:
            printErr("Wrong arguments");
            break;
        }
        printHelp(argv[0]);
        exit(1);
    }
    //----------------------------//
    //       Asign Parameters     //
    //----------------------------//
    int PI = param[0];
    int IG = param[1];
    int JG = param[2];
    int IT = param[3];
    int JT = param[4];
    //----------------------------//
    //           Init             //
    //----------------------------//
    if (init() == 1)
    {
        clear();
        printErr("Something went wrong while initializing.");
        exit(1);
    }
    memory->certLeft = PI;
    memory->processleft = PI + 2; //Imigrants + Generator + Judge
    //----------------------------//
    //      Create Processes      //
    //----------------------------//
    pid_t parentPid = getpid();
    pid_t generatorPid;
    pid_t judgePid;

    fork();
    if (parentPid != getpid())
        generatorPid = getpid();
    if (parentPid == getpid())
    {
        fork();
        if (parentPid != getpid())
            judgePid = getpid();
    }

    //----------------------------//
    //         Generator          //
    //----------------------------//
    if (getpid() == generatorPid)
    {
        for (int i = 0; i < PI; i++)
        {
            sleepRand(IG);
            pid_t imi = fork();
            if (imi == 0)
            {
                imiProcess(i + 1, IT);
            }
        }

        //Last process check
        memory->processleft--;
        if (memory->processleft <= 0)
            sem_post(parent);

        exit(0);
    }
    //----------------------------//
    //           Judge            //
    //----------------------------//
    else if (getpid() == judgePid)
    {
        judgeProcess(JG, JT);
    }
    //----------------------------//
    //           Parent           //
    //----------------------------//

    sem_wait(parent);

    clear();
    exit(0);
}

void judgeProcess(int JG, int JT)
{

    while (memory->certLeft > 0)
    {
        sleepRand(JG);

        //Want to enter
        sem_wait(sem);
        fprintf(file, "%d:  JUDGE:     wants to enter\n", ++memory->ProcessCount);
        sem_post(sem);

        //Enters
        sem_wait(noJudge);
        sem_wait(mutex);

        sem_wait(sem);
        fprintf(file, "%d:  JUDGE:     enters : %d : %d : %d\n", ++memory->ProcessCount, memory->NE, memory->NC, memory->NB);
        memory->judgeconfirm = 1;
        sem_post(sem);

        //Check if evrery imigrant in bulding is checked
        if (memory->NE > memory->NC)
        {
            sem_wait(sem);
            fprintf(file, "%d:  JUDGE:     waits for imm : %d : %d : %d\n", ++memory->ProcessCount, memory->NE, memory->NC, memory->NB);
            sem_post(sem);

            sem_post(mutex);
            sem_wait(allSigned);
        }

        //Starts confrimation
        sem_wait(sem);
        int passimi = memory->NC;
        fprintf(file, "%d:  JUDGE:     starts confirmation : %d : %d : %d\n", ++memory->ProcessCount, memory->NE, memory->NC, memory->NB);
        sem_post(sem);

        sleepRand(JT);

        //Ends Confrimation
        sem_wait(sem);
        memory->certLeft -= memory->NC;
        memory->NE = 0;
        memory->NC = 0;
        fprintf(file, "%d:  JUDGE:     ends confirmation : %d : %d : %d\n", ++memory->ProcessCount, memory->NE, memory->NC, memory->NB);
        sem_post(sem);

        //Judge giving signal to imigrants to ask for certificate
        for (int i = 0; i < passimi; i++)
        {
            sem_post(confirmed);
        }

        sleepRand(JG);

        //Judge leaving
        sem_wait(sem);
        fprintf(file, "%d:  JUDGE:     leaves : %d : %d : %d\n", ++memory->ProcessCount, memory->NE, memory->NC, memory->NB);
        memory->judgeconfirm = 0;
        sem_post(sem);

        sem_post(mutex);
        sem_post(noJudge);
    }

    sem_wait(sem);
    fprintf(file, "%d:  JUDGE:      finishes\n", ++memory->ProcessCount);
    sem_post(sem);

    //Last process check
    memory->processleft--;
    if (memory->processleft <= 0)
        sem_post(parent);

    exit(0);
}

void imiProcess(int id, int IT)
{
    sem_wait(sem);
    fprintf(file, "%d:  IMM %d:     starts \n", ++memory->ProcessCount, id);
    sem_post(sem);

    //Enter
    sem_wait(noJudge);

    sem_wait(sem);
    fprintf(file, "%d:  IMM %d:     enters : %d : %d : %d\n", ++memory->ProcessCount, id, ++memory->NE, memory->NC, ++memory->NB);
    sem_post(sem);

    sem_post(noJudge);

    //Check
    sem_wait(mutex);

    sem_wait(sem);
    fprintf(file, "%d:  IMM %d:     checks : %d : %d : %d\n", ++memory->ProcessCount, id, memory->NE, ++memory->NC, memory->NB);
    sem_post(sem);

    if (memory->judgeconfirm == 1 && memory->NE == memory->NC)
        sem_post(allSigned);
    else
        sem_post(mutex);

    //Wait for confrimation
    sem_wait(confirmed);

    sem_wait(sem);
    fprintf(file, "%d:  IMM %d:     wants certificate : %d : %d : %d\n", ++memory->ProcessCount, id, memory->NE, memory->NC, memory->NB);
    sem_post(sem);

    sleepRand(IT);

    sem_wait(sem);
    fprintf(file, "%d:  IMM %d:     got certificate : %d : %d : %d\n", ++memory->ProcessCount, id, memory->NE, memory->NC, memory->NB);
    sem_post(sem);

    //Leaving
    sem_wait(noJudge);

    sem_wait(sem);
    memory->NB--;
    fprintf(file, "%d:  IMM %d:     leaves : %d : %d : %d\n", ++memory->ProcessCount, id, memory->NE, memory->NC, memory->NB);
    sem_post(sem);

    sem_post(noJudge);

    //Last process check
    memory->processleft--;
    if (memory->processleft <= 0)
        sem_post(parent);

    exit(0);
}

int init()
{
    file = fopen(fileout, "w");
    if(file == NULL)
        return 1;
    setbuf(file, NULL);

    sem = sem_open("xzdrav00.ios.proj2.semafor", O_CREAT | O_EXCL, 0666, 1);
    noJudge = sem_open("xzdrav00.ios.proj2.noJudge", O_CREAT | O_EXCL, 0666, 1);
    mutex = sem_open("xzdrav00.ios.proj2.mutex", O_CREAT | O_EXCL, 0666, 1);
    confirmed = sem_open("xzdrav00.ios.proj2.confirmed", O_CREAT | O_EXCL, 0666, 0);
    allSigned = sem_open("xzdrav00.ios.proj2.allSigned", O_CREAT | O_EXCL, 0666, 0);
    parent = sem_open("xzdrav00.ios.proj2.parent", O_CREAT | O_EXCL, 0666, 0);

    memory = mmap(NULL, sizeof(*(memory)), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sem == SEM_FAILED || noJudge == SEM_FAILED || mutex == SEM_FAILED || confirmed == SEM_FAILED || allSigned == SEM_FAILED || parent == SEM_FAILED)
        return 1;
    return 0;
}

void clear()
{
    sem_close(sem);
    sem_unlink("xzdrav00.ios.proj2.semafor");
    sem_close(noJudge);
    sem_unlink("xzdrav00.ios.proj2.noJudge");
    sem_close(mutex);
    sem_unlink("xzdrav00.ios.proj2.mutex");
    sem_close(confirmed);
    sem_unlink("xzdrav00.ios.proj2.confirmed");
    sem_close(allSigned);
    sem_unlink("xzdrav00.ios.proj2.allSigned");
    sem_close(parent);
    sem_unlink("xzdrav00.ios.proj2.parent");

    if (file != NULL)
        fclose(file);

    munmap(memory, sizeof(memory));
}

int argumentParser(int argc, char *argv[], int *param)
{
    if (argc != 6)
        return 1;

    char *endptr;
    for (int i = 1; i < argc; i++)
    {
        param[i - 1] = strtol(argv[i], &endptr, 10);
        if (strcmp(endptr, "\0") != 0)
            return 2;
    }

    if (param[0] < 1 || param[1] < 0 || param[1] > 2000 || param[2] < 0 || param[2] > 2000 || param[3] < 0 || param[3] > 2000 || param[4] < 0 || param[4] > 2000)
        return 3;

    return 0;
}

void printErr(char *errmessage)
{
    fprintf(stderr, "%s\n", errmessage);
}

void printHelp(char *prgname)
{
    fprintf(stderr, "usage : %s [ PI | IG | JG | IT | JT ]\n", prgname);
    fprintf(stderr, "        PI : numbers of threads to be generated                     | PI >= 1 | PI = Integer\n");
    fprintf(stderr, "        IG : max value after new thread is generated (ms)           | IG >= 0 && IG <= 2000 | IG = Integer\n");
    fprintf(stderr, "        JG : max value after judge again enter to building (ms)     | JG >= 0 && JG <= 2000 | JG = Integer\n");
    fprintf(stderr, "        IT : max value which simulate time to gain certificate (ms) | IT >= 0 && IT <= 2000 | IT = Integer\n");
    fprintf(stderr, "        JT : max value which simulate time to give certificate (ms) | JT >= 0 && JT <= 2000 | JT = Integer\n");
}