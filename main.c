#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include "malloc.h"

#define TIME_TEST(testCode, time) { \
    clock_t start_time = clock(); \
    testCode \
        clock_t end_time = clock(); \
    clock_t sort_time = end_time - start_time; \
    time = (double) sort_time / CLOCKS_PER_SEC; \
    }

#define ARRAY_SIZE(a) sizeof(a) / sizeof(a[0])

// функция сортировки
typedef struct SortFunc {
    void (*sort )(int *a, size_t n); // указатель на функцию сортировки
    char name[64];                   // имя сортировки, используемое при выводе
} SortFunc;

// функция генерации
typedef struct GeneratingFunc {
    void (*generate )(int *a, size_t n); // указатель на функцию генерации последовательности
    char name[64];                       // имя генератора, используемое при выводе
} GeneratingFunc;

bool isOrdered(const int *a, size_t size) {
    for (int i = 0; i < size - 1; i++)
        if (a[i] > a[i + 1])
            return false;

    return true;
}

swap(int *a, int *b) {
    int t = *a;
    *a = *b;
    *b = t;
}

void outputArray_(const int *a, size_t size) {
    for (size_t i = 0; i < size; i++)
        printf("%d ", a[i]);
    printf("\n");
}

void checkTime(void (*sortFunc)(int *, size_t),
               void (*generateFunc)(int *, size_t),
               size_t size, char *experimentName) {
    static size_t runCounter = 1;

    // генерация последовательности
    static int innerBuffer[100000];
    generateFunc(innerBuffer, size);
    printf("Run #%zu| ", runCounter++);
    printf("Name: %s\n", experimentName);

    // замер времени
    double time;
    TIME_TEST({ sortFunc(innerBuffer, size); }, time);

    // результаты замера
    printf("Status: ");
    if (isOrdered(innerBuffer, size)) {
        printf("OK! Time : %.3f s.\n", time);

        // запись в файл
        char filename[256];
        sprintf(filename, "./data/%s.csv", experimentName);
        FILE *f = fopen(filename, "a");
        if (f == NULL) {
            printf("FileOpenError %s", filename);
            exit(1);
        }
        fprintf(f, "%zu; %.3f\n", size, time);
        fclose(f);
    } else {
        printf("Wrong!\n");

        // вывод массива, который не смог быть отсортирован
        outputArray_(innerBuffer, size);

        exit(1);
    }
}


//сортировка подсчётом (выбрана вместо пузырьковой)
void getMinMax(const int *a, size_t size, int *min, int *max) {
    *min = a[0];
    *max = a[0];
    for (int i = 1; i < size; i++) {
        if (a[i] < *min)
            *min = a[i];
        else if (a[i] > *max)
            *max = a[i];
    }
}

void countSort(int *a, const size_t size) {
    int min, max;
    getMinMax(a, size, &min, &max);
    int k = max - min + 1;

    // выделение памяти под динамический массив из k элементов,
    // где каждый из элементов равен 0
    int *b = (int *) calloc(k, sizeof(int));
    for (int i = 0; i < size; i++)
        b[a[i] - min]++;

    int ind = 0;
    for (int i = 0; i < k; i++) {
        int x = b[i];
        for (int j = 0; j < x; j++)
            a[ind++] = min + i;
    }

    // освобождение памяти, выделенной под динамический массив
    free(b);
}

void selectionSort(int *a, size_t size) {
    for (int i = 0; i < size - 1; i++) {
        int minPos = i;
        for (int j = i + 1; j < size; j++)
            if (a[j] < a[minPos])
                minPos = j;
        swap(&a[i], &a[minPos]);
    }
}

void insertionSort(int *a, size_t size) {
    for (size_t i = 1; i < size; i++) {
        int t = a[i];
        size_t j = i;
        while (j > 0 && a[j - 1] > t) {
            a[j] = a[j - 1];
            j--;
        }
        a[j] = t;
    }
}

void combSort(int *a, const size_t size) {
    size_t step = size;
    int swapped = 1;
    while (step > 1 || swapped) {
        if (step > 1)
            step /= 1.24733;
        swapped = 0;
        for (size_t i = 0, j = i + step; j < size; ++i, ++j)
            if (a[i] > a[j]) {
                swap(&a[i], &a[j]);
                swapped = 1;
            }
    }
}

void shellSort(int *a, size_t size) {
    for (size_t step = size / 2; step > 0; step /= 2)
        for (size_t i = step; i < size; i++) {
            int tmp = a[i];
            size_t j;
            for (j = i; j >= step; j -= step) {
                if (tmp < a[j - step])
                    a[j] = a[j - step];
                else
                    break;
            }
            a[j] = tmp;
        }
}

int digit(int n, int k, int N, int M) {
    return (n >> (N * k) & (M - 1));
}

void _radixSort(int *l, int *r, int N) {
    int k = (32 + N - 1) / N;
    int M = 1 << N;
    int sz = r - l;
    int *b = (int *) malloc(sizeof(int) * sz);
    int *c = (int *) malloc(sizeof(int) * M);
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < M; j++)
            c[j] = 0;

        for (int *j = l; j < r; j++)
            c[digit(*j, i, N, M)]++;

        for (int j = 1; j < M; j++)
            c[j] += c[j - 1];

        for (int *j = r - 1; j >= l; j--)
            b[--c[digit(*j, i, N, M)]] = *j;

        int cur = 0;
        for (int *j = l; j < r; j++)
            *j = b[cur++];
    }
    free(b);
    free(c);
}

void radixSort(int *a, size_t n) {
    _radixSort(a, a + n, 8);
}

/* sort unsigned ints */
static void rad_sort_u(unsigned *begin, unsigned *end, unsigned bit) {
    if (!bit || end < begin + 1)
        return;

    unsigned *ll = begin;
    unsigned *rr = end - 1;
    while (1) {
        while (ll < rr && !(*ll & bit))
            ll++;
        while (ll < rr && (*rr & bit))
            rr--;
        if (ll >= rr)
            break;
        swap(ll, rr);
    }

    if (!(bit & *ll) && ll < end)
        ll++;

    bit >>= 1;

    rad_sort_u(begin, ll, bit);
    rad_sort_u(ll, end, bit);
}

/* sort signed ints: flip highest bit, sort as unsigned, flip back */
static void radix_sort(int *a, const size_t size) {
    unsigned *x = (unsigned *) a;

    for (size_t i = 0; i < size; i++)
        x[i] ^= INT_MIN;

    rad_sort_u(x, x + size, INT_MIN);

    for (size_t i = 0; i < size; i++)
        x[i] ^= INT_MIN;
}

int cmp_int(const void *a, const void *b) {
    return *(const int *) a - *(const int *) b;
}

int cmp_intReverse(const void *a, const void *b) {
    return *(const int *) b - *(const int *) a;
}


void qSort(int *a, size_t size) {
    qsort(a, size, sizeof(int), cmp_int);
}

void generateRandomArray(int *a, size_t size) {
    srand(time(0));
    for (int i = 0; i < size; i++)
        a[i] = rand() % 10000 - 5000;
}

void generateOrderedArray(int *a, size_t size) {
    generateRandomArray(a, size);
    qsort(a, size, sizeof(int), cmp_int);
}



void generateOrderedBackwards(int *a, size_t size) {
    generateRandomArray(a, size);
    qsort(a, size, sizeof(int), cmp_intReverse);
}

void timeExperiment() {
    SortFunc sorts[] = {
            {countSort, "countSort"},
            {selectionSort, "selectionSort"},
            {insertionSort, "insertionSort"},
            {shellSort, "shellSort"},
            {radix_sort, "radix_sort"},
            {qSort, "qSort"},
    };
    const unsigned FUNCS_N = ARRAY_SIZE(sorts);

    // описание функций генерации
    GeneratingFunc generatingFuncs[] = {
            {generateRandomArray,      "random"},
            {generateOrderedArray,     "ordered"},
            {generateOrderedBackwards, "orderedBackwards"}
    };
    const unsigned CASES_N = ARRAY_SIZE(generatingFuncs);

    // запись статистики в файл
    for (size_t size = 10000; size <= 100000; size += 10000) {
        printf(" - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
        printf("Size : %d\n", size);
        for (int i = 0; i < FUNCS_N; i++) {
            for (int j = 0; j < CASES_N; j++) {
                // генерация имени файла
                static char filename[128];
                sprintf(filename, "%s_%s_time",
                        sorts[i].name, generatingFuncs[j].name);
                checkTime(sorts[i].sort,
                          generatingFuncs[j].generate,
                          size, filename);
            }
        }
        printf("\n");
    }
}

int main (){
    timeExperiment();
}
