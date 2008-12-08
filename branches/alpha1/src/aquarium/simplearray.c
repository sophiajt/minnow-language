#include <stdlib.h>
#include <string.h>

#include "simplearray.h"

void initialize_simplearray(SimpleArray *container, unsigned int size) {
    container->contents = (void*)malloc(DEFAULT_SIMPLEARRAY_SIZE * size);
    container->bufferSize = DEFAULT_SIMPLEARRAY_SIZE;
    container->currentSize = 0;
    container->elemSize = size;
}

void push_onto_simplearray(SimpleArray *container, void *value) {
    if (container->currentSize >= container->bufferSize) {
        container->bufferSize *= 2;
        void* temp = (void*)realloc(container->contents, container->bufferSize * container->elemSize);
        container->contents = temp;
    }

    switch (container->elemSize) {
        case (4) : ((int*)(container->contents))[container->currentSize] = *(int *)value;
            break;
        case (2) : ((short*)(container->contents))[container->currentSize] = *(short *)value;
            break;
        case (1) : ((char*)(container->contents))[container->currentSize] = *(char *)value;
            break;
        default: memcpy (container->contents + container->currentSize * container->elemSize, value, container->elemSize);
            break;
    }
    ++(container->currentSize);
}

void pop_off_simplearray(SimpleArray *container) {
    if (container->currentSize > 0)
        --(container->currentSize);
}

void insert_into_simplearray(SimpleArray *container, void* value, unsigned int pos) {
    if (pos > container->currentSize) {
        push_onto_simplearray(container, value);
    }
    else {
        if (container->currentSize >= container->bufferSize) {
            container->bufferSize *= 2;
            void* temp = (void*)realloc(container->contents, container->bufferSize * container->elemSize);
            container->contents = temp;
        }

        memmove(container->contents + (pos + 1) * container->elemSize, container->contents + pos * container->elemSize,
                container->elemSize * (container->currentSize - pos));

        switch (container->elemSize) {
            case (4) : ((int*)(container->contents))[pos] = *(int *)value;
                break;
            case (2) : ((short*)(container->contents))[pos] = *(short *)value;
                break;
            case (1) : ((char*)(container->contents))[pos] = *(char *)value;
                break;
            default: memcpy (container->contents + pos * container->elemSize, value, container->elemSize);
                break;
        }
        ++(container->currentSize);
    }
}
void delete_from_simplearray(SimpleArray *container, unsigned int pos) {
    if (pos > container->currentSize) {
        pop_off_simplearray(container);
    }
    else if (pos == container->currentSize) {
        if (pos > 0) {
            --(container->currentSize);
        }
    }
    else {
        memmove(container->contents + pos * container->elemSize, container->contents + (pos+1) * container->elemSize,
                container->elemSize * (container->currentSize - pos));
        --(container->currentSize);
    }
}
void delete_from_simplearray_range(SimpleArray *container, unsigned int pos, unsigned int amount) {
    if (pos > container->currentSize) {
        pop_off_simplearray(container);
    }
    else if (pos == container->currentSize) {
        if (pos > 0) {
            --(container->currentSize);
        }
    }
    else {
        memmove(container->contents + pos * container->elemSize, container->contents + (pos+amount) * container->elemSize,
                container->elemSize * (container->currentSize - pos));
        --(container->currentSize);
    }
}
