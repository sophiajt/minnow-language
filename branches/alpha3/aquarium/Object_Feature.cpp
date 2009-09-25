// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <stdlib.h>

#include "Common.hpp"

#include "Object_Feature.hpp"

void *create_feature__(size_t feature_size, unsigned int feature_id) {
    void *v = malloc(feature_size);
    initialize_feature__((Object_Feature__*)v, feature_id);

    return v;
}

void initialize_feature__(Object_Feature__ *top, unsigned int feature_id) {
    top->parent = NULL;
    top->next = NULL;
    top->prev = NULL;
    top->feature_id = feature_id;
}

Object_Feature__ *add_child_feature__(Object_Feature__ *parent, Object_Feature__ *feature) {
    Object_Feature__ *tmp = parent, *tmp_child, *tmp_runner;
    while (tmp->next != NULL) {
        if (tmp->parent == parent) {
            //we found the children, go ahead and insert here
            feature->parent = parent;
            feature->prev = tmp;
            tmp_child = feature->next;
            feature->next = tmp->next;
            tmp->next = feature;

            //Then, if there are children
            if (tmp_child != NULL) {
                //Skip over the rest of the brothers/sisters
                while ((tmp->next != NULL) && (tmp->parent == parent)) {
                    tmp = tmp->next;
                }
                if (tmp->parent != parent) {
                    tmp = tmp->prev;
                }

                //Fit in the children after the primaries, in their own group
                tmp_child->prev = tmp;
                tmp_runner = tmp_child;
                while (tmp_runner->next != NULL) {
                    tmp_runner = tmp_runner->next;
                }
                tmp_runner->next = tmp->next;
                if (tmp->next != NULL) {
                    tmp->next->prev = tmp_runner;
                }

                tmp->next = tmp_child;
            }

            return parent;
        }
        tmp = tmp->next;
    }
    feature->parent = parent;
    feature->prev = tmp;
    tmp_runner = feature;
    while(tmp_runner->next != NULL) {
        tmp_runner = tmp_runner->next;
    }
    tmp_runner->next = tmp->next;
    if (tmp->next != NULL) {
        tmp->next->prev = tmp_runner;
    }
    tmp->next = feature;

    return parent;
}

Object_Feature__ *add_primary_feature__(Object_Feature__ *top, Object_Feature__ *feature) {
    Object_Feature__ *tmp = top, *tmp_child, *tmp_runner;

    if (top == NULL) return feature;

    //first, add the feature
    feature->parent = NULL;
    feature->prev = tmp;
    tmp_child = feature->next;
    feature->next = tmp->next;
    if (feature->next != NULL) {
        feature->next->prev = feature;
    }
    tmp->next = feature;

    //Then, if there are children
    if (tmp_child != NULL) {
        //Skip over the rest of the primaries
        while ((tmp->next != NULL) && (tmp->parent == NULL)) {
            tmp = tmp->next;
        }
        if (tmp->parent != NULL) {
            tmp = tmp->prev;
        }

        //Fit in the children after the primaries, in their own group
        tmp_child->prev = tmp;
        tmp_runner = tmp_child;
        while (tmp_runner->next != NULL) {
            tmp_runner = tmp_runner->next;
        }
        tmp_runner->next = tmp->next;
        if (tmp->next != NULL) {
            tmp->next->prev = tmp_runner;
        }

        tmp->next = tmp_child;
    }
    return top;
}

Object_Feature__ *remove_feature__(Object_Feature__ *to_remove) {
    Object_Feature__ *tmp = NULL;

    //first, snip the node out of the dl-list
    if (to_remove->prev != NULL) {
        to_remove->prev->next = to_remove->next;
    }
    if (to_remove->next != NULL) {
        tmp = to_remove->next;
        to_remove->next->prev = to_remove->prev;
    }

    //then remove any child that has this feature as its parent, recursively
    while ((tmp != NULL) && (tmp->parent != to_remove)) {
        tmp = tmp->next;
    }
    while ((tmp != NULL) && (tmp->parent == to_remove)) {
        remove_feature__(tmp);
        tmp = tmp->next;
    }

    //and finally, return the new head feature, which acts as the ptr to the object
    if ((to_remove->parent == NULL) && (to_remove->prev == NULL)) {
        return to_remove->next;
    }
    else {
        tmp = to_remove;
        while (tmp->parent != NULL) {
            tmp = tmp->parent;
        }
        while (tmp->prev != NULL) {
            tmp = tmp->prev;
        }
        return tmp;
    }
}

Object_Feature__ *find_primary_feature__(Object_Feature__ *top, unsigned int feature_id) {
    Object_Feature__ *tmp = top;
    while ((tmp != NULL) && (tmp->parent == NULL)) {
        if (tmp->feature_id == feature_id) {
            return tmp;
        }
        tmp = tmp->next;
    }
    return NULL;
}

Object_Feature__ *find_feature__(Object_Feature__ *start, unsigned int feature_id) {
    if (start->feature_id == feature_id) {
        return start;
    }
    else {
        return find_child_feature__(start, feature_id);
    }
}

Object_Feature__ *find_child_feature__(Object_Feature__ *parent, unsigned int feature_id) {
    Object_Feature__ *tmp = parent;

    while ((tmp->next != NULL) && (tmp->parent != parent)) {
        tmp = tmp->next;
    }

    while (tmp->parent == parent) {
        if (tmp->feature_id == feature_id) {
            return tmp;
        }
        else if (tmp->next == NULL) {
            return NULL;
        }
        tmp = tmp->next;
    }

    return NULL;
}

Object_Feature__ *find_root__(Object_Feature__ *start) {
    Object_Feature__ *tmp = start;

    while (tmp->parent != NULL) {
        tmp = tmp->parent;
    }
    while (tmp->prev != NULL) {
        tmp = tmp->prev;
    }

    return tmp;
}
