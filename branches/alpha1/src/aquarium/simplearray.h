#ifndef SIMPLEARRAY_H_
#define SIMPLEARRAY_H_

typedef struct {
    void* contents;
    unsigned int elemSize;
    unsigned int currentSize;
    unsigned int bufferSize;
} SimpleArray;

#define STORAGE_VAR(x) ((void*)&(x))
#define INDEX_AT(c,x,y) (*((y*)(c.contents + (x) * c.elemSize)))
#define DEFAULT_SIMPLEARRAY_SIZE 4

void initialize_simplearray(SimpleArray *container, unsigned int size);
void push_onto_simplearray(SimpleArray *container, void *value);
void pop_off_simplearray(SimpleArray *container);
void insert_into_simplearray(SimpleArray *container, void* value, unsigned int pos);
void delete_from_simplearray(SimpleArray *container, unsigned int pos);
void delete_from_simplearray_range(SimpleArray *container, unsigned int pos, unsigned int amount);


#endif /* SIMPLEARRAY_H_ */
