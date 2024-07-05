#include "dynamic_array.h"

//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

ArrayList* init(unsigned long long unitSize) {
    ArrayList* array = (ArrayList*) malloc(sizeof(ArrayList));
    array->data = malloc(STEPSIZE*unitSize);
    array->unitSize = unitSize;
    array->size = 0;
    array->maxSize = STEPSIZE;
    return array;
}

void push(ArrayList* array, genericPtr item) {
    addAt(array, item, array->size);
}

void shrink(ArrayList* array) {
    //create temp array
    genericPtr tmp = malloc((array->maxSize - STEPSIZE) * array->unitSize);
    //copy items to temp array
    memcpy(tmp, array->data, array->unitSize*array->size);
    //delete old array
    free(array->data);
    array->data = tmp;
    array->maxSize -= STEPSIZE;
}

void expand(ArrayList* array) {
    //create temp array
    genericPtr tmp = malloc((array->maxSize + STEPSIZE) * array->unitSize);
    //copy items to temp array
    memcpy(tmp, array->data, array->unitSize*array->size);
    //delete old array
    free(array->data);
    array->data = tmp;
    array->maxSize += STEPSIZE;
}

genericPtr pop(ArrayList* array) {
    return removeAt(array, array->size);
}

genericPtr removeAt(ArrayList* array, int i) {
//    pthread_mutex_lock(&mutex);
    genericPtr toReturn = array->data + i * array->unitSize;
    //shift left to overwrite removed element
    memmove(array->data + i * array->unitSize, array->data + (i + 1)*array->unitSize, array->unitSize * (array->size - i));

    array->size -= 1;
    if (array->maxSize - array->size > STEPSIZE) shrink(array);
//    pthread_mutex_unlock(&mutex);
    return toReturn;
}

void addAt(ArrayList* array, genericPtr item, int i) {
//    pthread_mutex_lock(&mutex);
    while (array->size == array->maxSize || i > array->size) {
        expand(array);//expand if array is full or index is out of range;
    }
    //shift right to make space
    array->size += 1;
    memmove(array->data + (i + 1)*array->unitSize, array->data + i * array->unitSize, array->unitSize * (array->size - i - 1));
    //add element
    memmove(array->data + i * array->unitSize, item, array->unitSize);

//    pthread_mutex_unlock(&mutex);
}

void setAt(ArrayList* array, genericPtr item, int i) {//TODO: fix this segfaulting
//    pthread_mutex_lock(&mutex);
    while (array->size == array->maxSize || i > array->size) {
        expand(array);//expand if array is full or index is out of range;
    }
    //add element
    memmove(array->data + i * array->unitSize, item, array->unitSize);

//    pthread_mutex_unlock(&mutex);
}

genericPtr at(ArrayList* array, int i) {
    if (i >= array->size) return NULL;
//    pthread_mutex_lock(&mutex);
    genericPtr b = array->data + i*array->unitSize;
//    pthread_mutex_unlock(&mutex);
    return b;
}

void clear(ArrayList* array) {
//    pthread_mutex_lock(&mutex);
    array->size = 0;
//    pthread_mutex_unlock(&mutex);
}

void delete(ArrayList* array) {
//    pthread_mutex_lock(&mutex);
    free(array);
    array = NULL;
//    pthread_mutex_unlock(&mutex);
}

void deleteContents(ArrayList* array) {
//    pthread_mutex_lock(&mutex);
    free(array->data);
    array->data = malloc(STEPSIZE*array->unitSize);
    //    pthread_mutex_unlock(&mutex);
}