#include "constants.hpp"

#include "js_native_api_types.h"
#include "session/config/contacts.hpp"
#include "session/config/groups/info.hpp"
#include "session/config/user_groups.hpp"
#include "session/pro_backend.h"
#include "session/version.h"
#include "utilities.hpp"
#include "version.h"

namespace session::nodeapi {
ConstantsWrapper::ConstantsWrapper(const Napi::CallbackInfo& info) :
        Napi::ObjectWrap<ConstantsWrapper>(info) {}

Napi::Object ConstantsWrapper::Init(Napi::Env env, Napi::Object exports) {
    const char* class_name = "CONSTANTS";

    auto pro_urls = Napi::Object::New(env);
    pro_urls["roadmap"] = toJs(env, SESSION_PRO_URLS.roadmap.data);
    pro_urls["privacy_policy"] = toJs(env, SESSION_PRO_URLS.privacy_policy.data);
    pro_urls["terms_of_conditions"] = toJs(env, SESSION_PRO_URLS.terms_of_conditions.data);
    pro_urls["pro_access_not_found"] = toJs(env, SESSION_PRO_URLS.pro_access_not_found.data);
    pro_urls["support_url"] = toJs(env, SESSION_PRO_URLS.support_url.data);

    // construct javascript constants object
    Napi::Function cls = DefineClass(
            env,
            class_name,
            {ObjectWrap::StaticValue(
                     "CONTACT_MAX_NAME_LENGTH",
                     Napi::Number::New(env, session::config::contact_info::MAX_NAME_LENGTH),
                     napi_enumerable),
             ObjectWrap::StaticValue(
                     "BASE_GROUP_MAX_NAME_LENGTH",
                     Napi::Number::New(env, session::config::base_group_info::NAME_MAX_LENGTH),
                     napi_enumerable),
             ObjectWrap::StaticValue(
                     "GROUP_INFO_MAX_NAME_LENGTH",
                     Napi::Number::New(env, session::config::groups::Info::NAME_MAX_LENGTH),
                     napi_enumerable),
             ObjectWrap::StaticValue(
                     "GROUP_INFO_DESCRIPTION_MAX_LENGTH",
                     Napi::Number::New(env, session::config::groups::Info::DESCRIPTION_MAX_LENGTH),
                     napi_enumerable),
             ObjectWrap::StaticValue(
                     "COMMUNITY_FULL_URL_MAX_LENGTH",
                     Napi::Number::New(env, session::config::community::FULL_URL_MAX_LENGTH),
                     napi_enumerable),
             ObjectWrap::StaticValue(
                     "LIBSESSION_PRO_URLS",
                     pro_urls,
                     napi_enumerable),
             ObjectWrap::StaticValue(
                     "LIBSESSION_UTIL_VERSION",
                     Napi::String::New(env, LIBSESSION_UTIL_VERSION_FULL),
                     napi_enumerable),
             ObjectWrap::StaticValue(
                     "LIBSESSION_NODEJS_VERSION",
                     Napi::String::New(env, LIBSESSION_NODEJS_VERSION),
                     napi_enumerable),
             ObjectWrap::StaticValue(
                     "LIBSESSION_NODEJS_COMMIT",
                     Napi::String::New(env, LIBSESSION_NODEJS_COMMIT),
                     napi_enumerable)});

    // export object as javascript module
    exports.Set(class_name, cls);
    return exports;
}

}  // namespace session::nodeapi
