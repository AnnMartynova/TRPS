#ifndef addThread_H
#define addThread_H

#include <pthread.h>

typedef void * (*PTThreadFunc)(void *); /* Указатель на функцию потока*/

extern int addThreadCreate
(
	pthread_t	*pThreadId,	/* Адрес идентификатора потока	*/
    PTThreadFunc threadFunc,		/* Адрес функции потока */
    void		*threadArg,			/* Аргумент для функции	*/
    char		*threadName			/* Имя потока для печати*/
);

#endif
