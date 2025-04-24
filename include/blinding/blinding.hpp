#pragma once

#include <napi.h>

#include <algorithm>
#include <vector>

#include "../meta/meta_base_wrapper.hpp"
#include "../utilities.hpp"
#include "oxenc/hex.h"
#include "session/blinding.hpp"
#include "session/config/user_profile.hpp"
#include "session/platform.hpp"
#include "session/random.hpp"

namespace session::nodeapi {

class BlindingWrapper : public Napi::ObjectWrap<BlindingWrapper> {

  public:
    BlindingWrapper(const Napi::CallbackInfo& info) : Napi::ObjectWrap<BlindingWrapper>{info} {
        throw std::invalid_argument("BlindingWrapper is static and doesn't need to be constructed");
    }

    static void Init(Napi::Env env, Napi::Object exports) {
        MetaBaseWrapper::NoBaseClassInitHelper<BlindingWrapper>(
                env,
                exports,
                "BlindingWrapperNode",
                {
                        StaticMethod<&BlindingWrapper::blindVersionPubkey>(
                                "blindVersionPubkey",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),
                        StaticMethod<&BlindingWrapper::blindVersionSignRequest>(
                                "blindVersionSignRequest",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),
                        StaticMethod<&BlindingWrapper::blindVersionSign>(
                                "blindVersionSign",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),
                });
    }

  private:
    static Napi::Value blindVersionPubkey(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            assertInfoLength(info, 1);
            assertIsObject(info[0]);
            auto obj = info[0].As<Napi::Object>();

            if (obj.IsEmpty())
                throw std::invalid_argument("blindVersionPubkey received empty");

            assertIsUInt8Array(obj.Get("ed25519SecretKey"), "BlindingWrapper::blindVersionPubkey");
            auto ed25519_secret_key =
                    toCppBuffer(obj.Get("ed25519SecretKey"), "blindVersionPubkey.ed25519SecretKey");

            auto keypair = session::blind_version_key_pair(ed25519_secret_key);
            session::uc32 pk_arr = std::get<0>(keypair);
            std::vector<unsigned char> blinded_pk = session::to_vector(pk_arr);
            std::string blinded_pk_hex;
            blinded_pk_hex.reserve(66);
            blinded_pk_hex += "07";
            oxenc::to_hex(blinded_pk.begin(), blinded_pk.end(), std::back_inserter(blinded_pk_hex));

            return blinded_pk_hex;
        });
    };

    static Napi::Value blindVersionSignRequest(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            assertInfoLength(info, 1);
            assertIsObject(info[0]);
            auto obj = info[0].As<Napi::Object>();

            if (obj.IsEmpty())
                throw std::invalid_argument("blindVersionSignRequest received empty");

            assertIsUInt8Array(
                    obj.Get("ed25519SecretKey"), "blindVersionSignRequest.ed25519SecretKey");
            auto ed25519_secret_key = toCppBuffer(
                    obj.Get("ed25519SecretKey"), "blindVersionSignRequest.ed25519SecretKey");

            assertIsNumber(
                    obj.Get("sigTimestampSeconds"), "blindVersionSignRequest.sigTimestampSeconds");
            auto sig_timestamp = toCppInteger(
                    obj.Get("sigTimestampSeconds"),
                    "blindVersionSignRequest.sigTimestampSeconds",
                    false);

            assertIsString(obj.Get("sigMethod"));
            auto sig_method =
                    toCppString(obj.Get("sigMethod"), "blindVersionSignRequest.sigMethod");

            assertIsString(obj.Get("sigPath"));
            auto sig_path = toCppString(obj.Get("sigPath"), "blindVersionSignRequest.sigPath");

            assertIsUInt8ArrayOrNull(obj.Get("sigBody"));
            auto sig_body =
                    maybeNonemptyBuffer(obj.Get("sigBody"), "blindVersionSignRequest.sigBody");

            return session::blind_version_sign_request(
                    ed25519_secret_key, sig_timestamp, sig_method, sig_path, sig_body);
        });
    };

    static Napi::Value blindVersionSign(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            assertInfoLength(info, 1);
            assertIsObject(info[0]);
            auto obj = info[0].As<Napi::Object>();

            if (obj.IsEmpty())
                throw std::invalid_argument("blindVersionSign received empty");

            assertIsUInt8Array(obj.Get("ed25519SecretKey"), "BlindingWrapper::blindVersionSign");
            auto ed25519_secret_key =
                    toCppBuffer(obj.Get("ed25519SecretKey"), "blindVersionSign.ed25519SecretKey");

            assertIsNumber(obj.Get("sigTimestampSeconds"), "BlindingWrapper::blindVersionSign");
            auto sig_timestamp = toCppInteger(
                    obj.Get("sigTimestampSeconds"), "blindVersionSign.sigTimestampSeconds", false);

            return session::blind_version_sign(
                    ed25519_secret_key, Platform::desktop, sig_timestamp);
        });
    };
};

}  // namespace session::nodeapi
