#pragma once

#include <napi.h>
#include <oxenc/hex.h>

#include <algorithm>
#include <span>
#include <vector>

#include "../utilities.hpp"
#include "session/attachments.hpp"
#include "session/config/user_profile.hpp"
#include "session/multi_encrypt.hpp"
#include "session/random.hpp"

namespace session::nodeapi {

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

inline std::span<const unsigned char> extractSenderEd25519PrivkeyAsSpan(
        const Napi::Object& obj, const std::string identifier) {
    assertIsString(obj.Get("senderEd25519Privkey"));
    auto ed25519PrivkeyHex = toCppString(obj.Get("senderEd25519Privkey"), identifier);
    return from_hex_to_span(ed25519PrivkeyHex);
}

inline session::array_uc33 extractRecipientPubkeyAsArray(
        const Napi::Object& obj, const std::string identifier) {
    assertIsString(obj.Get("recipientPubkey"));
    auto recipientPubkeyHex = toCppString(obj.Get("recipientPubkey"), identifier);
    return from_hex_to_array<33>(recipientPubkeyHex);
}

inline session::array_uc32 extractCommunityPubkeyAsArray(
        const Napi::Object& obj, const std::string identifier) {
    assertIsString(obj.Get("communityPubkey"));
    auto communityPubkeyHex = toCppString(obj.Get("communityPubkey"), identifier);
    return from_hex_to_array<32>(communityPubkeyHex);
}

inline session::array_uc33 extractGroupEd25519PubkeyAsArray(
        const Napi::Object& obj, const std::string identifier) {
    assertIsString(obj.Get("groupEd25519Pubkey"));
    auto communityPubkeyHex = toCppString(obj.Get("groupEd25519Pubkey"), identifier);
    return from_hex_to_array<33>(communityPubkeyHex);
}

inline cleared_uc32 extractGroupEncPrivKeyAsArray(
        const Napi::Object& obj, const std::string identifier) {
    assertIsString(obj.Get("groupEncPrivKey"));
    auto groupEncPrivKeyHex = toCppString(obj.Get("groupEncPrivKey"), identifier);
    auto arr = from_hex_to_array<32>(groupEncPrivKeyHex);
    cleared_uc32 result;
    std::copy(arr.begin(), arr.end(), result.begin());
    return result;
}

inline std::span<const unsigned char> extractProRotatingEd25519PrivkeyAsSpan(
        const Napi::Object& obj, const std::string identifier) {
    assertIsStringOrNull(obj.Get("proRotatingEd25519Privkey"));
    auto proRotatingEd25519PrivkeyHex =
            maybeNonemptyString(obj.Get("proRotatingEd25519Privkey"), identifier);
    return from_hex_to_span(proRotatingEd25519PrivkeyHex.value_or(""));
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

    static Napi::Value encryptFor1o1(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            // we expect an single argument which is an array of objects with the following
            // properties:
            // {
            //   "plaintext": Uint8Array,
            //   "sentTimestampMs": Number,
            //   "senderEd25519Privkey": Hexstring,
            //   "recipientPubkey": Hexstring,
            //   "proRotatingEd25519Privkey": Hexstring | null,
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
                        extractSenderEd25519PrivkeyAsSpan(
                                obj, "encryptFor1o1.obj.senderEd25519Privkey"),
                        extractSentTimestampMs(obj, "encryptFor1o1.obj.sentTimestampMs"),
                        extractRecipientPubkeyAsArray(obj, "encryptFor1o1.obj.recipientPubkey"),
                        extractProRotatingEd25519PrivkeyAsSpan(
                                obj, "encryptFor1o1.obj.proRotatingEd25519Privkey"));
            }

            auto ret = Napi::Object::New(info.Env());
            ret.Set("encryptedData", toJs(info.Env(), ready_to_send));

            return ret;
        });
    };

    static Napi::Value encryptForCommunityInbox(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            // we expect an single argument which is an array of objects with the following
            // properties:
            // {
            //   "plaintext": Uint8Array,
            //   "senderEd25519Privkey": Hexstring,
            //   "sentTimestampMs": Number,
            //   "recipientPubkey": Hexstring,
            //   "communityPubkey": Hexstring,
            //   "proRotatingEd25519Privkey": Hexstring | null,
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
                        extractSenderEd25519PrivkeyAsSpan(
                                obj, "encryptForCommunityInbox.obj.senderEd25519Privkey"),
                        extractSentTimestampMs(obj, "encryptForCommunityInbox.obj.sentTimestampMs"),
                        extractRecipientPubkeyAsArray(
                                obj, "encryptForCommunityInbox.obj.recipientPubkey"),
                        extractCommunityPubkeyAsArray(
                                obj, "encryptForCommunityInbox.obj.communityPubkey"),
                        extractProRotatingEd25519PrivkeyAsSpan(
                                obj, "encryptForCommunityInbox.obj.proRotatingEd25519Privkey"));
            }

            auto ret = Napi::Object::New(info.Env());
            ret.Set("encryptedData", toJs(info.Env(), ready_to_send));

            return ret;
        });
    };

    static Napi::Value encryptForCommunity(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            // we expect an single argument which is an array of objects with the following
            // properties:
            // {
            //   "plaintext": Uint8Array,
            //   "proRotatingEd25519Privkey": Hexstring | null,
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
                        extractProRotatingEd25519PrivkeyAsSpan(
                                obj, "encryptForCommunity.obj.proRotatingEd25519Privkey"));
            }

            auto ret = Napi::Object::New(info.Env());
            ret.Set("encryptedData", toJs(info.Env(), ready_to_send));

            return ret;
        });
    };

    static Napi::Value encryptForGroup(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            // we expect an single argument which is an array of objects with the following
            // properties:
            // {
            //   "plaintext": Uint8Array,
            //   "senderEd25519Privkey": Hexstring,
            //   "sentTimestampMs": Number,
            //   "groupEd25519Pubkey": Hexstring,
            //   "groupEncPrivKey": Hexstring,
            //   "proRotatingEd25519Privkey": Hexstring | null,
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

                ready_to_send[i] = session::encode_for_group(
                        extractPlaintext(obj, "encryptForGroup.obj.plaintext"),
                        extractSenderEd25519PrivkeyAsSpan(
                                obj, "encryptForGroup.obj.senderEd25519Privkey"),
                        extractSentTimestampMs(obj, "encryptForGroup.obj.sentTimestampMs"),
                        extractGroupEd25519PubkeyAsArray(
                                obj, "encryptForGroup.obj.recipientPubkey"),
                        extractGroupEncPrivKeyAsArray(obj, "encryptForGroup.obj.groupEncPrivKey"),
                        extractProRotatingEd25519PrivkeyAsSpan(
                                obj, "encryptForGroup.obj.proRotatingEd25519Privkey"));
            }

            auto ret = Napi::Object::New(info.Env());
            ret.Set("encryptedData", toJs(info.Env(), ready_to_send));

            return ret;
        });
    };
};
};  // namespace session::nodeapi
