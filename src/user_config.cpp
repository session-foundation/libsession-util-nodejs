#include "user_config.hpp"

#include <iostream>

#include "base_config.hpp"
#include "profile_pic.hpp"
#include "session/config/base.hpp"
#include "session/config/user_profile.hpp"

namespace session::nodeapi {

// using config::LogLevel;
using config::UserProfile;

void UserConfigWrapper::Init(Napi::Env env, Napi::Object exports) {
    InitHelper<UserConfigWrapper>(
            env,
            exports,
            "UserConfigWrapperNode",
            {
                    InstanceMethod("getPriority", &UserConfigWrapper::getPriority),
                    InstanceMethod("getName", &UserConfigWrapper::getName),
                    InstanceMethod("getProfilePic", &UserConfigWrapper::getProfilePic),
                    InstanceMethod("setPriority", &UserConfigWrapper::setPriority),
                    InstanceMethod("setName", &UserConfigWrapper::setName),
                    InstanceMethod("setNameTruncated", &UserConfigWrapper::setNameTruncated),
                    InstanceMethod("setProfilePic", &UserConfigWrapper::setProfilePic),
                    InstanceMethod(
                            "getEnableBlindedMsgRequest",
                            &UserConfigWrapper::getEnableBlindedMsgRequest),
                    InstanceMethod(
                            "setEnableBlindedMsgRequest",
                            &UserConfigWrapper::setEnableBlindedMsgRequest),
                    InstanceMethod("getNoteToSelfExpiry", &UserConfigWrapper::getNoteToSelfExpiry),
                    InstanceMethod("setNoteToSelfExpiry", &UserConfigWrapper::setNoteToSelfExpiry),
            });
}

UserConfigWrapper::UserConfigWrapper(const Napi::CallbackInfo& info) :
        ConfigBaseImpl{construct<config::UserProfile>(info, "UserConfig")},
        Napi::ObjectWrap<UserConfigWrapper>{info} {}

Napi::Value UserConfigWrapper::getPriority(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        auto env = info.Env();
        return config.get_nts_priority();
    });
}

Napi::Value UserConfigWrapper::getName(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        auto env = info.Env();
        return config.get_name();
    });
}

Napi::Value UserConfigWrapper::getProfilePic(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        auto env = info.Env();
        auto pic = config.get_profile_pic();
        auto obj = Napi::Object::New(env);
        if (pic) {
            obj["url"] = toJs(env, pic.url);
            obj["key"] = toJs(env, pic.key);
        } else {
            obj["url"] = env.Null();
            obj["key"] = env.Null();
        }
        return obj;
    });
}

void UserConfigWrapper::setPriority(const Napi::CallbackInfo& info) {
    return wrapExceptions(info, [&] {
        auto env = info.Env();
        assertInfoLength(info, 1);
        auto priority = info[0];
        assertIsNumber(priority, "UserConfigWrapper::setPriority");

        auto new_priority = toPriority(priority, config.get_nts_priority());
        config.set_nts_priority(new_priority);
    });
}

void UserConfigWrapper::setName(const Napi::CallbackInfo& info) {
    return wrapExceptions(info, [&] {
        auto env = info.Env();
        assertInfoLength(info, 1);
        auto name = info[0];
        assertIsString(name);

        auto new_name = name.As<Napi::String>().Utf8Value();
        // this will throw if the name is too long
        config.set_name(new_name);
    });
}

void UserConfigWrapper::setNameTruncated(const Napi::CallbackInfo& info) {
    return wrapExceptions(info, [&] {
        auto env = info.Env();
        assertInfoLength(info, 1);
        auto name = info[0];
        assertIsString(name);

        auto new_name = name.As<Napi::String>().Utf8Value();
        // this will truncate silently if the name is too long
        config.set_name_truncated(new_name);
    });
}

void UserConfigWrapper::setProfilePic(const Napi::CallbackInfo& info) {
    return wrapExceptions(info, [&] {
        assertInfoLength(info, 1);
        auto profile_pic_obj = info[0];

        if (!profile_pic_obj.IsNull() && !profile_pic_obj.IsUndefined())
            assertIsObject(profile_pic_obj);

        config.set_profile_pic(profile_pic_from_object(profile_pic_obj));
    });
}

Napi::Value UserConfigWrapper::getEnableBlindedMsgRequest(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        auto env = info.Env();
        auto blindedMsgRequest = toJs(env, config.get_blinded_msgreqs());

        return blindedMsgRequest;
    });
}

void UserConfigWrapper::setEnableBlindedMsgRequest(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 1);

        auto blindedMsgRequests = info[0];
        assertIsBoolean(blindedMsgRequests);

        auto blindedMsgReqCpp = toCppBoolean(blindedMsgRequests, "set_blinded_msgreqs");
        config.set_blinded_msgreqs(blindedMsgReqCpp);
    });
}

Napi::Value UserConfigWrapper::getNoteToSelfExpiry(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        auto nts_expiry = config.get_nts_expiry();
        if (nts_expiry) {
            return nts_expiry->count();
        }

        return static_cast<int64_t>(0);
    });
}

void UserConfigWrapper::setNoteToSelfExpiry(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 1);

        auto expirySeconds = info[0];
        assertIsNumber(expirySeconds, "setNoteToSelfExpiry");

        auto expiryCppSeconds = toCppInteger(expirySeconds, "set_nts_expiry", false);
        config.set_nts_expiry(std::chrono::seconds{expiryCppSeconds});
    });
}

}  // namespace session::nodeapi
