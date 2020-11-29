#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
volatile int thread_amount = 0, mass_amount = 0;//1-кол-во потоков рабочего пула, 2-кол-во элементов массива
volatile int answers = 0;
int sum;
unsigned long long step;
int *numbers;
pthread_mutex_t mutex;
void init(void) {
	int i = 0;
	FILE* input = fopen("input.txt", "r+");
	if (!input) {
		printf("ERROR WITH INPUT FILE!");
	}
	else {
		fscanf(input, "%d", &thread_amount);
		fscanf(input, "%d", &mass_amount);
		numbers = (int*)malloc(mass_amount * sizeof(int));
		for(i=0;i<mass_amount;i++){
		fscanf(input, "%d", &numbers[i]);			
		}
		fscanf(input,"%d", &sum);
		fclose(input);
	}
}
void out(double time) {
	FILE* output = fopen("output.txt", "w+");
	if (output) {
		fprintf(output, "%d\n", thread_amount);
		fprintf(output, "%d\n", mass_amount);
		fprintf(output, "%d\n", answers);
		fprintf(output, "%c", '\n');
		fclose(output);
	}
	else
		printf("ERROR WITH OUTPUT.TXT!\n");
	output = fopen("time.txt", "w+");
	if (output) {
		fprintf(output, "%d\n", (unsigned int)time);
		fclose(output);
	}
	else {
		printf("ERROR WITH TIME.TXT!\n");
	}

	printf("I wrote data to output.txt\n");
}
int solo_thread(int sum_now, int idx) {
	int count = 0;
	if (idx != mass_amount - 1) {
		count += solo_thread(sum_now + numbers[idx + 1], idx + 1);
		count += solo_thread(sum_now - numbers[idx + 1], idx + 1);
	}
	else {
		if (sum_now == sum) {
			//pthread_mutex_lock(&mutex);
			count++;
			//pthread_mutex_unlock(&mutex);
		}
	}
	return count;
}
void* thread_entry(void* param)
{
	int idx = ((char*)param-(char*)0);
	int temp = pow(2.0, (double)mass_amount - 1.0);
	int j;
	unsigned long long sign, max, i;
	sign = step * idx;
	max = step;
	if (idx + 1 == thread_amount) {
		max = temp - sign;
	}
	for (i = 0; i < max; i++) {
		temp = numbers[0];
		for (j = 1; j < mass_amount; j++) {
			if (sign & (1 << mass_amount - 1 - j))
			{
				temp += numbers[j];
			}
			else {
				temp -= numbers[j];
			}
		}
		if (temp == sum) {
			pthread_mutex_lock(&mutex);
			answers++;
			pthread_mutex_unlock(&mutex);
		}
		sign++;
	}
	return 0;
}
int main() {
	pthread_t *threads;
	pthread_mutex_init(&mutex, 0);
	init();

	printf("Creating threads...\n");
	//когда один поток
	if (thread_amount == 1 || thread_amount > mass_amount) {
		time_t t = clock();
		int res = solo_thread(numbers[0], 0);
		printf("I founded answers = %d!", res);
		time_t time = 1000 * ((double)clock() - (double)t) / (double)CLOCKS_PER_SEC;
		answers=res;
		out(time);
		printf("I`m going to delete mutex and free memory\n");
		pthread_mutex_destroy(&mutex);
		//free(numbers);
		printf("All memory free\n");
		return 0;
	}
	//когда не один поток
	step = floor(pow(2.0, (double)mass_amount - 1.0) / (double)thread_amount); //шаг=кол-во всевозм исходов поделим на кол-во тредов
	threads=(pthread_t*)malloc(thread_amount*sizeof(pthread_t));
	//надо впихнуть мьютекс в тред интрай!!!
	//pthread_mutex_lock(&mutex);
	for (int i = 0; i < thread_amount; i++) {
		if (0 != pthread_create(&threads[i], 0, thread_entry, (void *)((char *)0 + i)))
		{
			printf("pthread_create failed. errno: %d\n", errno);
			return -1;
		}
	}
	printf("Threads created\n");
	time_t time = clock();
	for (int i = 0; i < thread_amount; i++)
		pthread_join(threads[i], 0);
	double work_time = 1000 * ((double)clock() - (double)time) / (double)CLOCKS_PER_SEC;
	//pthread_mutex_unlock(&mutex);
	out(work_time);

	printf("I`m going to delete mutex and free memory\n");
	pthread_mutex_destroy(&mutex);
	//free(numbers);
	free(threads);
	printf("All memory free\n");
	return 0;
}
