#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
volatile int thread_amount = 0, mass_amount = 0;//1-кол-во потоков рабочего пула, 2-кол-во элементов массива
int* volatile mass;
HANDLE *threads; 
CRITICAL_SECTION cs;
void init(void);       //инициализация
void out(double time); //вывод в файл времени
void quicksort(int *mass, int left, int right); //quicksort
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
void quicksort(int* mass, int left, int right) {
	volatile int middle, i, j, buf;
	if (left < right) {
		middle = left;
		i = left;
		j = right;
		while (i < j) {
			while (mass[i] <= mass[middle] && i <= right) {
				i++;
			}
			while (mass[j] > mass[middle] && j > left) {
				j--;
			}
			if (i < j) {
				buf = mass[i];
				mass[i] = mass[j];
				mass[j] = buf;
			}
		}
		buf = mass[j];
		mass[j] = mass[middle];
		mass[middle] = buf;
		quicksort(mass, left, j - 1);
		quicksort(mass, j + 1, right);

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
	quicksort(thread_state[index - 1].data, 0, thread_state[index - 1].size - 1);
	printf("Thread %d sorted data\n", index);

	WaitForSingleObject(&cs, INFINITE);
	EnterCriticalSection(&cs);
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
	LeaveCriticalSection(&cs);

	if (index == thread_amount && index != 1)
	{
		WaitForMultipleObjects(thread_amount - 1, threads, TRUE, INFINITE);
		//EnterCriticalSection(&cs);
		quicksort(mass, 0, mass_amount - 1);
		//LeaveCriticalSection(&cs);
		printf("All data sorted\n");
	}

	//ExitThread(0);
	return 0;
}


int main() {
	init();
	threads = (HANDLE*)malloc(thread_amount * sizeof(HANDLE));
	InitializeCriticalSection(&cs);
	printf("Creating threads...\n");
	//если поток один или кол-во потоков больше(=) колва эл-ов массива
	if (thread_amount > mass_amount || thread_amount == 1) {
		time_t time = clock();
		quicksort(mass, 0, mass_amount - 1);
		printf("I`m sorted your mass\n");		
		double work_time = 1000 * ((double)clock() - (double)time) / (double)CLOCKS_PER_SEC;	
		out(work_time);
		printf("blyat\n");
		DeleteCriticalSection(&cs);
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
	//EnterCriticalSection(&cs);
	double work_time = 1000 * ((double)clock() - (double)time) / (double)CLOCKS_PER_SEC;
	//LeaveCriticalSection(&cs);
	out(work_time);
	for (int i = 0; i < thread_amount; i++) {
		free(thread_state[i].data);
		CloseHandle(threads[i]);
	}
	printf("I`m going to delete critical section\n");
	DeleteCriticalSection(&cs);
	free(mass);
	free(thread_state);
	printf("All memory free\n");
	return 0;
}
