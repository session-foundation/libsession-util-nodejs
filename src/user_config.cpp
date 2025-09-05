#include "user_config.hpp"

#include <iostream>

#include "base_config.hpp"
#include "profile_pic.hpp"
#include "session/config/base.hpp"
#include "session/config/user_profile.hpp"

namespace session::nodeapi {

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
                    InstanceMethod(
                            "getProfileUpdatedSeconds",
                            &UserConfigWrapper::getProfileUpdatedSeconds),
                    InstanceMethod(
                            "setReuploadProfilePic", &UserConfigWrapper::setReuploadProfilePic),
                    InstanceMethod("setPriority", &UserConfigWrapper::setPriority),
                    InstanceMethod("setName", &UserConfigWrapper::setName),
                    InstanceMethod("setNameTruncated", &UserConfigWrapper::setNameTruncated),
                    InstanceMethod("setUserConfig", &UserConfigWrapper::setUserConfig),
                    InstanceMethod("setNewProfilePic", &UserConfigWrapper::setNewProfilePic),
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

void UserConfigWrapper::setUserConfig(const Napi::CallbackInfo& info) {
    return wrapExceptions(info, [&] {
        auto env = info.Env();
        assertInfoLength(info, 1);
        auto configObj = info[0];
        assertIsObject(configObj);

        auto obj = configObj.As<Napi::Object>();
        if (obj.IsEmpty()) {
            return;
        }
        // Handle priority field
        if (auto priority = maybeNonemptyInt(
                    obj.Get("priority"), "UserConfigWrapper::setUserConfig - priority")) {
            auto new_priority = toPriority(priority.value(), config.get_nts_priority());
            config.set_nts_priority(new_priority);
        }

        // Handle nameTruncated field
        if (auto new_name = maybeNonemptyString(
                    obj.Get("name"), "UserConfigWrapper::setUserConfig - name")) {
            config.set_name_truncated(*new_name);  // truncates silently if too long
        }

        // Handle newProfilePic field
        if (auto newProfilePic = maybeNonemptyProfilePic(
                    obj.Get("newProfilePic"), "UserConfigWrapper::setUserConfig - newProfilePic")) {
            config.set_profile_pic(*newProfilePic);
        }

        // Handle reuploadProfilePic field
        if (auto reuploadProfilePic = maybeNonemptyProfilePic(
                    obj.Get("reuploadProfilePic"),
                    "UserConfigWrapper::setUserConfig - reuploadProfilePic")) {
            config.set_reupload_profile_pic(*reuploadProfilePic);
        }

        // Handle enableBlindedMsgRequest field
        if (auto blindedMsgReqCpp = maybeNonemptyBoolean(
                    obj.Get("enableBlindedMsgRequest"),
                    "UserConfigWrapper::setUserConfig - enableBlindedMsgRequest")) {
            config.set_blinded_msgreqs(*blindedMsgReqCpp);
        }

        // Handle noteToSelfExpiry field
        if (auto new_nts_expiry_seconds = maybeNonemptyInt(
                    obj.Get("noteToSelfExpirySeconds"),
                    "UserConfigWrapper::setUserConfig - noteToSelfExpiry")) {
            auto expiry = obj.Get("noteToSelfExpiry");

            config.set_nts_expiry(std::chrono::seconds{*new_nts_expiry_seconds});
        }
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

void UserConfigWrapper::setNewProfilePic(const Napi::CallbackInfo& info) {
    return wrapExceptions(info, [&] {
        assertInfoLength(info, 1);
        auto profile_pic_obj = info[0];

        if (!profile_pic_obj.IsNull() && !profile_pic_obj.IsUndefined())
            assertIsObject(profile_pic_obj);

        config.set_profile_pic(profile_pic_from_object(profile_pic_obj));
    });
}

void UserConfigWrapper::setReuploadProfilePic(const Napi::CallbackInfo& info) {
    assertInfoLength(info, 1);
    auto profile_pic_obj = info[0];

    if (!profile_pic_obj.IsNull() && !profile_pic_obj.IsUndefined())
        assertIsObject(profile_pic_obj);

    config.set_reupload_profile_pic(profile_pic_from_object(profile_pic_obj));
}

Napi::Value UserConfigWrapper::getProfileUpdatedSeconds(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        auto env = info.Env();
        return config.get_profile_updated();
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
