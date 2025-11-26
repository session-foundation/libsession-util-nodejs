#include "user_config.hpp"

#include <napi.h>

#include "base_config.hpp"
#include "oxenc/hex.h"
#include "pro/types.hpp"
#include "profile_pic.hpp"
#include "session/config/user_profile.hpp"
#include "session/ed25519.hpp"
#include "utilities.hpp"

namespace session::nodeapi {

using config::UserProfile;

session::config::ProConfig pro_config_from_object(Napi::Object input) {
    session::config::ProConfig pro_config = {};

    auto rotating_privkey_hex_js = input.Get("rotatingPrivKeyHex");
    assertIsString(rotating_privkey_hex_js, "pro_config_from_object.rotating_privkey_js");
    auto rotating_privkey_cpp = from_hex_to_vector(
            toCppString(rotating_privkey_hex_js, "pro_config_from_object.rotating_privkey_js"));
    assert_length(rotating_privkey_cpp, 64, "pro_config_from_object.rotating_privkey_js");
    std::copy(
            rotating_privkey_cpp.begin(),
            rotating_privkey_cpp.end(),
            pro_config.rotating_privkey.begin());

    auto proProof = input.Get("proProof");
    assertIsObject(proProof);
    auto proof_js = proProof.As<Napi::Object>();

    // extract version
    assertIsNumber(proof_js.Get("version"), "pro_config_from_object.version");
    pro_config.proof.version =
            toCppInteger(proof_js.Get("version"), "pro_config_from_object.version");
    // extract genIndexHashB64
    auto gen_index_hash_b64 = proof_js.Get("genIndexHashB64");
    assertIsString(gen_index_hash_b64, "pro_config_from_object.genIndexHashB64");
    auto gen_index_hash_b64_cpp =
            toCppString(gen_index_hash_b64, "pro_config_from_object.genIndexHashB64");
    auto gen_index_hash_cpp = from_base64_to_vector(gen_index_hash_b64_cpp);
    std::copy(
            gen_index_hash_cpp.begin(),
            gen_index_hash_cpp.end(),
            pro_config.proof.gen_index_hash.begin());

    // extract rotatingPubkeyHex
    auto rotating_pubkey_hex_js = proof_js.Get("rotatingPubkeyHex");
    assertIsString(rotating_pubkey_hex_js, "pro_config_from_object.rotatingPubkeyHex");
    auto rotating_pubkey_hex_cpp =
            toCppString(rotating_pubkey_hex_js, "pro_config_from_object.rotatingPubkeyHex");
    auto rotating_pubkey_cpp = from_hex_to_vector(rotating_pubkey_hex_cpp);
    assert_length(rotating_pubkey_cpp, 32, "pro_config_from_object.rotatingPubkeyHex");
    std::copy(
            rotating_pubkey_cpp.begin(),
            rotating_pubkey_cpp.end(),
            pro_config.proof.rotating_pubkey.begin());

    // extract backend signature
    auto signature_hex_js = proof_js.Get("signatureHex");
    assertIsString(signature_hex_js, "pro_config_from_object.signature_hex_js");
    auto signature_hex_cpp =
            toCppString(signature_hex_js, "pro_config_from_object.signature_hex_js");
    auto signature_cpp = from_hex_to_vector(signature_hex_cpp);
    std::copy(signature_cpp.begin(), signature_cpp.end(), pro_config.proof.sig.begin());
    assert_length(signature_cpp, 64, "pro_config_from_object.signature_cpp");

    // extract expiryMs
    assertIsNumber(proof_js.Get("expiryMs"), "pro_config_from_object.expiryMs");
    pro_config.proof.expiry_unix_ts =
            toCppSysMs(proof_js.Get("expiryMs"), "pro_config_from_object.expiryMs");

    return pro_config;
};

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
                    InstanceMethod("setNewProfilePic", &UserConfigWrapper::setNewProfilePic),
                    InstanceMethod(
                            "getEnableBlindedMsgRequest",
                            &UserConfigWrapper::getEnableBlindedMsgRequest),
                    InstanceMethod(
                            "setEnableBlindedMsgRequest",
                            &UserConfigWrapper::setEnableBlindedMsgRequest),
                    InstanceMethod("getNoteToSelfExpiry", &UserConfigWrapper::getNoteToSelfExpiry),
                    InstanceMethod("setNoteToSelfExpiry", &UserConfigWrapper::setNoteToSelfExpiry),
                    InstanceMethod("getProConfig", &UserConfigWrapper::getProConfig),
                    InstanceMethod("setProConfig", &UserConfigWrapper::setProConfig),
                    InstanceMethod("removeProConfig", &UserConfigWrapper::removeProConfig),
                    InstanceMethod("setProBadge", &UserConfigWrapper::setProBadge),
                    InstanceMethod("setAnimatedAvatar", &UserConfigWrapper::setAnimatedAvatar),
                    InstanceMethod("getProProfileBitset", &UserConfigWrapper::getProProfileBitset),
                    InstanceMethod(
                            "generateProMasterKey", &UserConfigWrapper::generateProMasterKey),
                    InstanceMethod(
                            "generateRotatingPrivKeyHex",
                            &UserConfigWrapper::generateRotatingPrivKeyHex),
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

Napi::Value UserConfigWrapper::getProConfig(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        if (config.get_pro_config().has_value()) {
            return toJs(info.Env(), config.get_pro_config().value());
        }

        return info.Env().Null();
    });
}

void UserConfigWrapper::setProConfig(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 1);
        auto pro_config_js = info[0];
        assertIsObject(pro_config_js);

        session::config::ProConfig pro_config =
                pro_config_from_object(pro_config_js.As<Napi::Object>());

        config.set_pro_config(pro_config);
    });
}

Napi::Value UserConfigWrapper::removeProConfig(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 0);

        return config.remove_pro_config();
    });
}

Napi::Value UserConfigWrapper::getProProfileBitset(const Napi::CallbackInfo& info) {
    return wrapResult(
            info, [&] { return proProfileBitsetToJS(info.Env(), config.get_profile_bitset()); });
}

void UserConfigWrapper::setProBadge(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 1);
        assertIsBoolean(info[0], "setProBadge");

        auto enabled = toCppBoolean(info[0], "UserConfigWrapper::setProBadge");

        config.set_pro_badge(enabled);
    });
}

void UserConfigWrapper::setAnimatedAvatar(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 1);
        assertIsBoolean(info[0], "setAnimatedAvatar");

        auto enabled = toCppBoolean(info[0], "UserConfigWrapper::setAnimatedAvatar");

        config.set_animated_avatar(enabled);
    });
}

Napi::Value UserConfigWrapper::generateProMasterKey(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);

        auto input = info[0];
        assertIsObject(input);
        auto ed25519_seed_js_hex = input.As<Napi::Object>().Get("ed25519SeedHex");
        assertIsString(ed25519_seed_js_hex, "generateProMasterKey");
        auto ed25519_seed_cpp_hex = toCppString(ed25519_seed_js_hex, "generateProMasterKey");
        assert_length(ed25519_seed_cpp_hex, 64, "generateProMasterKey");
        auto converted = from_hex_to_vector(ed25519_seed_cpp_hex);

        auto pro_master_key_hex = session::ed25519::ed25519_pro_privkey_for_ed25519_seed(converted);
        auto obj = Napi::Object::New(info.Env());
        obj["proMasterKeyHex"] = toJs(info.Env(), to_hex(pro_master_key_hex));

        return obj;
    });
}

Napi::Value UserConfigWrapper::generateRotatingPrivKeyHex(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 0);
        auto result = session::ed25519::ed25519_key_pair();
        auto [ed_pk, ed_sk] = result;

        std::string rotating_privkey_hex = to_hex(ed_sk);
        auto obj = Napi::Object::New(info.Env());
        obj["rotatingPrivKeyHex"] = toJs(info.Env(), rotating_privkey_hex);

        return obj;
    });
}

}  // namespace session::nodeapi
