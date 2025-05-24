#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct Node {
    char* path;
    time_t time;
    struct stat meta;
    struct Node* pre;
    struct Node* next;
} Node;

typedef struct Hashst {
    Node* node;
    struct Hashst* next;
} Hashst;

typedef struct LRUCache {
    size_t amount;
    time_t timeout;
    size_t n;
    Node* head;
    Node* tail;
    Hashst** arr;
    size_t arr_size;
} LRUCache;

static unsigned int hash(const char* str, size_t arr_size) {
    unsigned int hash = 5381;
    while (*str) hash = (hash << 5) + hash + *str++;
    return hash % arr_size;
}

static Node* create_node(const char* path) {
    Node* node = malloc(sizeof(Node));
    if (!node) return NULL;
    
    node->path = strdup(path);
    if (!node->path) {
        free(node);
        return NULL;
    }
    
    node->time = time(NULL);
    if (stat(path, &node->meta) != 0) {
        free(node->path);
        free(node);
        return NULL;
    }
    
    node->pre = node->next = NULL;
    return node;
}

static void move_to_front(LRUCache* cache, Node* node) {
    if (node == cache->head) return;
    
    if (node->pre) node->pre->next = node->next;
    if (node->next) node->next->pre = node->pre;
    else cache->tail = node->pre;
    
    node->next = cache->head;
    node->pre = NULL;
    if (cache->head) cache->head->pre = node;
    cache->head = node;
    if (!cache->tail) cache->tail = node;
}

static void remove_node(LRUCache* cache, Node* node) {
    unsigned int ind = hash(node->path, cache->arr_size);
    Hashst* st = cache->arr[ind];
    Hashst* pre = NULL;
    
    while (st && st->node != node) {
        pre = st;
        st = st->next;
    }
    
    if (st) {
        if (pre) pre->next = st->next;
        else cache->arr[ind] = st->next;
        free(st);
    }
    
    if (node->pre) node->pre->next = node->next;
    else cache->head = node->next;
    if (node->next) node->next->pre = node->pre;
    else cache->tail = node->pre;
    
    free(node->path);
    free(node);
    cache->n--;
}

LRUCache* create_lru(size_t amount, time_t timeout) {
    LRUCache* cache = malloc(sizeof(LRUCache));
    if (!cache) return NULL;
    
    cache->amount = amount;
    cache->timeout = timeout;
    cache->n = 0;
    cache->head = cache->tail = NULL;
    cache->arr_size = amount * 2 + 1;
    cache->arr = calloc(cache->arr_size, sizeof(Hashst*));
    if (!cache->arr) {
        free(cache);
        return NULL;
    }
    
    return cache;
}

void free_lru(LRUCache* cache) {
    if (!cache) return;
    
    Node* current = cache->head;
    while (current) {
        Node* next = current->next;
        remove_node(cache, current);
        current = next;
    }
    
    free(cache->arr);
    free(cache);
}

int add_st(LRUCache* cache, const char* path) {
    if (!cache || !path) return -1;
    
    unsigned int ind = hash(path, cache->arr_size);
    Hashst* st = cache->arr[ind];
    
    while (st) {
        if (strcmp(st->node->path, path) == 0) {
            st->node->time = time(NULL);
            if (stat(path, &st->node->meta) != 0) return -1;
            move_to_front(cache, st->node);
            return 0;
        }
        st = st->next;
    }
    
    if (cache->n >= cache->amount) {
        remove_node(cache, cache->tail);
    }
    
    Node* node = create_node(path);
    if (!node) return -1;
    
    Hashst* new_st = malloc(sizeof(Hashst));
    if (!new_st) {
        free(node->path);
        free(node);
        return -1;
    }
    
    new_st->node = node;
    new_st->next = cache->arr[ind];
    cache->arr[ind] = new_st;
    
    move_to_front(cache, node);
    cache->n++;
    return 0;
}

Node* search_st(LRUCache* cache, const char* path) {
    if (!cache || !path) return NULL;
    
    unsigned int ind = hash(path, cache->arr_size);
    Hashst* st = cache->arr[ind];
    
    while (st) {
        if (strcmp(st->node->path, path) == 0) {
            st->node->time = time(NULL);
            move_to_front(cache, st->node);
            return st->node;
        }
        st = st->next;
    }
    
    return NULL;
}

int remove_st(LRUCache* cache, const char* path) {
    if (!cache || !path) return -1;
    
    unsigned int ind = hash(path, cache->arr_size);
    Hashst* st = cache->arr[ind];
    
    while (st) {
        if (strcmp(st->node->path, path) == 0) {
            remove_node(cache, st->node);
            return 0;
        }
        st = st->next;
    }
    
    return -1;
}

void remove_expired(LRUCache* cache) {
    if (!cache || !cache->timeout) return;
    
    time_t now = time(NULL);
    Node* node = cache->tail;
    
    while (node && (now - node->time) > cache->timeout) {
        Node* pre = node->pre;
        remove_node(cache, node);
        node = pre;
    }
}

void print_node(Node* node) {
    if (!node) {
        printf("Not found\n");
        return;
    }
    printf("File: %s\n", node->path);
    printf("time: %ld\n", node->time);
    printf("Size: %ld bytes\n", node->meta.st_size);
    printf("Last modified: %ld\n", node->meta.st_mtime);
    printf("Inode: %ld\n", node->meta.st_ino);
    printf("------------------------\n");
}

int main() {
    LRUCache* cache = create_lru(3, 5);
    if (!cache) {
        printf("Failed to create cache\n");
        return 1;
    }

    FILE* f1 = fopen("test1.txt", "w");
    FILE* f2 = fopen("test2.txt", "w");
    FILE* f3 = fopen("test3.txt", "w");
    
    if (!f1 || !f2 || !f3) {
        printf("Failed to create test files\n");
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        if (f3) fclose(f3);
        free_lru(cache);
        return 1;
    }
    fclose(f1); fclose(f2); fclose(f3);

    printf("Adding files...\n");
    add_st(cache, "test1.txt");
    add_st(cache, "test2.txt");
    add_st(cache, "test3.txt");

    printf("\nSearching files...\n");
    print_node(search_st(cache, "test1.txt"));
    print_node(search_st(cache, "test2.txt"));

    printf("\nWaiting 6 seconds...\n");
    sleep(6);

    printf("\nUpdating test1.txt...\n");
    add_st(cache, "test1.txt");

    printf("\nRemoving expired entries...\n");
    remove_expired(cache);

    printf("\nRemoving test2.txt...\n");
    remove_st(cache, "test2.txt");

    printf("\nSearching removed file...\n");
    print_node(search_st(cache, "test2.txt"));

    free_lru(cache);
    unlink("test1.txt");
    unlink("test2.txt");
    unlink("test3.txt");

    return 0;
}