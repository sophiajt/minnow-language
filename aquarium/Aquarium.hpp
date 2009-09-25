// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef AQUARIUM_HPP_
#define AQUARIUM_HPP_

#include "Common.hpp"
#include "Typeless_Vector.hpp"
#include "Typeless_Dictionary.hpp"
#include "Char_String.hpp"
#include "Actor.hpp"
#include "Object_Feature.hpp"
#include "Message.hpp"
#include "Message_Channel.hpp"
#include "Scheduler.hpp"

#include "Minnow_Prelude.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * \addtogroup Functions
 */
/*@{*/
int aquarium_main__(int argc, char *argv[], BOOL(*task)(Message__ *), BOOL pass_cmd_line);
/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* AQUARIUM_HPP_ */
