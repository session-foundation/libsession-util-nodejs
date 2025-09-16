#include <napi.h>

#include "session/config/profile_pic.hpp"
#include "utilities.hpp"

namespace session::nodeapi {


// Constructs a profile_pic from a Napi::Value which must be either Null or an Object; if an
// Object then it *must* contain "url" (string or null) and "key" (uint8array of size 32 or
// null) keys; if either is empty or Null then you get back an empty (i.e. clearing)
// profile_pic.  Throws on invalid input.
config::profile_pic profile_pic_from_object(Napi::Value val);

}  // namespace session::nodeapi
