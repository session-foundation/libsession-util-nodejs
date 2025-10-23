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
    pro_urls["roadmap"] = toJs(env, SESSION_PRO_URLS.roadmap);
    pro_urls["privacy_policy"] = toJs(env, SESSION_PRO_URLS.privacy_policy);
    pro_urls["terms_of_conditions"] = toJs(env, SESSION_PRO_URLS.terms_of_conditions);
    pro_urls["pro_access_not_found"] = toJs(env, SESSION_PRO_URLS.pro_access_not_found);
    pro_urls["support_url"] = toJs(env, SESSION_PRO_URLS.support_url);

    auto pro_provider_nil = Napi::Object::New(env);
    pro_provider_nil["device"] = toJs(env, SESSION_PRO_BACKEND_PAYMENT_PROVIDER_METADATA[SESSION_PRO_BACKEND_PAYMENT_PROVIDER_NIL].device);
    pro_provider_nil["store"] = toJs(env, SESSION_PRO_BACKEND_PAYMENT_PROVIDER_METADATA[SESSION_PRO_BACKEND_PAYMENT_PROVIDER_NIL].store);
    pro_provider_nil["platform"] = toJs(env, SESSION_PRO_BACKEND_PAYMENT_PROVIDER_METADATA[SESSION_PRO_BACKEND_PAYMENT_PROVIDER_NIL].platform);
    pro_provider_nil["platform_account"] = toJs(env, SESSION_PRO_BACKEND_PAYMENT_PROVIDER_METADATA[SESSION_PRO_BACKEND_PAYMENT_PROVIDER_NIL].platform_account);
    pro_provider_nil["refund_url"] = toJs(env, SESSION_PRO_BACKEND_PAYMENT_PROVIDER_METADATA[SESSION_PRO_BACKEND_PAYMENT_PROVIDER_NIL].refund_url);
    pro_provider_nil["update_subscription_url"] = toJs(env, SESSION_PRO_BACKEND_PAYMENT_PROVIDER_METADATA[SESSION_PRO_BACKEND_PAYMENT_PROVIDER_NIL].update_subscription_url);
    pro_provider_nil["cancel_subscription_url"] = toJs(env, SESSION_PRO_BACKEND_PAYMENT_PROVIDER_METADATA[SESSION_PRO_BACKEND_PAYMENT_PROVIDER_NIL].cancel_subscription_url);

    auto pro_provider_google = Napi::Object::New(env);
    pro_provider_google["device"] = toJs(env, SESSION_PRO_BACKEND_PAYMENT_PROVIDER_METADATA[SESSION_PRO_BACKEND_PAYMENT_PROVIDER_GOOGLE_PLAY_STORE].device);
    pro_provider_google["store"] = toJs(env, SESSION_PRO_BACKEND_PAYMENT_PROVIDER_METADATA[SESSION_PRO_BACKEND_PAYMENT_PROVIDER_GOOGLE_PLAY_STORE].store);
    pro_provider_google["platform"] = toJs(env, SESSION_PRO_BACKEND_PAYMENT_PROVIDER_METADATA[SESSION_PRO_BACKEND_PAYMENT_PROVIDER_GOOGLE_PLAY_STORE].platform);
    pro_provider_google["platform_account"] = toJs(env, SESSION_PRO_BACKEND_PAYMENT_PROVIDER_METADATA[SESSION_PRO_BACKEND_PAYMENT_PROVIDER_GOOGLE_PLAY_STORE].platform_account);
    pro_provider_google["refund_url"] = toJs(env, SESSION_PRO_BACKEND_PAYMENT_PROVIDER_METADATA[SESSION_PRO_BACKEND_PAYMENT_PROVIDER_GOOGLE_PLAY_STORE].refund_url);
    pro_provider_google["update_subscription_url"] = toJs(env, SESSION_PRO_BACKEND_PAYMENT_PROVIDER_METADATA[SESSION_PRO_BACKEND_PAYMENT_PROVIDER_GOOGLE_PLAY_STORE].update_subscription_url);
    pro_provider_google["cancel_subscription_url"] = toJs(env, SESSION_PRO_BACKEND_PAYMENT_PROVIDER_METADATA[SESSION_PRO_BACKEND_PAYMENT_PROVIDER_GOOGLE_PLAY_STORE].cancel_subscription_url);

    auto pro_provider_ios = Napi::Object::New(env);
    pro_provider_ios["device"] = toJs(env, SESSION_PRO_BACKEND_PAYMENT_PROVIDER_METADATA[SESSION_PRO_BACKEND_PAYMENT_PROVIDER_IOS_APP_STORE].device);
    pro_provider_ios["store"] = toJs(env, SESSION_PRO_BACKEND_PAYMENT_PROVIDER_METADATA[SESSION_PRO_BACKEND_PAYMENT_PROVIDER_IOS_APP_STORE].store);
    pro_provider_ios["platform"] = toJs(env, SESSION_PRO_BACKEND_PAYMENT_PROVIDER_METADATA[SESSION_PRO_BACKEND_PAYMENT_PROVIDER_IOS_APP_STORE].platform);
    pro_provider_ios["platform_account"] = toJs(env, SESSION_PRO_BACKEND_PAYMENT_PROVIDER_METADATA[SESSION_PRO_BACKEND_PAYMENT_PROVIDER_IOS_APP_STORE].platform_account);
    pro_provider_ios["refund_url"] = toJs(env, SESSION_PRO_BACKEND_PAYMENT_PROVIDER_METADATA[SESSION_PRO_BACKEND_PAYMENT_PROVIDER_IOS_APP_STORE].refund_url);
    pro_provider_ios["update_subscription_url"] = toJs(env, SESSION_PRO_BACKEND_PAYMENT_PROVIDER_METADATA[SESSION_PRO_BACKEND_PAYMENT_PROVIDER_IOS_APP_STORE].update_subscription_url);
    pro_provider_ios["cancel_subscription_url"] = toJs(env, SESSION_PRO_BACKEND_PAYMENT_PROVIDER_METADATA[SESSION_PRO_BACKEND_PAYMENT_PROVIDER_IOS_APP_STORE].cancel_subscription_url);

    auto pro_providers = Napi::Object::New(env);
    pro_providers["0"] = toJs(env, pro_provider_nil);
    pro_providers["1"] = toJs(env, pro_provider_google);
    pro_providers["2"] = toJs(env, pro_provider_ios);

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
                     "LIBSESSION_PRO_PROVIDERS",
                     pro_providers,
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
