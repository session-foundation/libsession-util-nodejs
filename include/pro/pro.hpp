#pragma once

#include <oxenc/base64.h>
#include <oxenc/hex.h>

#include <vector>

#include "../../node_modules/node-addon-api/napi.h"
#include "../meta/meta_base_wrapper.hpp"
#include "../utilities.hpp"
#include "meta/meta_base_wrapper.hpp"
#include "session/pro_backend.hpp"
#include "session/session_protocol.hpp"

namespace session::nodeapi {

class ProWrapper : public Napi::ObjectWrap<ProWrapper> {

  public:
    ProWrapper(const Napi::CallbackInfo& info) : Napi::ObjectWrap<ProWrapper>{info} {
        throw std::invalid_argument("ProWrapper is static and doesn't need to be constructed");
    }

    static void Init(Napi::Env env, Napi::Object exports) {
        MetaBaseWrapper::NoBaseClassInitHelper<ProWrapper>(
                env,
                exports,
                "ProWrapperNode",
                {
                        // Pro features
                        StaticMethod<&ProWrapper::proFeaturesForMessage>(
                                "proFeaturesForMessage",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),
                        StaticMethod<&ProWrapper::proProofRequestBody>(
                                "proProofRequestBody",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),
                });
    }

  private:
    static Napi::Value proFeaturesForMessage(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            // we expect two arguments that match:
            // first: {
            //   "utf16": string,
            //   "proFeatures": Array<ProFeature>,
            // }

            assertInfoLength(info, 1);
            assertIsObject(info[0]);
            auto env = info.Env();

            auto first = info[0].As<Napi::Object>();

            if (first.IsEmpty())
                throw std::invalid_argument("proFeaturesForMessage first received empty");

            assertIsArray(first.Get("proFeatures"), "proFeaturesForMessage.proFeatures");
            auto proFeaturesJS = first.Get("proFeatures").As<Napi::Array>();
            std::vector<std::string> proFeatures;
            proFeatures.reserve(proFeaturesJS.Length());
            for (uint32_t i = 0; i < proFeaturesJS.Length(); i++) {
                auto itemValue = proFeaturesJS.Get(i);
                assertIsString(itemValue, "proFeaturesForMessage.proFeatures.itemValue");
                std::string item =
                        toCppString(itemValue, "proFeaturesForMessage.proFeatures.itemValue");
                proFeatures.push_back(item);
            }

            SESSION_PROTOCOL_PRO_EXTRA_FEATURES flags = 0;
            for (std::string& feature : proFeatures) {
                // Note: 10K_CHARACTER_LIMIT cannot be requested by the caller
                if (feature == "PRO_BADGE") {
                    flags |= SESSION_PROTOCOL_PRO_EXTRA_FEATURES_PRO_BADGE;
                } else if (feature == "ANIMATED_AVATAR") {
                    flags |= SESSION_PROTOCOL_PRO_EXTRA_FEATURES_ANIMATED_AVATAR;
                }
            }
            assertIsString(first.Get("utf16"), "proFeaturesForMessage.utf16");
            std::u16string utf16 = first.Get("utf16").As<Napi::String>().Utf16Value();
            auto pro_features_msg =
                    session::pro_features_for_utf16((utf16.data()), utf16.length(), flags);

            auto obj = Napi::Object::New(env);

            obj["success"] = toJs(env, pro_features_msg.success);
            obj["error"] =
                    pro_features_msg.error.size() ? toJs(env, pro_features_msg.error) : env.Null();
            obj["codepointCount"] = toJs(env, pro_features_msg.codepoint_count);
            obj["proFeatures"] = proFeaturesToJs(env, pro_features_msg.features);

            return obj;
        });
    };

    static Napi::Value proProofRequestBody(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            // we expect arguments that match:
            // first: {
            //   "version": string,
            //   "master_privkey": Uint8Array,
            //   "rotating_privkey": Uint8Array,
            //   "unix_ts": number,
            // }

            assertInfoLength(info, 1);
            assertIsObject(info[0]);
            auto env = info.Env();

            auto first = info[0].As<Napi::Object>();

            if (first.IsEmpty())
                throw std::invalid_argument("proProofRequestBody first received empty");

            assertIsNumber(first.Get("version"), "proProofRequestBody.version");
            assertIsNumber(first.Get("unix_ts"), "proProofRequestBody.unix_ts");
            auto version = first.Get("version").As<Napi::Number>();
            auto unix_ts = toCppSysMs(first.Get("unix_ts"), "proProofRequestBody.unix_ts");

            assertIsUInt8Array(first.Get("master_privkey"), "proProofRequestBody.master_privkey");
            assertIsUInt8Array(
                    first.Get("rotating_privkey"), "proProofRequestBody.rotating_privkey");

            // stack allocate to the buffer view so the ref doesnt get deleted
            auto master_privkey_napi = first.Get("master_privkey");
            auto rotating_privkey_napi = first.Get("rotating_privkey");
            auto master_privkey =
                    toCppBufferView(master_privkey_napi, "proProofRequestBody.master_privkey");
            auto rotating_privkey =
                    toCppBuffer(rotating_privkey_napi, "proProofRequestBody.rotating_privkey");

            assert(master_privkey.size() == 64);
            assert(rotating_privkey.size() == 64);

            pro_backend::GetProProofRequest proProofRequest{
                    .version = static_cast<uint8_t>(version.Int32Value()),
                    .unix_ts = unix_ts,
            };

            auto [master_sig, rotating_sig] = proProofRequest.build_sigs(
                    proProofRequest.version,
                    master_privkey,
                    rotating_privkey,
                    proProofRequest.unix_ts);

            assert(master_sig.size() == 64);
            assert(rotating_sig.size() == 64);

            proProofRequest.master_sig = master_sig;
            proProofRequest.rotating_sig = rotating_sig;
            memcpy(proProofRequest.master_pkey.data(), master_privkey.data(), 32);
            memcpy(proProofRequest.rotating_pkey.data(), rotating_privkey.data(), 32);

            auto json = proProofRequest.to_json();
            auto json_str = Napi::String::New(env, json);
            return json_str;
        });
    };
};

};  // namespace session::nodeapi
