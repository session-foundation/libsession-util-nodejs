#pragma once

#include <napi.h>
#include <oxenc/base64.h>
#include <oxenc/hex.h>

#include <algorithm>
#include <span>
#include <vector>

#include "../utilities.hpp"
#include "oxen/log.hpp"
#include "session/attachments.hpp"
#include "session/config/user_profile.hpp"
#include "session/multi_encrypt.hpp"
#include "session/random.hpp"

namespace session::nodeapi {

namespace log = oxen::log;

auto cat = log::Cat("multi_encrypt");

template <>
struct toJs_impl<session::ProProof> {
    Napi::Object operator()(const Napi::Env& env, const session::ProProof pro_proof) {
        auto obj = Napi::Object::New(env);

        obj["version"] = toJs(env, pro_proof.version);
        obj["genIndexHashB64"] = toJs(env, oxenc::to_base64(pro_proof.gen_index_hash));
        obj["rotatingPubkeyHex"] = toJs(env, oxenc::to_hex(pro_proof.rotating_pubkey));
        obj["expiryMs"] = toJs(env, pro_proof.expiry_unix_ts.time_since_epoch().count());

        return obj;
    }
};

template <>
struct toJs_impl<session::Envelope> {
    Napi::Object operator()(const Napi::Env& env, const session::Envelope envelope) {
        auto obj = Napi::Object::New(env);

        obj["timestampMs"] = toJs(env, envelope.timestamp.count());
        obj["source"] = envelope.source.size() ? toJs(env, envelope.source) : env.Null();
        obj["proSigHex"] =
                envelope.pro_sig.size() ? toJs(env, oxenc::to_hex(envelope.pro_sig)) : env.Null();

        return obj;
    }
};

inline std::vector<unsigned char> extractPlaintext(
        const Napi::Object& obj, const std::string identifier) {

    assertIsUInt8Array(obj.Get("plaintext"), identifier);
    auto plaintext = toCppBuffer(obj.Get("plaintext"), identifier);

    return plaintext;
}

inline std::chrono::milliseconds extractSentTimestampMs(
        const Napi::Object& obj, const std::string identifier) {
    assertIsNumber(obj.Get("sentTimestampMs"), identifier);
    auto sentTimestampMs = toCppMs(obj.Get("sentTimestampMs"), identifier);

    return sentTimestampMs;
}

inline std::span<const unsigned char> extractSenderEd25519SeedAsSpan(
        const Napi::Object& obj, const std::string identifier) {

    assertIsUInt8Array(
            obj.Get("senderEd25519Seed"), "extractSenderEd25519SeedAsSpan.senderEd25519Seed");

    auto senderEd25519Seed = toCppBuffer(obj.Get("senderEd25519Seed"), identifier);
    assert_length(senderEd25519Seed, 32, identifier);

    return senderEd25519Seed;
}

inline session::array_uc33 extractRecipientPubkeyAsArray(
        const Napi::Object& obj, const std::string identifier) {
    assertIsString(obj.Get("recipientPubkey"));
    auto recipientPubkeyHex = toCppString(obj.Get("recipientPubkey"), identifier);
    assert_length(recipientPubkeyHex, 66, identifier);

    return from_hex_to_array<33>(recipientPubkeyHex);
}

inline session::array_uc32 extractCommunityPubkeyAsArray(
        const Napi::Object& obj, const std::string identifier) {
    assertIsString(obj.Get("communityPubkey"));
    auto communityPubkeyHex = toCppString(obj.Get("communityPubkey"), identifier);
    assert_length(communityPubkeyHex, 64, identifier);

    return from_hex_to_array<32>(communityPubkeyHex);
}

inline session::array_uc33 extractGroupEd25519PubkeyAsArray(
        const Napi::Object& obj, const std::string identifier) {
    assertIsString(obj.Get("groupEd25519Pubkey"));
    std::string groupEd25519PubkeyHex = toCppString(obj.Get("groupEd25519Pubkey"), identifier);

    assert_length(groupEd25519PubkeyHex, 66, identifier);
    auto arr = from_hex_to_array<33>(groupEd25519PubkeyHex);

    return arr;
}

inline cleared_uc32 extractGroupEncKeyAsArray(
        const Napi::Object& obj, const std::string identifier) {
    assertIsString(obj.Get("groupEncKey"));

    auto groupEncKeyHex = toCppString(obj.Get("groupEncKey"), identifier);
    assert_length(groupEncKeyHex, 64, identifier);

    auto arr = from_hex_to_array<32>(groupEncKeyHex);
    cleared_uc32 result;

    std::copy(arr.begin(), arr.end(), result.begin());

    return result;
}

inline std::optional<std::span<const unsigned char>> extractProRotatingEd25519PrivKeyAsSpan(
        const Napi::Object& obj, const std::string identifier) {
    assertIsStringOrNull(obj.Get("proRotatingEd25519PrivKey"));
    auto proRotatingEd25519PrivKeyHex =
            maybeNonemptyString(obj.Get("proRotatingEd25519PrivKey"), identifier);

    if (proRotatingEd25519PrivKeyHex.has_value() && proRotatingEd25519PrivKeyHex.value().size()) {
        assert_length(*proRotatingEd25519PrivKeyHex, 64, identifier);

        auto ret = from_hex_to_span(*proRotatingEd25519PrivKeyHex);

        return ret;
    }

    return std::nullopt;
}

inline std::vector<unsigned char> extractContentOrEnvelope(
        const Napi::Object& obj, const std::string identifier) {
    assertIsUInt8Array(obj.Get("contentOrEnvelope"), identifier);
    auto contentOrEnvelope = toCppBuffer(obj.Get("contentOrEnvelope"), identifier);

    return contentOrEnvelope;
}

inline std::chrono::sys_time<std::chrono::milliseconds> extractNowSysMs(
        const Napi::Object& obj, const std::string identifier) {
    assertIsNumber(obj.Get("nowMs"), identifier);
    auto nowMs = toCppSysMs(obj.Get("nowMs"), identifier);

    return nowMs;
}

inline session::array_uc32 extractProBackendPubkeyHex(
        const Napi::Object& obj, const std::string identifier) {
    assertIsString(obj.Get("proBackendPubkeyHex"));

    auto proBackendPubkeyHex = toCppString(obj.Get("proBackendPubkeyHex"), identifier);
    assert_length(proBackendPubkeyHex, 64, identifier);

    auto arr = from_hex_to_array<32>(proBackendPubkeyHex);

    return arr;
}

class MultiEncryptWrapper : public Napi::ObjectWrap<MultiEncryptWrapper> {
  public:
    MultiEncryptWrapper(const Napi::CallbackInfo& info) :
            Napi::ObjectWrap<MultiEncryptWrapper>{info} {
        throw std::invalid_argument(
                "MultiEncryptWrapper is static and doesn't need to be constructed");
    }

    static void Init(Napi::Env env, Napi::Object exports) {
        MetaBaseWrapper::NoBaseClassInitHelper<MultiEncryptWrapper>(
                env,
                exports,
                "MultiEncryptWrapperNode",
                {
                        StaticMethod<&MultiEncryptWrapper::multiEncrypt>(
                                "multiEncrypt",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),
                        StaticMethod<&MultiEncryptWrapper::multiDecryptEd25519>(
                                "multiDecryptEd25519",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),
                        // Attachments encrypt/decrypt
                        StaticMethod<&MultiEncryptWrapper::attachmentDecrypt>(
                                "attachmentDecrypt",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),
                        StaticMethod<&MultiEncryptWrapper::attachmentEncrypt>(
                                "attachmentEncrypt",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),

                        // Destination encrypt
                        StaticMethod<&MultiEncryptWrapper::encryptFor1o1>(
                                "encryptFor1o1",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),

                        StaticMethod<&MultiEncryptWrapper::encryptForCommunity>(
                                "encryptForCommunity",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),
                        StaticMethod<&MultiEncryptWrapper::encryptForCommunityInbox>(
                                "encryptForCommunityInbox",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),
                        StaticMethod<&MultiEncryptWrapper::encryptForGroup>(
                                "encryptForGroup",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),

                        // Destination decrypt
                        // StaticMethod<&MultiEncryptWrapper::encryptFor1o1>(
                        //         "encryptFor1o1",
                        //         static_cast<napi_property_attributes>(
                        //                 napi_writable | napi_configurable)),

                        StaticMethod<&MultiEncryptWrapper::decryptForCommunity>(
                                "decryptForCommunity",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),
                });
    }

  private:
    static Napi::Value multiEncrypt(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            assertInfoLength(info, 1);
            assertIsObject(info[0]);
            auto obj = info[0].As<Napi::Object>();

            if (obj.IsEmpty())
                throw std::invalid_argument("multiEncrypt received empty");

            assertIsUInt8Array(obj.Get("ed25519SecretKey"), "multiEncrypt.ed25519SecretKey");
            auto ed25519SecretKey =
                    toCppBuffer(obj.Get("ed25519SecretKey"), "multiEncrypt.ed25519SecretKey");

            assertIsString(obj.Get("domain"));
            auto domain = toCppString(obj.Get("domain"), "multiEncrypt.domain");

            // handle the messages conversion
            auto messagesJSValue = obj.Get("messages");
            assertIsArray(messagesJSValue, "multiEncrypt");
            auto messagesJS = messagesJSValue.As<Napi::Array>();
            std::vector<std::vector<unsigned char>> messages;
            messages.reserve(messagesJS.Length());
            for (uint32_t i = 0; i < messagesJS.Length(); i++) {
                auto itemValue = messagesJS.Get(i);
                assertIsUInt8Array(itemValue, "multiEncrypt.itemValue.message");
                auto item = toCppBuffer(itemValue, "multiEncrypt.itemValue.message");
                messages.push_back(item);
            }

            // handle the recipients conversion
            auto recipientsJSValue = obj.Get("recipients");
            assertIsArray(recipientsJSValue, "multiEncrypt");
            auto recipientsJS = recipientsJSValue.As<Napi::Array>();
            std::vector<std::vector<unsigned char>> recipients;
            recipients.reserve(recipientsJS.Length());
            for (uint32_t i = 0; i < recipientsJS.Length(); i++) {
                auto itemValue = recipientsJS.Get(i);
                assertIsUInt8Array(itemValue, "multiEncrypt.itemValue.recipient");
                auto item = toCppBuffer(itemValue, "multiEncrypt.itemValue.recipient");
                recipients.push_back(item);
            }
            std::vector<unsigned char> random_nonce = session::random::random(24);

            std::vector<std::span<const unsigned char>> messages_sv(
                    messages.begin(), messages.end());
            std::vector<std::span<const unsigned char>> recipients_sv(
                    recipients.begin(), recipients.end());

            // Note: this function needs the first 2 args to be vector of sv explicitly
            return session::encrypt_for_multiple_simple(
                    messages_sv, recipients_sv, ed25519SecretKey, domain, random_nonce);
        });
    };

    static Napi::Value multiDecryptEd25519(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            assertInfoLength(info, 1);
            assertIsObject(info[0]);
            auto obj = info[0].As<Napi::Object>();

            if (obj.IsEmpty())
                throw std::invalid_argument("multiDecryptEd25519 received empty");

            assertIsUInt8Array(obj.Get("encoded"), "multiDecryptEd25519.encoded");
            auto encoded = toCppBuffer(obj.Get("encoded"), "multiDecryptEd25519.encoded");

            assertIsUInt8Array(
                    obj.Get("userEd25519SecretKey"), "multiDecryptEd25519.userEd25519SecretKey");
            auto ed25519_secret_key = toCppBuffer(
                    obj.Get("userEd25519SecretKey"), "multiDecryptEd25519.userEd25519SecretKey");

            assertIsUInt8Array(
                    obj.Get("senderEd25519Pubkey"), "multiDecryptEd25519.senderEd25519Pubkey");
            auto sender_ed25519_pubkey = toCppBuffer(
                    obj.Get("senderEd25519Pubkey"), "multiDecryptEd25519.senderEd25519Pubkey");

            assertIsString(obj.Get("domain"));
            auto domain = toCppString(obj.Get("domain"), "multiDecryptEd25519.domain");

            return session::decrypt_for_multiple_simple_ed25519(
                    encoded, ed25519_secret_key, sender_ed25519_pubkey, domain);
        });
    };

    /**
     * ===========================================
     * =========== ATTACHMENTS CALLS =============
     * ===========================================
     */

    static Napi::Value attachmentEncrypt(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            assertInfoLength(info, 1);
            assertIsObject(info[0]);
            auto obj = info[0].As<Napi::Object>();

            if (obj.IsEmpty())
                throw std::invalid_argument("attachmentEncrypt received empty");

            assertIsUInt8Array(obj.Get("seed"), "attachmentEncrypt.seed");
            auto seed = toCppBuffer(obj.Get("seed"), "attachmentEncrypt.seed");

            assertIsUInt8Array(obj.Get("data"), "attachmentEncrypt.data");
            auto data = toCppBuffer(obj.Get("data"), "attachmentEncrypt.data");

            assertIsString(obj.Get("domain"));
            auto domain = toCppString(obj.Get("domain"), "attachmentEncrypt.domain");

            assertIsBoolean(obj.Get("allowLarge"));

            auto allow_large = toCppBoolean(obj.Get("allowLarge"), "attachmentEncrypt.allowLarge");

            if (domain != "attachment" && domain != "profilePic") {
                throw std::invalid_argument(
                        "attachmentEncrypt.domain must be either 'attachment' or 'profilePic'");
            }

            session::attachment::Domain attachment_domain =
                    domain == "attachment" ? session::attachment::Domain::ATTACHMENT
                                           : session::attachment::Domain::PROFILE_PIC;

            std::vector<std::byte> seed_bytes(
                    reinterpret_cast<const std::byte*>(seed.data()),
                    reinterpret_cast<const std::byte*>(seed.data() + seed.size()));

            std::vector<std::byte> data_bytes(
                    reinterpret_cast<const std::byte*>(data.data()),
                    reinterpret_cast<const std::byte*>(data.data() + data.size()));
            auto encrypted = session::attachment::encrypt(
                    seed_bytes, data_bytes, attachment_domain, allow_large);

            auto ret = Napi::Object::New(info.Env());
            ret.Set("encryptedData", toJs(info.Env(), encrypted.first));
            ret.Set("encryptionKey", toJs(info.Env(), encrypted.second));

            return ret;
        });
    };

    static Napi::Value attachmentDecrypt(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            assertInfoLength(info, 1);
            assertIsObject(info[0]);
            auto obj = info[0].As<Napi::Object>();

            if (obj.IsEmpty())
                throw std::invalid_argument("attachmentDecrypt received empty");

            assertIsUInt8Array(obj.Get("encryptedData"), "attachmentDecrypt.encryptedData");
            auto encrypted_data =
                    toCppBuffer(obj.Get("encryptedData"), "attachmentDecrypt.encryptedData");

            assertIsUInt8Array(obj.Get("decryptionKey"), "attachmentDecrypt.decryptionKey");
            auto decryption_key =
                    toCppBuffer(obj.Get("decryptionKey"), "attachmentDecrypt.decryptionKey");

            std::vector<std::byte> encrypted_data_bytes(
                    reinterpret_cast<const std::byte*>(encrypted_data.data()),
                    reinterpret_cast<const std::byte*>(
                            encrypted_data.data() + encrypted_data.size()));

            std::vector<std::byte> decryption_key_bytes(
                    reinterpret_cast<const std::byte*>(decryption_key.data()),
                    reinterpret_cast<const std::byte*>(
                            decryption_key.data() + decryption_key.size()));

            if (decryption_key_bytes.size() != session::attachment::ENCRYPT_KEY_SIZE) {
                throw std::invalid_argument("Key size mismatch");
            }

            std::span<const std::byte, session::attachment::ENCRYPT_KEY_SIZE> decryption_key_span(
                    decryption_key_bytes.data(), session::attachment::ENCRYPT_KEY_SIZE);

            auto decrypted =
                    session::attachment::decrypt(encrypted_data_bytes, decryption_key_span);

            auto ret = Napi::Object::New(info.Env());
            ret.Set("decryptedData", toJs(info.Env(), decrypted));

            return ret;
        });
    };

    /**
     * ===========================================
     * ============= ENCRYPT CALLS ===============
     * ===========================================
     */

    static Napi::Value encryptFor1o1(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            // we expect a single argument which is an array of objects with the following
            // properties:
            // {
            //   "plaintext": Uint8Array,
            //   "sentTimestampMs": Number,
            //   "senderEd25519Seed": Hexstring,
            //   "recipientPubkey": Hexstring,
            //   "proRotatingEd25519PrivKey": Hexstring | null,
            // }
            //

            assertInfoLength(info, 1);
            assertIsArray(info[0], "encryptFor1o1 info[0]");

            auto array = info[0].As<Napi::Array>();

            if (array.IsEmpty())
                throw std::invalid_argument("encryptFor1o1 received empty");

            std::vector<std::vector<uint8_t>> ready_to_send(array.Length());
            for (uint32_t i = 0; i < array.Length(); i++) {
                auto itemValue = array.Get(i);
                if (!itemValue.IsObject()) {
                    throw std::invalid_argument("encryptFor1o1 itemValue is not an object");
                }
                auto obj = itemValue.As<Napi::Object>();

                ready_to_send[i] = session::encode_for_1o1(
                        extractPlaintext(obj, "encryptFor1o1.obj.plaintext"),
                        extractSenderEd25519SeedAsSpan(obj, "encryptFor1o1.obj.senderEd25519Seed"),
                        extractSentTimestampMs(obj, "encryptFor1o1.obj.sentTimestampMs"),
                        extractRecipientPubkeyAsArray(obj, "encryptFor1o1.obj.recipientPubkey"),
                        extractProRotatingEd25519PrivKeyAsSpan(
                                obj, "encryptFor1o1.obj.proRotatingEd25519PrivKey"));
            }

            auto ret = Napi::Object::New(info.Env());
            ret.Set("encryptedData", toJs(info.Env(), ready_to_send));

            return ret;
        });
    };

    static Napi::Value encryptForCommunityInbox(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            // we expect a single argument which is an array of objects with the following
            // properties:
            // {
            //   "plaintext": Uint8Array,
            //   "senderEd25519Seed": Hexstring,
            //   "sentTimestampMs": Number,
            //   "recipientPubkey": Hexstring,
            //   "communityPubkey": Hexstring,
            //   "proRotatingEd25519PrivKey": Hexstring | null,
            // }
            //

            assertInfoLength(info, 1);
            assertIsArray(info[0], "encryptForCommunityInbox info[0]");

            auto array = info[0].As<Napi::Array>();

            if (array.IsEmpty())
                throw std::invalid_argument("encryptForCommunityInbox received empty");

            std::vector<std::vector<uint8_t>> ready_to_send(array.Length());
            for (uint32_t i = 0; i < array.Length(); i++) {
                auto itemValue = array.Get(i);
                if (!itemValue.IsObject()) {
                    throw std::invalid_argument(
                            "encryptForCommunityInbox itemValue is not an object");
                }
                auto obj = itemValue.As<Napi::Object>();

                ready_to_send[i] = session::encode_for_community_inbox(
                        extractPlaintext(obj, "encryptForCommunityInbox.obj.plaintext"),
                        extractSenderEd25519SeedAsSpan(
                                obj, "encryptForCommunityInbox.obj.senderEd25519Seed"),
                        extractSentTimestampMs(obj, "encryptForCommunityInbox.obj.sentTimestampMs"),
                        extractRecipientPubkeyAsArray(
                                obj, "encryptForCommunityInbox.obj.recipientPubkey"),
                        extractCommunityPubkeyAsArray(
                                obj, "encryptForCommunityInbox.obj.communityPubkey"),
                        extractProRotatingEd25519PrivKeyAsSpan(
                                obj, "encryptForCommunityInbox.obj.proRotatingEd25519PrivKey"));
            }

            auto ret = Napi::Object::New(info.Env());
            ret.Set("encryptedData", toJs(info.Env(), ready_to_send));

            return ret;
        });
    };

    static Napi::Value encryptForCommunity(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            // we expect a single argument which is an array of objects with the following
            // properties:
            // {
            //   "plaintext": Uint8Array,
            //   "proRotatingEd25519PrivKey": Hexstring | null,
            // }
            //

            assertInfoLength(info, 1);
            assertIsArray(info[0], "encryptForCommunity info[0]");

            auto array = info[0].As<Napi::Array>();

            if (array.IsEmpty())
                throw std::invalid_argument("encryptForCommunity received empty");

            std::vector<std::vector<uint8_t>> ready_to_send(array.Length());
            for (uint32_t i = 0; i < array.Length(); i++) {
                auto itemValue = array.Get(i);
                if (!itemValue.IsObject()) {
                    throw std::invalid_argument("encryptForCommunity itemValue is not an object");
                }
                auto obj = itemValue.As<Napi::Object>();

                ready_to_send[i] = session::encode_for_community(
                        extractPlaintext(obj, "encryptForCommunity.obj.plaintext"),
                        extractProRotatingEd25519PrivKeyAsSpan(
                                obj, "encryptForCommunity.obj.proRotatingEd25519PrivKey"));
            }

            auto ret = Napi::Object::New(info.Env());
            ret.Set("encryptedData", toJs(info.Env(), ready_to_send));

            return ret;
        });
    };

    static Napi::Value encryptForGroup(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            // we expect a single argument which is an array of objects with the following
            // properties:
            // {
            //   "plaintext": Uint8Array,
            //   "senderEd25519Seed": Uint8Array, 32 bytes
            //   "sentTimestampMs": Number,
            //   "groupEd25519Pubkey": Hexstring,
            //   "groupEncKey": Hexstring,
            //   "proRotatingEd25519PrivKey": Hexstring | null,
            // }
            //

            assertInfoLength(info, 1);
            assertIsArray(info[0], "encryptForGroup info[0]");

            auto array = info[0].As<Napi::Array>();

            if (array.IsEmpty())
                throw std::invalid_argument("encryptForGroup received empty");

            std::vector<std::vector<uint8_t>> ready_to_send(array.Length());
            for (uint32_t i = 0; i < array.Length(); i++) {
                auto itemValue = array.Get(i);
                if (!itemValue.IsObject()) {
                    throw std::invalid_argument("encryptForGroup itemValue is not an object");
                }
                auto obj = itemValue.As<Napi::Object>();

                auto plaintext = extractPlaintext(obj, "encryptForGroup.obj.plaintext");
                auto senderEd25519Seed = extractSenderEd25519SeedAsSpan(
                        obj, "encryptForGroup.obj.senderEd25519Seed");

                auto sentTimestampMs =
                        extractSentTimestampMs(obj, "encryptForGroup.obj.sentTimestampMs");

                auto groupEd25519Pubkey = extractGroupEd25519PubkeyAsArray(
                        obj, "encryptForGroup.obj.recipientPubkey");

                auto groupEncKey =
                        extractGroupEncKeyAsArray(obj, "encryptForGroup.obj.groupEncKey");
                auto proRotatingEd25519PrivKey = extractProRotatingEd25519PrivKeyAsSpan(
                        obj, "encryptForGroup.obj.proRotatingEd25519PrivKey");

                ready_to_send[i] = session::encode_for_group(
                        plaintext,
                        senderEd25519Seed,
                        sentTimestampMs,
                        groupEd25519Pubkey,
                        groupEncKey,
                        proRotatingEd25519PrivKey);
            }

            auto ret = Napi::Object::New(info.Env());
            ret.Set("encryptedData", toJs(info.Env(), ready_to_send));

            return ret;
        });
    };

    /**
     * ===========================================
     * ============= DECRYPT CALLS ===============
     * ===========================================
     */

    static Napi::Value decryptForCommunity(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            // we expect two arguments that match:
            // first: [{
            //   "contentOrEnvelope": Uint8Array,
            // }],
            // second: {
            //   "nowMs": number,
            //   "proBackendPubkeyHex": Hexstring,
            //  }
            //

            assertInfoLength(info, 2);
            assertIsArray(info[0], "decryptForCommunity info[0]");
            assertIsObject(info[1]);

            auto first = info[0].As<Napi::Array>();

            if (first.IsEmpty())
                throw std::invalid_argument("decryptForCommunity first received empty");

            auto second = info[1].As<Napi::Array>();

            if (second.IsEmpty())
                throw std::invalid_argument("decryptForCommunity second received empty");

            auto nowMs = extractNowSysMs(second, "decryptForCommunity.second.nowMs");
            auto proBackendPubkeyHex = extractProBackendPubkeyHex(
                    second, "decryptForCommunity.second.proBackendPubkeyHex");

            std::vector<DecodedCommunityMessage> decrypted(first.Length());

            for (uint32_t i = 0; i < first.Length(); i++) {
                auto itemValue = first.Get(i);
                if (!itemValue.IsObject()) {
                    throw std::invalid_argument(
                            "decryptForCommunity itemValue is not an "
                            "object");
                }
                auto obj = itemValue.As<Napi::Object>();

                try {
                    decrypted[i] = session::decode_for_community(
                            extractContentOrEnvelope(
                                    obj, "decryptForCommunity.obj.contentOrEnvelope"),
                            nowMs,
                            proBackendPubkeyHex);

                } catch (const std::exception& e) {
                    log::warning(
                            cat,
                            "decryptForCommunity: Failed to decrypt "
                            "message at index {}",
                            i);
                }
            }

            auto ret = Napi::Array::New(info.Env(), decrypted.size());
            uint32_t i = 0;
            for (auto& d : decrypted) {
                auto to_insert = Napi::Object::New(info.Env());
                std::span<unsigned char> content_plaintext_unpadded =
                        std::span(d.content_plaintext)
                                .subspan(0, d.content_plaintext_unpadded_size);

                to_insert.Set(
                        "contentPlaintextUnpadded", toJs(info.Env(), content_plaintext_unpadded));
                to_insert.Set(
                        "envelope", d.envelope ? toJs(info.Env(), *d.envelope) : info.Env().Null());

                if (d.pro_sig)
                    to_insert.Set("proSigHex", toJs(info.Env(), oxenc::to_hex(*d.pro_sig)));
                else
                    to_insert.Set("proSigHex", info.Env().Null());

                if (!d.pro.has_value())
                    to_insert.Set("proProof", info.Env().Null());
                else
                    to_insert.Set("proProof", toJs(info.Env(), d.pro->proof));

                ret.Set(i, to_insert);
                i++;
            }

            return ret;
        });
    };
};
};  // namespace session::nodeapi
