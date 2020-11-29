#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <Windows.h>

int Num_of_threads;
int SizeOfChest, OnChest, VariableOnChest, result = 1;//N,K,L
volatile int THREADSRESULT = 1;
int additinal = 1;
bool PutFine = true;

CRITICAL_SECTION cs;


void readfile();
int putFigure(int FigureLeft);
bool checkCorrect();
struct elem
{
	int x;
	int y;
};

struct elem *Figure;

DWORD WINAPI Thread_entry(void *param)
{
	while (VariableOnChest > 1 && result > 0)
	{
		EnterCriticalSection(&cs);
		if (VariableOnChest < 1 || result < 1)
		{
			LeaveCriticalSection(&cs);
			break;
		}
		THREADSRESULT *= putFigure(VariableOnChest);
		VariableOnChest--;
		LeaveCriticalSection(&cs);
	}
	return 0;
}

int main()
{
	HANDLE *handles;
	time_t t;
	double time;
	int NumThreadsToCreate, i;
	readfile();
	PutFine = checkCorrect();
	if (!PutFine)

	{
		FILE *OutFile, *timeFile;
		if (OutFile = fopen("output.txt", "w+"))
		{
			fprintf(OutFile, "%d\n", 0);
			fclose(OutFile);
		}
		else
			printf("Error: can`t open output.txt\n");

		if (timeFile = fopen("time.txt", "w+"))
		{
			fprintf(timeFile, "%d\n", 0);
			fclose(timeFile);
		}
		else
			printf("Error: can`t open time.txt\n");
		return 0;
	}

	NumThreadsToCreate = (int)(SizeOfChest / 500);
	if (NumThreadsToCreate > 64)
		NumThreadsToCreate = 64;

	if (Num_of_threads == 1 || NumThreadsToCreate < 3)
	{
		t = clock();
		while (VariableOnChest > 1 && result > 0)
		{
			result *= putFigure(VariableOnChest);
			VariableOnChest--;
		}

		time = 1000 * ((double)clock() - (double)t) / (double)CLOCKS_PER_SEC;

		FILE *OutFile, *timeFile;
		if (OutFile = fopen("output.txt", "w+"))
		{
			fprintf(OutFile, "%d\n", result*additinal);
			fclose(OutFile);
		}
		else
			printf("Error: can`t open output.txt\n");
		if (timeFile = fopen("time.txt", "w+"))
		{
			fprintf(timeFile, "%d\n", (unsigned int)time);
			fclose(timeFile);
		}
		else
			printf("Error: can`t open time.txt\n");
		return 0;
	}

	handles = (HANDLE*)malloc(NumThreadsToCreate * sizeof(HANDLE));
	InitializeCriticalSection(&cs);

	t = clock();
	for (i = 0; i < NumThreadsToCreate; i++)
	{
		handles[i] = CreateThread(0, 0, Thread_entry, (void*)i, 0, NULL);
		if (0 == handles[i])
		{
			printf("CreateThread failed. GetLastError(): %u\n", GetLastError());
			return -1;
		}
	}
	// Îæèäàíèå çàâåðøåíèÿ ïîòîêîâ
	WaitForMultipleObjects(NumThreadsToCreate, handles, TRUE, INFINITE);
	time = 1000 * ((double)clock() - (double)t) / (double)CLOCKS_PER_SEC;
	DeleteCriticalSection(&cs);
	printf("All threads finished analyzing\n");
	// Çàêðûòèå îïèñàòåëåé ñîçäàííûõ ïîòîêîâ
	for (i = 0; i < NumThreadsToCreate; i++)
		CloseHandle(handles[i]);
	printf("%d\n", THREADSRESULT);

	FILE *OutFile, *timeFile;
	if (OutFile = fopen("output.txt", "w+"))
	{
		fprintf(OutFile, "%d\n", THREADSRESULT*additinal);
		fclose(OutFile);
	}
	else
		printf("Error: can`t open output.txt\n");
	if (timeFile = fopen("time.txt", "w+"))
	{
		fprintf(timeFile, "%d\n", (unsigned int)time);
		fclose(timeFile);
	}
	else
		printf("Error: can`t open time.txt\n");
	return 0;

}

void readfile()
{
	FILE* f;
	int i, j, sx, sy;
	if (f = fopen("input.txt", "r+"))
	{
		fscanf(f, "%d", &Num_of_threads);
		fscanf(f, "%d", &SizeOfChest);//n
		fscanf(f, "%d", &VariableOnChest);//l
		fscanf(f, "%d", &OnChest);//k

		Figure = (struct elem*)malloc(OnChest * sizeof(struct elem));

		for (i = 0; i < OnChest; i++)
		{
			fscanf(f, "%d", &sx);
			fscanf(f, "%d", &sy);
			if (sx > SizeOfChest || sy > SizeOfChest || sx < 0 || sy < 0)
			{
				PutFine = false;
				fclose(f);
				return;
			}
			Figure[i].x = sx;
			Figure[i].y = sy;
		}
		SizeOfChest -= OnChest;
		if (SizeOfChest > VariableOnChest)
		{
			additinal = SizeOfChest - VariableOnChest;
			additinal += SizeOfChest*(additinal) + (SizeOfChest - 1)*additinal;
		}
	}
	else
		printf("Error: can't open input.txt\n");
	fclose(f);
}

int putFigure(int FigureLeft)
{
	if (SizeOfChest == 1 && FigureLeft > 1)
		return 0;
	else
		return (SizeOfChest--);
}
bool checkCorrect()
{
	int i, j;
	for (i = 0; i < OnChest - 1; i++)
	{
		for (j = i + 1; j < OnChest; j++)
		{
			if (Figure[i].x == Figure[j].x || Figure[i].y == Figure[j].y)
				return false;
		}
	}
	return true;
}
