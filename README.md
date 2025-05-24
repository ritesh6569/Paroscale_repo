# LRU

## Data Structures:
Doubly-linked list node (file path, timestamp, stat metadata, prev/next pointers).
Hashst: Hash table entry (node pointer, next for collision chaining).
LRUCache: Manages cache (capacity, timeout, node count, head/tail pointers, hash table).
Libraries: stdio.h, stdlib.h, string.h, time.h, sys/stat.h, unistd.h.

## Components:
Hash table for O(1) lookups.
Doubly-linked list for O(1) LRU updates.
File system interaction (stat, fopen, unlink).

## Algorithm
Hash Function: DJB2 (hash = (hash * 33) + char) modulo table size.
Create Node: Allocates node, duplicates path, fetches metadata, sets timestamp.
Move to Front: Moves node to list head (O(1)).
Remove Node: Removes from hash table and list, frees memory (O(1) average).
Create LRU Cache: Initializes cache with capacity, timeout, and hash table.
Free LRU Cache: Frees all nodes and cache.
Add Entry: Updates existing or adds new node; evicts tail if full (O(1) average).
Search Entry: Looks up node, updates timestamp, moves to front (O(1) average).
Remove Entry: Deletes node by path (O(1) average).
Remove Expired: Removes nodes from tail where now - time > timeout (O(n)).
Main: Tests cache with file creation, addition, search, expiration, and cleanup.

## Summary
Purpose: LRU Cache for file metadata with capacity and timeout.
Complexity: O(1) average for add/search/remove; O(n) for expiration.
Space: O(n) for nodes and hash table.

## How to run
### compile : gcc -o cache cache.c
### Run : ./cache.exe


# Multi-Threading

## Data Structures:
HashNode: Singly-linked list node (integer num, next pointer).
ThreadArg: Thread args (numbers array, start/end indices, hash table, mutex).
Libraries: windows.h (threading, mutex), stdio.h (file I/O, print), stdlib.h (memory).
Components: Hash table (size 1000, chaining), mutex for thread safety, 8 threads.

## Algorithm
Hash: num ^ (num >> 16) % 1000 for hash table index.
Add: Locks mutex, checks for duplicate, adds new node if unique (O(1) average).
Read: Reads integers from file into dynamic array (O(n)).
Process Numbers: Threads process number subsets, call add (O(n/8) per thread).
Free Hash Table: Frees all nodes and array (O(n)).
Main: Reads file, splits work across 8 threads, prints unique numbers, frees resources.

## Summary
Purpose: Finds unique integers from file using multi-threaded hash table.
Complexity: Time O(n), Space O(n) for n numbers.
Features: Thread-safe hash table, 8-thread parallel processing, dynamic array.

## How to run
### compile : gcc -o unique_numbers unique_numbers.c
### Run : ./unique_numbers.exe
