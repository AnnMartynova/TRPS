#include <stdio.h>
#include <stdlib.h>
#include <dispetcher.h>

pthread_t pId_first; // объявление TID первого потока

//usr_100HzFunc - функция передачи данных с частотой 100 гц (вместo printf добавляется функция transmission)
 void usr_100HzFunc(void)
{
	// Catch and ignoring signal "SIGPIPE"- процесс записал информацию в конвейер, но считывателей не было. сигнал завершения процесса
	struct sigaction act; // детализация контроля над сигналами

	act.sa_handler = SIG_IGN;  //действие, которое должно быть осуществлено после получения сигнала.  SIG_IGN - ядро игнорирует указанный в signo сигнал для процесса
	sigemptyset(&act.sa_mask); // sa_mask - поле для указания сигналов которые нужно блокировать. функция исключает из набора все сигналы и помечает как пустое множество
	act.sa_flags = 0; // установка нулевого флага
	sigaction(SIGPIPE, &act, 0); // изменение поведение сигнала SIGPIPE (аргумент signo) на поведение указанное в act, а oldact =0 сохраняет актуальное поведение сигнала

    printf("100Гц\t");
}

// usr_25HzFunc - функция передачи данных с частотой 25 гц
void usr_25HzFunc(void)
{
	struct sigaction act;

	act.sa_handler = SIG_IGN;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGPIPE, &act, 0);

    printf("25 Гц\t");
}

// usr_10HzFunc - функция передачи данных с частотой 10 гц
void usr_10HzFunc(void)
{
	struct sigaction act;

	act.sa_handler = SIG_IGN;
	sigemptyset(&act.sa_mask);
	sigaction(SIGPIPE, &act, 0);

	printf("10 Гц\t");

}

// usr_1HzFunc - функция передачи данных с частотой 1 гц
void usr_1HzFunc(void)
{
	struct sigaction act;

	act.sa_handler = SIG_IGN;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGPIPE, &act, 0);

	 printf("1  Гц\t");
}

// usr_1HzFunc - функция передачи данных с частотой 0,5 гц
void usr_05HzFunc(void)
{
    struct sigaction act;

    act.sa_handler = SIG_IGN;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGPIPE, &act, 0);

    printf("0.5Гц\t");
}


// Головная функция программного комплекса
int main(void)
{
	//Установка параметров потоков и построенеи списка
	threadParamSet((TUsrCircleFunc *)&usr_100HzFunc, NULL, 100.0);
	threadParamSet((TUsrCircleFunc *)&usr_25HzFunc, NULL, 25.0);
	threadParamSet((TUsrCircleFunc *)&usr_10HzFunc, NULL, 10.0);
	threadParamSet((TUsrCircleFunc *)&usr_1HzFunc, NULL, 1.0);
    threadParamSet((TUsrCircleFunc *)&usr_05HzFunc, NULL, 0.5);

    //Вызов диспетчера потоков
	dispetcherStart();

	//Изменение поведения сигнала при нажатии клавиш Ctrl+C (имимтация завершения работы бортового оборудования)
	struct sigaction endThr;

	endThr.sa_handler = (void (*)(int))&delThr;
	sigemptyset(&endThr.sa_mask);
    endThr.sa_flags = 0;
	sigaction(SIGINT, &endThr, 0);

	// Присоединение потоков, блокировка main до завершения pId_first - первого созданного потока
	pthread_join(pId_first, NULL);

	//Вывести счетчик потока (исключаемая функция)
	GetCount();

	//Очистка памяти
    CleanMem();

	return 0;
}
