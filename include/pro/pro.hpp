#pragma once

#include <napi.h>
#include <oxenc/base64.h>
#include <oxenc/hex.h>

#include <chrono>
#include <cstddef>
#include <vector>

#include "meta/meta_base_wrapper.hpp"
#include "pro/types.hpp"
#include "session/pro_backend.hpp"
#include "session/session_protocol.hpp"
#include "utilities.hpp"

namespace session::nodeapi {

using namespace std::literals;

std::string_view proBackendEnumToString(session::ProFeaturesForMsgStatus v);

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

                        // Pro requests -> {endpoint, body}
                        StaticMethod<&ProWrapper::proProofRequest>(
                                "proProofRequest",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),
                        StaticMethod<&ProWrapper::proRevocationsRequest>(
                                "proRevocationsRequest",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),
                        StaticMethod<&ProWrapper::proStatusRequest>(
                                "proStatusRequest",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),

                        // Pro response parsers -> typed structs (libsession is the source of truth)
                        StaticMethod<&ProWrapper::parseProProofResponse>(
                                "parseProProofResponse",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),
                        StaticMethod<&ProWrapper::parseRevocationsResponse>(
                                "parseRevocationsResponse",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),
                        StaticMethod<&ProWrapper::parseProStatusResponse>(
                                "parseProStatusResponse",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),

                        // Per-provider support/management URLs (or null)
                        StaticMethod<&ProWrapper::providerUrls>(
                                "providerUrls",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),

                        // Purchasable payment-provider slugs to surface to users
                        StaticMethod<&ProWrapper::visiblePlatforms>(
                                "visiblePlatforms",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),
                });
    }

  private:
    // The wire carries whole-second timestamps; the JS domain is milliseconds, so we convert at
    // this boundary (the signed request timestamp round-trips losslessly as the value is always
    // seconds).
    static std::chrono::sys_seconds unixTsMsToSeconds(Napi::Value v, const std::string& id) {
        return std::chrono::floor<std::chrono::seconds>(toCppSysMs(v, id));
    }

    static Napi::Object proRequestToJs(const Napi::Env& env, const pro_backend::ProRequest& req) {
        auto obj = Napi::Object::New(env);
        obj["endpoint"] = toJs(env, req.endpoint);
        obj["contentType"] = toJs(env, req.content_type);
        // C++ member is the opaque `data` payload; expose it under the JS key `body` (what desktop
        // reads)
        obj["body"] = toJs(env, req.data);
        return obj;
    }

    // §5: the response envelope is a CLOSED status enum + an optional machine slug (error_code, §5.1)
    // + an optional English diagnostic (error) — no more errors[] array. Render status as its wire
    // string ("ok"/"fail"/"error") for JS consumers; error_code/error are null on success.
    static std::string_view responseStatusToJs(session::pro_backend::ResponseStatus s) {
        using RS = session::pro_backend::ResponseStatus;
        switch (s) {
            case RS::Ok: return "ok";
            case RS::Fail: return "fail";
            case RS::Error: return "error";
        }
        return "error";  // fail-closed on an unexpected value
    }

    static void emitResponseHeader(
            const Napi::Env& env,
            Napi::Object& obj,
            const session::pro_backend::ResponseBase& resp) {
        obj["status"] = toJs(env, responseStatusToJs(resp.status));
        obj["errorCode"] = resp.error_code ? toJs(env, *resp.error_code) : env.Null();
        obj["error"] = resp.error ? toJs(env, *resp.error) : env.Null();
    }

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

    static Napi::Value proProofRequest(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            // first: {
            //   "masterPrivKeyHex": string,
            //   "rotatingPrivKeyHex": string,
            //   "unixTsMs": number,
            // }

            assertInfoLength(info, 1);
            assertIsObject(info[0]);
            auto env = info.Env();

            auto first = info[0].As<Napi::Object>();
            if (first.IsEmpty())
                throw std::invalid_argument("proProofRequest first received empty");

            assertIsNumber(first.Get("unixTsMs"), "proProofRequest.unixTsMs");
            auto unix_ts = unixTsMsToSeconds(first.Get("unixTsMs"), "proProofRequest.unixTsMs");

            assertIsString(first.Get("masterPrivKeyHex"), "proProofRequest.masterPrivKeyHex");
            assertIsString(first.Get("rotatingPrivKeyHex"), "proProofRequest.rotatingPrivKeyHex");
            auto master_privkey = from_hex(
                    toCppString(first.Get("masterPrivKeyHex"), "proProofRequest.masterPrivKeyHex"));
            auto rotating_privkey = from_hex(toCppString(
                    first.Get("rotatingPrivKeyHex"), "proProofRequest.rotatingPrivKeyHex"));

            auto req = session::pro_backend::pro_proof_request(
                    to_span(master_privkey), to_span(rotating_privkey), unix_ts);

            return proRequestToJs(env, req);
        });
    };

    static Napi::Value proRevocationsRequest(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            // first: {
            //   "ticket": number,   // 64-bit; 0 if unknown
            // }

            assertInfoLength(info, 1);
            assertIsObject(info[0]);
            auto env = info.Env();

            auto first = info[0].As<Napi::Object>();
            if (first.IsEmpty())
                throw std::invalid_argument("proRevocationsRequest first received empty");

            assertIsNumber(first.Get("ticket"), "proRevocationsRequest.ticket");
            auto ticket = first.Get("ticket").As<Napi::Number>().Int64Value();

            auto req = session::pro_backend::revocations_request(ticket);

            return proRequestToJs(env, req);
        });
    };

    static Napi::Value proStatusRequest(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            // first: {
            //   "masterPrivKeyHex": string,
            //   "unixTsMs": number,
            //   "count": number,
            // }

            assertInfoLength(info, 1);
            assertIsObject(info[0]);
            auto env = info.Env();

            auto first = info[0].As<Napi::Object>();
            if (first.IsEmpty())
                throw std::invalid_argument("proStatusRequest first received empty");

            assertIsNumber(first.Get("unixTsMs"), "proStatusRequest.unixTsMs");
            auto unix_ts = unixTsMsToSeconds(first.Get("unixTsMs"), "proStatusRequest.unixTsMs");

            assertIsString(first.Get("masterPrivKeyHex"), "proStatusRequest.masterPrivKeyHex");
            auto master_privkey = from_hex(toCppString(
                    first.Get("masterPrivKeyHex"), "proStatusRequest.masterPrivKeyHex"));

            // get_pro_status: the light "am I Pro?" query (no count/limit — a single latest payment
            // comes back in the response). Payment history (get_payment_details) is not wired.
            auto req = session::pro_backend::pro_status_request(to_span(master_privkey), unix_ts);

            return proRequestToJs(env, req);
        });
    };

    // The response body is relayed RAW from the network (the client never parses it — the wire
    // format is a contract between libsession and the backend only). Callers pass the raw bytes as
    // a Uint8Array; we hand them straight to libsession's parser, no client-side
    // decoding/assumption.
    static std::vector<unsigned char> requestBodyBytes(
            const Napi::CallbackInfo& info, const std::string& id) {
        assertInfoLength(info, 1);
        assertIsObject(info[0]);
        auto first = info[0].As<Napi::Object>();
        if (first.IsEmpty())
            throw std::invalid_argument(id + " first received empty");
        assertIsUInt8Array(first.Get("body"), id + ".body");
        return toCppBuffer(first.Get("body"), id + ".body");
    }

    static std::string_view asJsonView(const std::vector<unsigned char>& body) {
        return std::string_view(reinterpret_cast<const char*>(body.data()), body.size());
    }

    static Napi::Value parseProProofResponse(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            auto env = info.Env();
            auto body = requestBodyBytes(info, "parseProProofResponse");

            auto resp = session::pro_backend::parse_pro_proof(asJsonView(body));

            auto obj = Napi::Object::New(env);
            emitResponseHeader(env, obj, resp);
            obj["proof"] = toJs(env, resp.proof);
            return obj;
        });
    };

    static Napi::Value parseRevocationsResponse(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            auto env = info.Env();
            auto body = requestBodyBytes(info, "parseRevocationsResponse");

            auto resp = session::pro_backend::parse_revocations(asJsonView(body));

            auto obj = Napi::Object::New(env);
            emitResponseHeader(env, obj, resp);
            obj["ticket"] = toJs(env, resp.ticket);
            // The backend returns a retry *delay*; resolve it to the absolute unix instant (ms) at
            // which the revocation list may next be polled, clamped so it is never in the past.
            // Handing back an absolute instant lets callers schedule the next poll without needing
            // a clock of their own.
            auto retryIn = std::max(resp.retry_in, 0s);
            auto retryAt = std::chrono::system_clock::now() + retryIn;
            obj["retryAtMs"] = toJsMs(env, std::chrono::floor<std::chrono::milliseconds>(retryAt));
            // retain_for stays a duration (applied per item as seen + retain_for); milliseconds for
            // nodejs.
            obj["retainForMs"] = toJsMs(env, resp.retain_for);

            auto items = Napi::Array::New(env, resp.items.size());
            for (size_t i = 0; i < resp.items.size(); i++) {
                auto item = Napi::Object::New(env);
                item["revocationTagB64"] =
                        toJs(env, oxenc::to_base64(resp.items[i].revocation_tag));
                item["effectiveMs"] = toJsMs(env, resp.items[i].effective_at);
                items.Set(i, item);
            }
            obj["items"] = items;
            return obj;
        });
    };

    // Parsed plan unit -> lowercase slug for the JS domain to localize (plan grammar, §1).
    static std::string_view planUnitToString(session::pro_backend::ProPlanUnit u) {
        using U = session::pro_backend::ProPlanUnit;
        switch (u) {
            case U::second: return "second";
            case U::day: return "day";
            case U::week: return "week";
            case U::month: return "month";
            case U::year: return "year";
            case U::lifetime: return "lifetime";
        }
        return "";
    }

    // Emit a single ProPaymentItem: ms timestamps, plan as {planCount, planUnit}, opaque slugs.
    static Napi::Object paymentItemToJs(
            const Napi::Env& env, const session::pro_backend::ProPaymentItem& src) {
        auto item = Napi::Object::New(env);
        item["status"] = toJs(env, src.status);  // opaque status slug; pass through
        item["planCount"] = toJs(env, src.plan.count);
        item["planUnit"] = toJs(env, planUnitToString(src.plan.unit));
        item["paymentProvider"] = toJs(env, src.payment_provider);
        item["autoRenewing"] = toJs(env, src.auto_renewing);
        // purchased/revoked carry sub-second (ms) precision; the rest are whole seconds. toJsMs
        // normalises every one of them to the ms JS domain (see utilities.hpp).
        item["purchasedTsMs"] = toJsMs(env, src.purchased_at);
        item["revokedTsMs"] = toJsMs(env, src.revoked_at);
        item["redeemedTsMs"] = toJsMs(env, src.redeemed_at);
        item["expiryTsMs"] = toJsMs(env, src.expiry_at);
        item["gracePeriodDurationMs"] = toJsMs(env, src.grace_period_duration);
        item["platformRefundExpiryTsMs"] = toJsMs(env, src.platform_refund_expiry_at);
        item["refundRequestedTsMs"] = toJsMs(env, src.refund_requested_at);
        item["paymentId"] = toJs(env, src.payment_id);
        return item;
    }

    // get_pro_status: the light entitlement query. Carries user_status + expiry/grace/refund + a
    // single most-recent payment (latestPayment, or null). Payment history (get_payment_details) is
    // a separate library-only endpoint, not wired here.
    static Napi::Value parseProStatusResponse(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            auto env = info.Env();
            auto body = requestBodyBytes(info, "parseProStatusResponse");

            auto resp = session::pro_backend::parse_pro_status(asJsonView(body));

            auto obj = Napi::Object::New(env);
            emitResponseHeader(env, obj, resp);
            // user_status: opaque string code (never/active/expired; unknowns pass through)
            obj["userStatus"] = toJs(env, resp.user_status);
            obj["errorReport"] = toJs(env, static_cast<uint32_t>(resp.error_report));
            obj["autoRenewing"] = toJs(env, resp.auto_renewing);
            obj["expiryMs"] = toJsMs(env, resp.expiry_at);
            obj["gracePeriodDurationMs"] = toJsMs(env, resp.grace_period_duration);
            obj["refundRequestedTsMs"] = toJsMs(env, resp.refund_requested_at);
            if (resp.latest_payment) {
                obj["latestPayment"] = paymentItemToJs(env, *resp.latest_payment);
            } else {
                obj["latestPayment"] = env.Null();
            }
            return obj;
        });
    };

    static Napi::Value providerUrls(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&]() -> Napi::Value {
            // first: {
            //   "code": string,   // provider slug, e.g. "app_store"
            // }

            assertInfoLength(info, 1);
            assertIsObject(info[0]);
            auto env = info.Env();

            auto first = info[0].As<Napi::Object>();
            if (first.IsEmpty())
                throw std::invalid_argument("providerUrls first received empty");

            assertIsString(first.Get("code"), "providerUrls.code");
            auto code = toCppString(first.Get("code"), "providerUrls.code");

            auto urls = session::pro_backend::provider_urls(code);
            if (!urls)
                return env.Null();

            auto obj = Napi::Object::New(env);
            obj["refundPlatformUrl"] = toJs(env, urls->refund_platform_url);
            obj["refundSupportUrl"] = toJs(env, urls->refund_support_url);
            obj["refundStatusUrl"] = toJs(env, urls->refund_status_url);
            obj["updateSubscriptionUrl"] = toJs(env, urls->update_subscription_url);
            obj["cancelSubscriptionUrl"] = toJs(env, urls->cancel_subscription_url);
            return obj;
        });
    };

    // The purchasable payment-provider slugs to surface to users (single source of truth in
    // libsession; excludes non-purchasable providers like rangeproof). Order is not significant.
    static Napi::Value visiblePlatforms(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&]() -> Napi::Value {
            auto env = info.Env();
            auto platforms = session::pro_backend::visible_platforms();  // span<const string_view>
            auto arr = Napi::Array::New(env, platforms.size());
            uint32_t i = 0;
            for (auto slug : platforms)
                arr[i++] = Napi::String::New(env, std::string(slug));
            return arr;
        });
    };
};

};  // namespace session::nodeapi
