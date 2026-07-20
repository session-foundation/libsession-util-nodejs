#include "constants.hpp"

#include <oxenc/hex.h>

#include "js_native_api_types.h"
#include "session/config/contacts.hpp"
#include "session/config/groups/info.hpp"
#include "session/config/user_groups.hpp"
#include "session/pro_backend.hpp"
#include "session/session_protocol.h"
#include "session/version.h"
#include "utilities.hpp"
#include "version.h"

namespace session::nodeapi {
ConstantsWrapper::ConstantsWrapper(const Napi::CallbackInfo& info) :
        Napi::ObjectWrap<ConstantsWrapper>(info) {}

Napi::Object ConstantsWrapper::Init(Napi::Env env, Napi::Object exports) {
    const char* class_name = "CONSTANTS";

    auto pro_urls = Napi::Object::New(env);
    pro_urls["roadmap"] = toJs(env, SESSION_PROTOCOL_STRINGS.url_pro_roadmap);
    pro_urls["privacy_policy"] = toJs(env, SESSION_PROTOCOL_STRINGS.url_privacy_policy);
    pro_urls["terms_of_service"] = toJs(env, SESSION_PROTOCOL_STRINGS.url_terms_of_service);
    pro_urls["pro_access_not_found"] = toJs(env, SESSION_PROTOCOL_STRINGS.url_pro_access_not_found);
    pro_urls["support_url"] = toJs(env, SESSION_PROTOCOL_STRINGS.url_pro_support);

    // Provider display metadata (store/platform/account NAMES) is no longer shipped by libsession —
    // those are translation data owned by each client (keyed on the provider slug). The per-provider
    // support/management URLs are still libsession-owned but are now fetched on demand via
    // ProWrapper.providerUrls(code) rather than baked into a constants table here.

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
             ObjectWrap::StaticValue("LIBSESSION_PRO_URLS", pro_urls, napi_enumerable),
             // Session Pro backend identity — the single source of truth clients read instead of
             // hand-carrying their own copies (URL is the overridable prod/default; pubkey is hex).
             ObjectWrap::StaticValue(
                     "LIBSESSION_PRO_BACKEND_URL",
                     Napi::String::New(env, std::string(session::pro_backend::URL)),
                     napi_enumerable),
             ObjectWrap::StaticValue(
                     "LIBSESSION_PRO_BACKEND_PUBKEY_HEX",
                     Napi::String::New(env, oxenc::to_hex(session::pro_backend::PUBKEY)),
                     napi_enumerable),
             // X25519 form of the backend pubkey (for onion routing), so clients needn't derive it
             ObjectWrap::StaticValue(
                     "LIBSESSION_PRO_BACKEND_PUBKEY_X25519_HEX",
                     Napi::String::New(env, oxenc::to_hex(session::pro_backend::PUBKEY_X25519)),
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
