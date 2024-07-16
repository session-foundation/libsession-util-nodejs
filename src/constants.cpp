#include "constants.hpp"

#include "session/config/contacts.hpp"
#include "session/config/groups/info.hpp"
#include "session/config/user_groups.hpp"

namespace session::nodeapi {
ConstantsWrapper::ConstantsWrapper(const Napi::CallbackInfo& info) :
        Napi::ObjectWrap<ConstantsWrapper>(info) {}

Napi::Object ConstantsWrapper::Init(Napi::Env env, Napi::Object exports) {
    // fetch cpp constants
    Napi::Number CONTACT_MAX_NAME_LENGTH =
            Napi::Number::New(env, session::config::contact_info::MAX_NAME_LENGTH);
    Napi::Number BASE_GROUP_MAX_NAME_LENGTH =
            Napi::Number::New(env, session::config::base_group_info::NAME_MAX_LENGTH);
    Napi::Number GROUP_INFO_MAX_NAME_LENGTH =
            Napi::Number::New(env, session::config::groups::Info::NAME_MAX_LENGTH);

    const char* class_name = "CONSTANTS";

    // construct javascript constants object
    Napi::Function cls = DefineClass(
            env,
            class_name,
            {ObjectWrap::StaticValue(
                     "CONTACT_MAX_NAME_LENGTH", CONTACT_MAX_NAME_LENGTH, napi_enumerable),
             ObjectWrap::StaticValue(
                     "BASE_GROUP_MAX_NAME_LENGTH", BASE_GROUP_MAX_NAME_LENGTH, napi_enumerable),
             ObjectWrap::StaticValue(
                     "GROUP_INFO_MAX_NAME_LENGTH", GROUP_INFO_MAX_NAME_LENGTH, napi_enumerable)

            });

    // export object as javascript module
    exports.Set(class_name, cls);
    return exports;
}

}  // namespace session::nodeapi
