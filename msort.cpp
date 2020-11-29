#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
volatile int thread_amount = 0, mass_amount = 0;//1-кол-во потоков рабочего пула, 2-кол-во элементов массива
volatile int check_int;
int* volatile mass;
HANDLE *threads;
HANDLE mutex;
void init(void);       //инициализация
void out(double time); //вывод в файл времени
void mergesort(int *mass, int number); //mergesort
DWORD WINAPI thread_entry(void*param); //сама функция для нитей
typedef struct {
	volatile int size;
	int*volatile data;
}thread_work;
thread_work *thread_state;
void init(void) {
	int i = 0;
	FILE* input = fopen("input.txt", "r+");
	if (!input) {
		printf("ERROR WITH INPUT FILE!");
	}
	else {
		fscanf(input, "%d", &thread_amount);
		fscanf(input, "%d", &mass_amount);
		mass = (int*)malloc(mass_amount * sizeof(int));
		thread_state = (thread_work*)malloc(thread_amount * sizeof(thread_work));
		while (fscanf(input, "%d", &mass[i]) != EOF) {
			i++;
		}
		fclose(input);
	}
}
void out(double time) {
	FILE* output = fopen("output.txt", "w+");
	if (output) {
		fprintf(output, "%d\n", thread_amount);
		fprintf(output, "%d\n", mass_amount);
		for (int i = 0; i < mass_amount; i++) {
			fprintf(output, "%d ", mass[i]);
		}
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

	printf("I wrote mass to output.txt\n");
}
void mergesort(int *mass, int number) {
	int step = 1;
	int *temp = (int*)malloc(number * sizeof(temp));//доп массив
	while (step < number) {
		int idx = 0, left = 0;
		int middle = left + step;
		int right = left + step*2;
		do {
			if (middle < number) 
				middle = middle;//ATTENTION
			else 
				middle = number;
			if (right < number)
				right = right;
			else right = number;
			int idx1 = left, idx2 = middle;//индексы сравн эл-ов
			for (; idx1 < middle && idx2 < right;)
			{
				if (mass[idx1] < mass[idx2])
					temp[idx++] = mass[idx1++];
				else
					temp[idx++] = mass[idx2++];
			}
			//выбираем один из whilов
			while (idx1 < middle)
				temp[idx++] = mass[idx1++];
			while (idx2 < right)
				temp[idx++] = mass[idx2++];
			left += step * 2;
			middle += step * 2;
			right += step * 2;
		} while (left < number);
		for (int i = 0; i < number; i++)
			mass[i] = temp[i];	
		step *= 2;
	}

}

DWORD WINAPI thread_entry(void* volatile param) {
	volatile int index = ((char*)param - (char*)0) + 1;  // Thread number 
	volatile int counter = 0, i = 0, ctr_num;
	if (index == thread_amount) {

		thread_state[index - 1].size = mass_amount - (int)(mass_amount / thread_amount)*(thread_amount - 1);
		thread_state[index - 1].data = (int *)malloc(thread_state[index - 1].size * sizeof(int));

		ctr_num = mass_amount - (index - 1) * (int)(mass_amount / thread_amount);
		while (counter != thread_state[index - 1].size  && ctr_num != mass_amount)
		{
			thread_state[index - 1].data[counter] = mass[ctr_num];
			ctr_num++;
			counter++;
		}
		printf("Thread %d copied ORIGINAL data\n", index);
	}
	else
	{
		thread_state[index - 1].size = (int)(mass_amount / thread_amount);
		thread_state[index - 1].data = (int *)malloc(thread_state[index - 1].size * sizeof(int));

		ctr_num = (index - 1) * (int)(mass_amount / thread_amount);
		while (counter != thread_state[index - 1].size)
		{
			thread_state[index - 1].data[counter] = mass[ctr_num];
			ctr_num++;
			counter++;
		}
	}
	printf("Thread %d have %d elements\n", index, thread_state[index - 1].size);
	printf("Thread %d successfull allocated memory\n", index);
	mergesort(thread_state[index - 1].data,thread_state[index - 1].size);
	printf("Thread %d sorted data\n", index);

	WaitForSingleObject(mutex,INFINITE);
	if (index == thread_amount)
		ctr_num = mass_amount - (index - 1) * (int)(mass_amount / thread_amount);
	else
		ctr_num = (index - 1) * (int)(mass_amount / thread_amount);

	for (i = 0; i < thread_state[index - 1].size; i++)
	{
		mass[ctr_num] = thread_state[index - 1].data[i];
		ctr_num++;
	}
	printf("Thread %d copied data\n", index);
	ReleaseMutex(mutex);

	if (index == thread_amount && index != 1)
	{
		WaitForMultipleObjects(thread_amount - 1, threads, TRUE, INFINITE);
		mergesort(mass,mass_amount);
		printf("All data sorted\n");
	}
	return 0;
}


int main() {
	init();
	threads = (HANDLE*)malloc(thread_amount * sizeof(HANDLE));
	mutex = CreateMutex(NULL, FALSE, NULL);
	printf("Creating threads...\n");
	//если поток один или кол-во потоков больше(=) колва эл-ов массива
	if (thread_amount > mass_amount || thread_amount == 1) {
		time_t time = clock();
		mergesort(mass, mass_amount);
		printf("I`m sorted your mass\n");
		double work_time = 1000 * ((double)clock() - (double)time) / (double)CLOCKS_PER_SEC;
		out(work_time);
		CloseHandle(mutex);
		printf("I`m going to free using memory");
		free(mass);
		free(thread_state);
		printf("MEMORY IS FREE");
		return 0;
	}
	for (int i = 0; i < thread_amount; i++) {
		threads[i] = CreateThread(0, 0, thread_entry, (void*volatile)((char*)0 + i), 0, 0);
		if (threads[i] == 0) {
			printf("CREATING THREAD IS FAILED.GetLastError(): %u\n", GetLastError());
			return -1;
		}
	}

	printf("Creating threads is finished!\n");
	time_t time = clock();
	WaitForMultipleObjects(thread_amount, threads, TRUE, INFINITE);
	double work_time = 1000 * ((double)clock() - (double)time) / (double)CLOCKS_PER_SEC;
	out(work_time);
	for (int i = 0; i < thread_amount; i++) {
		free(thread_state[i].data);
		CloseHandle(threads[i]);
	}
	printf("I`m going to delete mutex\n");
	CloseHandle(mutex);
	free(mass);
	free(thread_state);
	printf("All memory free\n");
	return 0;
}
