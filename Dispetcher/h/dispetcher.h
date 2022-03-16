#ifndef DISPETCHER_H_
#define DISPETCHER_H_

#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>

#include <addThread.h>
#include <asm/param.h>

typedef void TUsrCircleFunc(void *);

extern pthread_t pId_first;

extern int threadParamSet
(
	TUsrCircleFunc	*pFunc,		/* Функция потока	*/
	void			*param,		/* Параметр функции	*/
	double			freqw		/* Частота, гц		*/
);

extern int GetCount(void);

extern void dispetcherStart(void);

extern void * delThr(void);

extern void CleanMem(void);

#endif
