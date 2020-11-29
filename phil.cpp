#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#define num_philosophers 5
int whole_time, dining_time;
struct timespec start, finish;
int dishes[5] = { 0 };
int first = 1, second = 3, butler = 2;
sem_t semaphore, mutex;
unsigned long to_ms(struct timespec*tm) {
	return ((unsigned long)tm->tv_sec * 1000 + (unsigned long)tm->tv_nsec / 1000000);
}
void *thread_entry(void *param) {
	int idx = *(int*)param, i;
	//volatile int idx = ((char*)param - (char*)0);
	idx++;
	struct timespec local_time;
	while (1) {
		sem_wait(&semaphore);
		clock_gettime(CLOCK_REALTIME, &local_time);
		if (to_ms(&local_time) - to_ms(&start) >= whole_time) {
			printf("All filosophers finished eating\n");
			sem_post(&semaphore);
			break;
		}
		sem_wait(&mutex);
		if ((idx == first || idx == second) && butler == 2){
			clock_gettime(CLOCK_REALTIME, &local_time);
			printf("%lu:%d:T->E\n", to_ms(&local_time) - to_ms(&start), idx);
			dishes[idx - 1]++;
			sem_post(&mutex);
			usleep(dining_time);//eating
			sem_wait(&mutex);//has eaten
			clock_gettime(CLOCK_REALTIME, &local_time);
			printf("%lu:%d:E->T\n", to_ms(&local_time) - to_ms(&start), idx);
			butler--;
			if (butler == 0) {
				first++; second++;
				first = first % 6;
				second = second % 6;
				if (!first)//first==0
					first++;
				if (!second)
					second++;
				butler = 2;//+=2
				sem_post(&semaphore);
				sem_post(&semaphore);
			}
			sem_post(&mutex);
		}
		else {
			sem_post(&mutex);
			sem_post(&semaphore);
			usleep(dining_time);
		}
	}
	free(param);
	pthread_exit(NULL);
}
int main(int argc, char* argv[]) {
	if (argc != 3) {
		printf("Wrong format! Try like: phil.exe TOTAL_TIME EAT_TIME\n");
		return -1;
	}
	whole_time = atoi(argv[1]);
	dining_time = atoi(argv[2]);
	whole_time *= 1000;
	pthread_t philosophers[num_philosophers];
	sem_init(&semaphore, 0, 2);
	sem_init(&mutex, 0, 1);
	clock_gettime(CLOCK_REALTIME, &start);
	for (int i = num_philosophers-1; i!=-1; i--) {
		if (0 != pthread_create(&philosophers[i], NULL, thread_entry, new int(i)))
		{
			printf("pthread_create failed. errno: %d\n", errno);
			return -1;
		}
	}
	for (int i = 0; i < 5; i++) {
		pthread_join(philosophers[i], 0);
	}
	clock_gettime(CLOCK_REALTIME, &finish);
	printf("Execution time: %lu ms\n", to_ms(&finish) - to_ms(&start));
	for (i = 0; i < 5; i++)
	{
		printf("Philosopher %d eat  %d dishes\n", i + 1, dishes[i]);
	}
	sem_destroy(&semaphore);
	sem_destroy(&mutex);

}