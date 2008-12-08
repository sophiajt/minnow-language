// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../Aquarium.hpp"

typedef struct {
    Object_Feature__ base;
    int x;
    int y;
} Position;

typedef struct {
    Object_Feature__ base;
    int z;
} Zed;

typedef struct {
    Object_Feature__ base;
    int a;
} Alpha;

#define FID_POSITION 100
#define FID_ZED 101
#define FID_ALPHA 102

void debug_print_feature__(Object_Feature__ *of) {
    printf("Obj: %p\n  Id: %i\n  Parent: %p\n  Prev: %p\n  Next: %p\n", of, of->feature_id, of->parent, of->prev, of->next);
}

void debug_print_feature_tree__(Object_Feature__ *of) {
    Object_Feature__ *tmp = of;

    while (tmp != NULL) {
        debug_print_feature__(tmp);
        tmp = tmp->next;
    }
}

int main() {
    Object_Feature__ *head;

    Position *p = (Position *)malloc(sizeof(Position));
    initialize_feature__(&p->base, FID_POSITION);
    p->x = 100;
    p->y = 200;

    head = &p->base;

    Zed *z = (Zed *)malloc(sizeof(Zed));
    initialize_feature__(&z->base, FID_ZED);
    z->z = 300;
    add_child_feature__(head, &z->base);

    Zed *z2 = (Zed *)create_feature__(sizeof(Zed), FID_ZED);
    z2->z = 400;
    add_primary_feature__(head, &z2->base);

    Position *p2 = (Position *)create_feature__(sizeof(Position), FID_POSITION);
    p2->x = 500;
    p2->y = 600;
    add_child_feature__(head, &p2->base);

    Alpha *a = (Alpha *)create_feature__(sizeof(Alpha), FID_ALPHA);
    a->a = 700;

    Alpha *a2 = (Alpha *)create_feature__(sizeof(Alpha), FID_ALPHA);
    a2->a = 800;

    Alpha *a3 = (Alpha *)create_feature__(sizeof(Alpha), FID_ALPHA);
    a3->a = 900;

    add_child_feature__(&a->base, &a2->base);
    add_child_feature__(&a->base, &a3->base);

    add_primary_feature__(head, &a->base);
    //debug_print_feature_tree__(head);

    assert(z->base.parent == &p->base);

    Object_Feature__ *result = find_child_feature__(head, FID_POSITION);
    assert(result == &p2->base);

    Object_Feature__ *result2 = find_feature__(head, FID_POSITION);
    assert(result2 == head);

    Object_Feature__ *result3 = find_primary_feature__(head, FID_ZED);
    assert(result3 == &z2->base);

    Object_Feature__ *result4 = find_root__(&p2->base);
    assert(result4 == head);

    debug_print_feature_tree__(head);

    head = remove_feature__(&p2->base);

    printf("Removed second Position\n");
    debug_print_feature_tree__(head);

    head = remove_feature__(&p->base);

    printf("Removed head and its children\n");
    debug_print_feature_tree__(head);

    return 0;
}
