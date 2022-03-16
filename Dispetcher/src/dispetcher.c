#include <dispetcher.h>

typedef struct	TThreadParam /* Параметры цикла-потока	*/
{
	unsigned int	count;		/* Счетчик потока		*/
	double/*int*/	freqw;		/* Частота 1-100гц		*/
	TUsrCircleFunc	*pFunc;		/* Функция потока		*/
	void			*param;		/* Параметр функции		*/
	pthread_t		thrIdent;	/* Идентификатор потока	*/
	unsigned int	delLunch;	/* Дельта в тиках для открытия семафора	*/
	unsigned int	nxtLunch;	/* Момент очередного открытия  семафора	*/
	sem_t			*pSem;				/* Указатель на семафор			*/
	char 			threadName[15]; /* Название потока */
	struct TThreadParam	*pNextParam;	/* На следующий элемент списка	*/
}			TThreadParam;


static TThreadParam	*bgnThrList=(TThreadParam *)0;	/* Начало списка	*/
unsigned int  addSysClockCount=0;	/* Системный счетчик тиков таймера	*/


//  Функция для обработки интеравального таймера ( изменение момента очередного открытия семафора на дельту и раазблокировка семафора)
static void addSysClockRoutine(void)
{
	TThreadParam	*pThrd = bgnThrList; /* Начало списка потоков*/

	printf("\n%i тик таймера: ", addSysClockCount);

	while (pThrd) // Не нулевое значение
	{
		if (addSysClockCount >= pThrd->nxtLunch) // Если системный счетчик тиков больше или равен момента очередного открытия семафора
		{
			pThrd->nxtLunch += pThrd->delLunch;  // Тогда момент очередного открытия семафора увеличичвается на дельту семафора
			sem_post(pThrd->pSem); // и происходит разблокировка семафора
		}
		pThrd = pThrd->pNextParam; /*Переход к следующий элемент списка*/

	}
	addSysClockCount++;	/* Системный счетчик тиков таймера	*/
}


//	Создать интервальный таймер. На вход получает значение в микросекундах
int itimerCreate
(
	int	microSec	/* Период в микросекундах*/
)
{
	struct sigaction alarmTimerAct; /* Поведение сигнала*/
	struct itimerval alarmTimerVal; /*Следующее значение и текущее значения таймера*/

	sigemptyset(&alarmTimerAct.sa_mask); //sa_mask - поле для указания сигналов которые нужно блокировать. функция исключает из набора все сигналы и помечает как пустое множество
	alarmTimerAct.sa_handler=(void (*)(int))&addSysClockRoutine; // После полученя сигнала необходимо выполнить функцию системного таймера
	alarmTimerAct.sa_flags = 0;

	if (sigaction(SIGALRM, &alarmTimerAct, 0) == -1) //При получении сигнала SIGALRM запуск функции системного таймера. Если он не запустился, то выводится ошибка
	{
		printf("Ошибка создания интервального таймера !\n");
		return 1;
	}

	alarmTimerVal.it_value.tv_sec     = microSec / 1000000; /*Секунды текущего значения*/
	alarmTimerVal.it_value.tv_usec    = microSec % 1000000; /*Милисекунды текущего значения*/
	alarmTimerVal.it_interval.tv_sec  = microSec / 1000000; /*Секунды следующего значения*/
	alarmTimerVal.it_interval.tv_usec = microSec % 1000000; /*Милисекунды следующего значения*/

	if (setitimer(ITIMER_REAL, &alarmTimerVal, 0) == -1) //ITIMER_REAL - тип функции, который изменряет фактическое время. Когда указанное время выйдет, ядро отправляет процессу сигнал SIGALRM
	{
		perror("Ошибка настройки интервального таймера !\n");
		return 1;
	}

	return 0;
}


//Установка параметров потоков управления и построение списка потоков
int threadParamSet
(
	TUsrCircleFunc	*pFunc,		/* Функция потока	*/
	void			*param,		/* Параметр функции	*/
	double			freqw		/* Частота, гц		*/
)
{
	/* Параметры цикла-потока	*/
	TThreadParam	*last, *fresh;  // Текущий и новый поток
	char			threadName_tmp[15] = {0};

	/* Определение имени потока */
	if (freqw == 100.0) memcpy(threadName_tmp, "100 Гц ", 8);
	if (freqw ==  25.0) memcpy(threadName_tmp, "25  Гц ", 8);
	if (freqw ==  10.0) memcpy(threadName_tmp, "10  Гц ", 8);
	if (freqw ==   1.0) memcpy(threadName_tmp, "1   Гц ", 8);
    if (freqw ==   0.5) memcpy(threadName_tmp, "0.5 Гц ", 8);


	fresh = (TThreadParam *) malloc(sizeof(TThreadParam)); //Выделение памяти для нового потока

	if (fresh == 0)
	{
		printf("Ошибка выделения памяти для потока %s !\n", threadName_tmp);

        return 1;
	}

	if (!(fresh->pFunc = pFunc))
	{
		free(fresh);
		printf("Ошибка, не задана функция потока %s !\n", threadName_tmp);

		return 1;
	}

	fresh->param 		= param;
	fresh->count 		= 0;				/* Счетчик потока	*/
	fresh->freqw 		= freqw;			/* Частота, гц		*/
	fresh->thrIdent 	= 0;		/* Идентификатор потока	*/
	fresh->delLunch 	= (int)(((double)HZ)/freqw); /* Дельта открытия семафора*/
	fresh->nxtLunch 	= 0;    /*Момент очередного открытия семафора*/
	fresh->pNextParam 	= 0;   /*Указатель на следующий элемент списка*/
	memcpy(fresh->threadName, threadName_tmp, 15);

	fresh->pSem = (sem_t *) malloc(sizeof(sem_t)); // выделение памяти под семафор

	if (fresh->pSem)
	{
		if (sem_init(fresh->pSem, 0, 0)) //инициализация семафора ( указатель на семафор, флаг, начальное значение семафора)
		{
			printf("Ошибка инициализиции семафора для потока %s !\n", threadName_tmp);
			free(fresh->pSem); // освобождение памяти под семафор
			free(fresh);

			return 1;
		}
	}
	else
	{
		printf("Ошибка выделения памяти для семафора потока %s !\n", threadName_tmp);
		free(fresh);
        return 1;
	}

	/* Добавление параметра потока в список параметров потоков */
	if (bgnThrList == 0)
		bgnThrList = fresh; // если первого потока в списке не существует, то новый поток становится первым
	else
	{
		last = bgnThrList;
        while (last->pNextParam)
			last = last->pNextParam;
	    last->pNextParam = fresh;
	}

	return 0;
}


//Функция потока, в которой закрывается семафор, вызывается функция передачи с определенной частотой и увеличичвается счетчик потока
void * cvmCircleFunc
(
	TThreadParam *pThrd
)
{
    while (pThrd->pSem)
	{   sem_wait(pThrd->pSem); //блокировка семафора
		if (pThrd->pFunc)
			(*pThrd->pFunc)(pThrd->param); //вызов функции гц с указанным параметром
		pThrd->count++; //увеличение счетчика потока
	}

    return (void *)0;
}

// Запуск диспетчера потоков проекта
void dispetcherStart(void)
{
	TThreadParam	*pThrd;		// Параметры потока
	int f = 0;

	// Создание потоков на основе параметров из списка параметров потоков
	pThrd = bgnThrList;

	while (pThrd)
	{
		if (pThrd->pFunc)
			addThreadCreate(&pThrd->thrIdent,
							(PTThreadFunc) &cvmCircleFunc,
							(void *)pThrd,
							pThrd->threadName);

		//! Функциональная вставка для запоминания идентификатора первого созданного потока
		if (!f)
		{
			f = 1;
			pId_first = pThrd->thrIdent;
		}

		pThrd = pThrd->pNextParam;
	 }

	 itimerCreate(1000000/HZ); //функция интервального таймера
}


// Вывод счетчика потока
int GetCount(void)
{
	TThreadParam *pThrd = bgnThrList;

	while (pThrd)
	{
		if (pThrd->count == 0)
		{
			printf("Ошибка чтения счетчика потока %s !\n", pThrd->threadName);
			return 1;
		}

		printf("%s счетчик потока = %i\n", pThrd->threadName, pThrd->count);
		pThrd = pThrd->pNextParam;
	}

	return 0;
}


//Функция завершения потоков
void * delThr(void)
{
	TThreadParam	*pThrd = bgnThrList;

	printf("\n\nЗавершение потоков:\n");

	while (pThrd)
	{
	  if (pthread_cancel(pThrd->thrIdent) == 0)
		 printf("Поток %s завершен!\n", pThrd->threadName);
	  else printf("Поток %s не может быть завершен!\n", pThrd->threadName);

	  pThrd = pThrd->pNextParam;
	}

	return 0;
}


//Функция очистки памяти
void CleanMem(void)
{
	TThreadParam    *pThrd = bgnThrList;
	TThreadParam    *cThrd;

	while (pThrd)
	{
		free(pThrd->pSem);
		cThrd = pThrd;
		free(cThrd);
		pThrd = pThrd->pNextParam;
	}
}
