#ifndef DYNAMIC_ARRAY_H_
#define DYNAMIC_ARRAY_H_

#include "physics/ball.h"
#include "pthread.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

#define STEPSIZE 10

typedef char* genericPtr;
typedef struct {
    genericPtr data;
    unsigned long long unitSize;
    int size;
    int maxSize;
} ArrayList;

ArrayList* init(unsigned long long unitSize);

void push(ArrayList*, genericPtr);

void shrink(ArrayList*);

void expand(ArrayList*);

genericPtr pop(ArrayList*);

genericPtr removeAt(ArrayList*, int);

void addAt(ArrayList*, genericPtr, int);

void setAt(ArrayList*, genericPtr, int);

genericPtr at(ArrayList*, int);

void clear(ArrayList*);

void delete(ArrayList*);

void deleteContents(ArrayList* array);

#endif