#pragma once

#include <napi.h>
#include <oxenc/base64.h>
#include <oxenc/hex.h>

#include <vector>

#include "../utilities.hpp"
#include "meta/meta_base_wrapper.hpp"
#include "session/session_protocol.hpp"

namespace session::nodeapi {

template <>
struct toJs_impl<session::ProFeaturesForMsg> {
    Napi::Object operator()(
            const Napi::Env& env, const session::ProFeaturesForMsg pro_features_msg) {
        auto obj = Napi::Object::New(env);

        obj["success"] = toJs(env, pro_features_msg.success);
        obj["error"] = toJs(env, pro_features_msg.error);
        obj["codepointCount"] = toJs(env, pro_features_msg.codepoint_count);
        obj["proFeatures"] = toJs(env, pro_features_msg.features);

        return obj;
    }
};
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
                });
    }

  private:
    static Napi::Value proFeaturesForMessage(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] () {
            // we expect two arguments that match:
            // first: {
            //   "utf16": string,
            //   "proFeatures": Array<ProFeature>,
            // }

            assertInfoLength(info, 1);
            assertIsObject(info[0]);

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

            SESSION_PROTOCOL_PRO_EXTRA_FEATURES flags;
            for (std::string& feature : proFeatures) {
                if (feature == "10K_CHARACTER_LIMIT") {
                    flags |= SESSION_PROTOCOL_PRO_FEATURES_10K_CHARACTER_LIMIT;
                } else if (feature == "PRO_BADGE") {
                    flags |= SESSION_PROTOCOL_PRO_FEATURES_PRO_BADGE;
                } else if (feature == "ANIMATED_AVATAR") {
                    flags |= SESSION_PROTOCOL_PRO_FEATURES_ANIMATED_AVATAR;
                }
            }
            assertIsString(first.Get("utf16"), "proFeaturesForMessage.utf16");
            std::u16string utf16 = first.Get("utf16").As<Napi::String>().Utf16Value();
            return session::pro_features_for_utf16((utf16.data()), utf16.length(), flags);
        });
    };
};
};  // namespace session::nodeapi
