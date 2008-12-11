// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef FEATURE_HPP_
#define FEATURE_HPP_

#include "Common.hpp"

/**
 * The structure for "features", the micro-classes used in minnow's object model
 */
typedef struct feature__{
    struct feature__ *parent; /**< The parent of the feature, NULL if a primary */
    struct feature__ *next; /**< The previous feature in the dl-list, NULL if head */
    struct feature__ *prev; /**< The next feature in the dl-list, NULL if tail */
    unsigned int feature_id; /**< The id, which is unique for each feature type */
} Object_Feature__;

#ifdef __cplusplus
extern "C" {
#endif
/**
 * \addtogroup Functions
 */
/*@{*/
void *create_feature__(size_t feature_size, unsigned int feature_id);
void initialize_feature__(Object_Feature__ *top, unsigned int feature_id);
Object_Feature__ *add_child_feature__(Object_Feature__ *parent, Object_Feature__ *feature);
Object_Feature__ *add_primary_feature__(Object_Feature__ *top, Object_Feature__ *feature);
Object_Feature__ *add_child_feature__(Object_Feature__ *parent, Object_Feature__ *feature);
Object_Feature__ *add_primary_feature__(Object_Feature__ *top, Object_Feature__ *feature);
Object_Feature__ *remove_feature__(Object_Feature__ *to_remove);
Object_Feature__ *find_feature__(Object_Feature__ *start, unsigned int feature_id);
Object_Feature__ *find_child_feature__(Object_Feature__ *parent, unsigned int feature_id);
Object_Feature__ *find_primary_feature__(Object_Feature__ *top, unsigned int feature_id);
Object_Feature__ *find_root__(Object_Feature__ *start);
/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* FEATURE_HPP_ */
