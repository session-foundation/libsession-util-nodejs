#pragma once

#include <napi.h>
#include <oxenc/base64.h>
#include <oxenc/hex.h>

#include <cstddef>
#include <vector>

#include "meta/meta_base_wrapper.hpp"
#include "pro/types.hpp"
#include "session/pro_backend.h"
#include "session/pro_backend.hpp"
#include "session/session_protocol.h"
#include "session/session_protocol.hpp"
#include "utilities.hpp"

namespace session::nodeapi {

std::string_view proBackendEnumToString(SESSION_PRO_BACKEND_PAYMENT_PROVIDER v);
std::string_view proBackendEnumToString(SESSION_PRO_BACKEND_PAYMENT_STATUS v);
std::string_view proBackendEnumToString(SESSION_PRO_BACKEND_PLAN v);
std::string_view proBackendEnumToString(SESSION_PRO_BACKEND_USER_PRO_STATUS v);
std::string_view proBackendEnumToString(SESSION_PRO_BACKEND_GET_PRO_DETAILS_ERROR_REPORT v);
std::string_view proBackendEnumToString(session::ProFeaturesForMsgStatus v);

template <typename T>
Napi::Value toJsOrNullIfErrors(
        const Napi::Env& env, const T& value, const std::vector<std::string>& errors) {
    return errors.empty() ? toJs(env, value) : env.Null();
}

template <>
struct toJs_impl<pro_backend::ProRevocationItem> {
    auto operator()(const Napi::Env& env, pro_backend::ProRevocationItem i) const {

        auto obj = Napi::Object::New(env);
        obj["genIndexHashB64"] = toJs(env, to_base64(i.gen_index_hash));
        obj["expiryUnixTsMs"] = toJs(env, i.expiry_unix_ts);

        return obj;
    }
};

template <>
struct toJs_impl<pro_backend::ProPaymentItem> {
    auto operator()(const Napi::Env& env, pro_backend::ProPaymentItem p) const {

        auto obj = Napi::Object::New(env);
        obj["status"] = toJs(env, proBackendEnumToString(p.status));
        obj["plan"] = toJs(env, proBackendEnumToString(p.plan));
        obj["paymentProvider"] = toJs(env, proBackendEnumToString(p.payment_provider));

        obj["autoRenewing"] = toJs(env, p.auto_renewing);
        obj["unredeemedTsMs"] = toJs(env, p.unredeemed_unix_ts);
        obj["redeemedTsMs"] = toJs(env, p.redeemed_unix_ts);
        obj["expiryTsMs"] = toJs(env, p.expiry_unix_ts);
        obj["gracePeriodDurationMs"] = toJs(env, p.grace_period_duration_ms);
        obj["platformRefundExpiryTsMs"] = toJs(env, p.platform_refund_expiry_unix_ts);
        obj["revokedTsMs"] = toJs(env, p.revoked_unix_ts);

        obj["googlePaymentToken"] = toJs(env, p.google_payment_token);
        obj["appleOriginalTxId"] = toJs(env, p.apple_original_tx_id);
        obj["appleTxId"] = toJs(env, p.apple_tx_id);
        obj["appleWebLineOrderId"] = toJs(env, p.apple_web_line_order_id);

        return obj;
    }
};

template <>
struct toJs_impl<pro_backend::ResponseHeader> {
    auto operator()(const Napi::Env& env, pro_backend::ResponseHeader r) const {

        auto obj = Napi::Object::New(env);
        obj["status"] = toJs(env, r.status);
        obj["errors"] = toJs(env, r.errors);

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
                        StaticMethod<&ProWrapper::utf16CountTruncatedToCodepoints>(
                                "utf16CountTruncatedToCodepoints",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),
                        StaticMethod<&ProWrapper::utf16Count>(
                                "utf16Count",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),
                        StaticMethod<&ProWrapper::proFeaturesForMessage>(
                                "proFeaturesForMessage",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),

                        // Pro requests
                        StaticMethod<&ProWrapper::proProofRequestBody>(
                                "proProofRequestBody",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),
                        StaticMethod<&ProWrapper::proRevocationsRequestBody>(
                                "proRevocationsRequestBody",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),
                        StaticMethod<&ProWrapper::proStatusRequestBody>(
                                "proStatusRequestBody",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),
                });
    }

  private:
    static Napi::Value proFeaturesForMessage(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            // we expect one argument that matches:
            // first: {
            //   "utf16": string,
            // }

            assertInfoLength(info, 1);
            assertIsObject(info[0]);
            auto env = info.Env();

            auto first = info[0].As<Napi::Object>();

            if (first.IsEmpty())
                throw std::invalid_argument("proFeaturesForMessage first received empty");

            auto lossless = true;

            assertIsString(first.Get("utf16"), "proFeaturesForMessage.utf16");
            std::u16string utf16 = first.Get("utf16").As<Napi::String>().Utf16Value();
            ProFeaturesForMsg pro_features_msg =
                    session::pro_features_for_utf16((utf16.data()), utf16.length());

            auto obj = Napi::Object::New(env);

            obj["status"] = toJs(env, proBackendEnumToString(pro_features_msg.status));
            obj["error"] =
                    pro_features_msg.error.size() ? toJs(env, pro_features_msg.error) : env.Null();
            obj["codepointCount"] = toJs(env, pro_features_msg.codepoint_count);
            obj["proMessageBitset"] = proMessageBitsetToJS(env, pro_features_msg.bitset);

            return obj;
        });
    };

    static Napi::Value utf16Count(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            // we expect one argument that matches:
            // first: {
            //   "utf16": string,
            // }
            // we return an object with a single property {`codepointCount: number`}

            assertInfoLength(info, 1);
            assertIsObject(info[0]);
            auto env = info.Env();

            auto first = info[0].As<Napi::Object>();

            if (first.IsEmpty())
                throw std::invalid_argument("utf16Count first received empty");

            auto lossless = true;

            assertIsString(first.Get("utf16"), "utf16Count.utf16");
            std::u16string utf16 = first.Get("utf16").As<Napi::String>().Utf16Value();
            size_t codepoint_count = session::utf16_count(utf16);

            auto obj = Napi::Object::New(env);
            obj["codepointCount"] = toJs(env, codepoint_count);

            return obj;
        });
    };

    static Napi::Value utf16CountTruncatedToCodepoints(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            // we expect one argument that matches:
            // first: {
            //   "utf16": string,
            //   "codepointLen": number,
            // }
            // we return an object with a single property {`truncateAt: number`}

            assertInfoLength(info, 1);
            assertIsObject(info[0]);
            auto env = info.Env();

            auto first = info[0].As<Napi::Object>();

            if (first.IsEmpty())
                throw std::invalid_argument("utf16CountTruncatedToCodepoints first received empty");

            assertIsString(first.Get("utf16"), "utf16CountTruncatedToCodepoints.utf16");
            std::u16string utf16 = first.Get("utf16").As<Napi::String>().Utf16Value();
            assertIsNumber(
                    first.Get("codepointLen"), "utf16CountTruncatedToCodepoints.codepointLen");
            size_t codepointLen = first.Get("codepointLen").As<Napi::Number>().Uint32Value();

            size_t truncate_at = session::utf16_count_truncated_to_codepoints(utf16, codepointLen);

            auto obj = Napi::Object::New(env);
            obj["truncateAt"] = toJs(env, truncate_at);

            return obj;
        });
    };

    static Napi::Value proProofRequestBody(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            // we expect arguments that match:
            // first: {
            //   "requestVersion": number,
            //   "masterPrivKeyHex": string,
            //   "rotatingPrivKeyHex": string,
            //   "unixTsMs": number,
            // }

            assertInfoLength(info, 1);
            assertIsObject(info[0]);
            auto env = info.Env();

            auto first = info[0].As<Napi::Object>();
            if (first.IsEmpty())
                throw std::invalid_argument("proProofRequestBody first received empty");

            assertIsNumber(first.Get("requestVersion"), "proProofRequestBody.requestVersion");
            Napi::Number requestVersion = first.Get("requestVersion").As<Napi::Number>();
            assertIsNumber(first.Get("unixTsMs"), "proProofRequestBody.unixTsMs");
            auto unix_ts_ms = toCppSysMs(first.Get("unixTsMs"), "proProofRequestBody.unixTsMs");

            assertIsString(first.Get("masterPrivKeyHex"), "proProofRequestBody.masterPrivKeyHex");
            assertIsString(
                    first.Get("rotatingPrivKeyHex"), "proProofRequestBody.rotatingPrivKeyHex");

            auto master_privkey_js = first.Get("masterPrivKeyHex");
            auto rotating_privkey_js = first.Get("rotatingPrivKeyHex");
            std::string master_privkey =
                    toCppString(master_privkey_js, "proProofRequestBody.masterPrivKeyHex");
            std::string rotating_privkey =
                    toCppString(rotating_privkey_js, "proProofRequestBody.rotatingPrivKeyHex");

            auto master_privkey_decoded = from_hex(master_privkey);
            auto rotating_privkey_decoded = from_hex(rotating_privkey);

            std::string json = pro_backend::GenerateProProofRequest::build_to_json(
                    static_cast<uint8_t>(requestVersion.Int32Value()),
                    to_span(master_privkey_decoded),
                    to_span(rotating_privkey_decoded),
                    unix_ts_ms);

            return json;
        });
    };

    static Napi::Value proRevocationsRequestBody(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            // we expect arguments that match:
            // first: {
            //   "requestVersion": number,
            //   "ticket": number,
            // }

            assertInfoLength(info, 1);
            assertIsObject(info[0]);
            auto env = info.Env();

            auto first = info[0].As<Napi::Object>();

            if (first.IsEmpty())
                throw std::invalid_argument("proRevocationsRequestBody first received empty");

            assertIsNumber(first.Get("requestVersion"), "proRevocationsRequestBody.requestVersion");
            assertIsNumber(first.Get("ticket"), "proRevocationsRequestBody.ticket");
            auto requestVersion = first.Get("requestVersion").As<Napi::Number>();
            auto ticket = first.Get("ticket").As<Napi::Number>();

            auto revocationsRequest = pro_backend::GetProRevocationsRequest{
                    .version = static_cast<uint8_t>(requestVersion.Int32Value()),
                    .ticket = ticket.Uint32Value(),
            };

            return revocationsRequest.to_json();
        });
    };

    static Napi::Value proStatusRequestBody(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            // we expect arguments that match:
            // first: {
            //   "requestVersion": number,
            //   "masterPrivKeyHex": string,
            //   "unixTsMs": number,
            //   "count": number,
            // }

            assertInfoLength(info, 1);
            assertIsObject(info[0]);
            auto env = info.Env();

            auto first = info[0].As<Napi::Object>();

            if (first.IsEmpty())
                throw std::invalid_argument("proStatusRequestBody first received empty");

            assertIsNumber(first.Get("requestVersion"), "proStatusRequestBody.requestVersion");
            assertIsNumber(first.Get("unixTsMs"), "proStatusRequestBody.unixTsMs");
            assertIsNumber(first.Get("count"), "proStatusRequestBody.count");
            auto requestVersion = first.Get("requestVersion").As<Napi::Number>();
            auto unix_ts_ms = toCppSysMs(first.Get("unixTsMs"), "proStatusRequestBody.unixTsMs");
            auto count = toCppInteger(first.Get("count"), "proStatusRequestBody.count");
            assertIsString(first.Get("masterPrivKeyHex"), "proStatusRequestBody.masterPrivKeyHex");

            auto master_privkey_js = first.Get("masterPrivKeyHex");
            auto master_privkey =
                    toCppString(master_privkey_js, "proStatusRequestBody.masterPrivKeyHex");

            auto master_privkey_decoded = from_hex(master_privkey);

            auto json = pro_backend::GetProDetailsRequest::build_to_json(
                    static_cast<uint8_t>(requestVersion.Int32Value()),
                    to_span(master_privkey_decoded),
                    unix_ts_ms,
                    count);

            return json;
        });
    };
};

};  // namespace session::nodeapi
