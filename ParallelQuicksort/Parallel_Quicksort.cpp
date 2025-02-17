#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <iostream>

#define TASK_SIZE 100

using std::nothrow;
using std::cin;
using std::cout;
using std::endl;

unsigned int rand_interval(unsigned int min, unsigned int max)
{
    int r;
    const unsigned int range = 1 + max - min;
    const unsigned int buckets = RAND_MAX / range;
    const unsigned int limit = buckets * range;

    do
    {
        r = rand();
    } while (r >= limit);

    return min + (r / buckets);
}

void fillupRandomly(int* m, int size, unsigned int min, unsigned int max) {
    for (int i = 0; i < size; i++)
        m[i] = rand_interval(min, max);
}

void fillUp(int* m, int size) {
    cout << "Please, input elements of array: ";
    for (int i = 0; i < size; i++)
        cin >> m[i];
}

void init(int* a, int size) {
    for (int i = 0; i < size; i++)
        a[i] = 0;
}

void printArray(int* a, int size) {
    for (int i = 0; i < size; i++)
        cout << a[i] << ' ';
    cout << endl;;
}

int isSorted(int* a, int size) {
    for (int i = 0; i < size - 1; i++)
        if (a[i] > a[i + 1])
            return 0;
    return 1;
}

void swap(int* a, int* b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

//Partitioning, using partiotion algorithm by Lomuto
int partition(int arr[], int low, int high)
{
    int pivot = arr[high]; // pivot
    int i = (low - 1); // Index of smaller element
    for (int j = low; j <= high - 1; j++)
    {
        if (arr[j] <= pivot)
        {
            i++; // increment index of smaller element
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

void quicksort(int* a, int low, int high)
{
    int div;

    if (low < high) {
        div = partition(a, low, high);
#pragma omp task shared(a) if(high - low > TASK_SIZE) 
        quicksort(a, low, div - 1);
#pragma omp task shared(a) if(high - low > TASK_SIZE)
        quicksort(a, div + 1, high);
    }
}

int main(int argc, char* argv[])
{
    //srand(123456);
    int N;
    cout << "Please, input size of array: ";
    cin >> N;
    int numThreads;
    cout << "Please, input number of threads: ";
    cin >> numThreads; // = (argc > 3) ? atoi(argv[3]) : 2;
    int* X = new(nothrow)int[N * sizeof(int)];
    int* tmp = new(nothrow)int[N * sizeof(int)];

    omp_set_dynamic(0);              /** Explicitly disable dynamic teams **/
    //omp_set_num_threads(numThreads); /** Use N threads for all parallel regions **/

     // Dealing with fail memory allocation
    if (!X || !tmp)
    {
        if (X) delete[] X;
        if (tmp) delete[] tmp;
        return (EXIT_FAILURE);
    }

    fillupRandomly(X, N, 0, 1000);
    //fillUp(X, N);

    double begin = omp_get_wtime();

#pragma omp parallel
    {
#pragma omp single
        quicksort(X, 0, N - 1);
    }
    printArray(X, N);
    cout << endl;
    double end = omp_get_wtime();
    printf("Time: %f (s) \n", end - begin);

    assert(1 == isSorted(X, N));

    delete[] X;
    delete[] tmp;
    return (EXIT_SUCCESS);

    return 0;
}
