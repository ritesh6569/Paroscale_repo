#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define THREADS 8
#define HASH_SIZE 1000

typedef struct HashNode {
    int num;
    struct HashNode* next;
} HashNode;

typedef struct ThreadArg {
    int* nums;
    long st;
    long end;
    HashNode** arr;
    HANDLE mutex;
} ThreadArg;

unsigned int hash(int num) {
    return (unsigned int)(num ^ (num >> 16)) % HASH_SIZE;
}

int add(HashNode** arr, HANDLE mutex, int num) {
    WaitForSingleObject(mutex, INFINITE);
    int index = hash(num);
    HashNode* node = arr[index];
    while (node) {
        if (node->num == num) {
            ReleaseMutex(mutex);
            return 1;
        }
        node = node->next;
    }
    node = malloc(sizeof(HashNode));
    if (!node) {
        ReleaseMutex(mutex);
        return 0;
    }
    node->num = num;
    node->next = arr[index];
    arr[index] = node;
    ReleaseMutex(mutex);
    return 0;
}

void free_arr(HashNode** arr) {
    for (int i = 0; i < HASH_SIZE; i++) {
        HashNode* node = arr[i];
        while (node) {
            HashNode* temp = node;
            node = node->next;
            free(temp);
        }
    }
    free(arr);
}

int read(char* name, int** nums, long* count) {
    FILE* file = fopen(name, "r");
    if (!file) return 1;
    long size = 100;
    int* temp = malloc(size * sizeof(int));
    if (!temp) {
        fclose(file);
        return 1;
    }
    long cnt = 0;
    int num;
    while (fscanf(file, "%d", &num) == 1) {
        if (cnt >= size) {
            size *= 2;
            int* new_temp = realloc(temp, size * sizeof(int));
            if (!new_temp) {
                free(temp);
                fclose(file);
                return 1;
            }
            temp = new_temp;
        }
        temp[cnt++] = num;
    }
    fclose(file);
    *nums = temp;
    *count = cnt;
    return 0;
}

DWORD WINAPI process_nums(LPVOID arg) {
    ThreadArg* t = (ThreadArg*)arg;
    for (long i = t->st; i < t->end; i++) {
        add(t->arr, t->mutex, t->nums[i]);
    }
    return 0;
}

int main() {
    char name[256];
    printf("Enter file name: ");
    scanf("%255s", name);
    int* nums;
    long cnt;
    if (read(name, &nums, &cnt)) {
        printf("Error reading file\n");
        return 1;
    }
    if (cnt == 0) {
        printf("File is empty\n");
        free(nums);
        return 0;
    }
    HashNode** arr = calloc(HASH_SIZE, sizeof(HashNode*));
    HANDLE mutex = CreateMutex(NULL, FALSE, NULL);
    if (!arr || !mutex) {
        if (arr) free(arr);
        if (mutex) CloseHandle(mutex);
        if (nums) free(nums);
        return 1;
    }
    HANDLE thread_array[THREADS];
    ThreadArg args[THREADS];
    long pre_th = (cnt + THREADS - 1) / THREADS;
    for (int i = 0; i < THREADS; i++) {
        args[i].nums = nums;
        args[i].st = i * pre_th;
        args[i].end = (i == THREADS - 1) ? cnt : (i + 1) * pre_th;
        args[i].arr = arr;
        args[i].mutex = mutex;
        thread_array[i] = CreateThread(NULL, 0, process_nums, &args[i], 0, NULL);
        if (!thread_array[i]) {
            for (int j = 0; j < i; j++) CloseHandle(thread_array[j]);
            free_arr(arr);
            CloseHandle(mutex);
            free(nums);
            return 1;
        }
    }
    WaitForMultipleObjects(THREADS, thread_array, TRUE, INFINITE);
    printf("num of threads used: %d\n", THREADS);
    printf("Unique nums:\n");
    int count = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        for (HashNode* node = arr[i]; node; node = node->next) {
            printf("%d\n", node->num);
            count++;
        }
    }
    printf("Total unique nums: %d\n", count);
    for (int i = 0; i < THREADS; i++) CloseHandle(thread_array[i]);
    free_arr(arr);
    CloseHandle(mutex);
    free(nums);
    return 0;
}