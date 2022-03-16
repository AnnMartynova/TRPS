#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

#include <addThread.h>

//  Cоздать поток управления на базе заданной функции
int
addThreadCreate
(
	pthread_t	*pThreadId,	/* Адрес идентификатора потока	*/
    PTThreadFunc threadFunc,		/* Адрес функции потока */
    void		*threadArg,			/* Аргумент для функции	*/
	char		*threadName			/* Имя потока для печати*/
)
{
	int result;			/* Результат формирования потока	*/

	result = pthread_create(pThreadId, NULL, threadFunc, threadArg);


	printf("\nПоток %s : ", threadName);
	if (result)
		printf("не создан ! Result=%d \n", result);
	else
		printf("идентификатор потока TID = %X\n",(int) *pThreadId);
	return result;
}
