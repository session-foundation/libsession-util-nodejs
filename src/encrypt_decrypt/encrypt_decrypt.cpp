#include "encrypt_decrypt/encrypt_decrypt.hpp"

#include <napi.h>
#include <oxenc/base64.h>
#include <oxenc/hex.h>

#include <algorithm>
#include <vector>

#include "pro/types.hpp"
#include "session/attachments.hpp"
#include "session/multi_encrypt.hpp"
#include "session/random.hpp"
#include "utilities.hpp"

namespace session::nodeapi {

namespace log = oxen::log;

std::vector<unsigned char> extractPlaintext(const Napi::Object& obj, const std::string identifier) {

    assertIsUInt8Array(obj.Get("plaintext"), identifier);
    auto plaintext = toCppBuffer(obj.Get("plaintext"), identifier);

    return plaintext;
}

std::chrono::milliseconds extractSentTimestampMs(
        const Napi::Object& obj, const std::string identifier) {
    assertIsNumber(obj.Get("sentTimestampMs"), identifier);
    auto sentTimestampMs = toCppMs(obj.Get("sentTimestampMs"), identifier);

    return sentTimestampMs;
}

std::vector<unsigned char> extractSenderEd25519SeedAsVector(
        const Napi::Object& obj, const std::string identifier) {

    assertIsUInt8Array(
            obj.Get("senderEd25519Seed"), "extractSenderEd25519SeedAsVector.senderEd25519Seed");

    auto senderEd25519Seed = toCppBuffer(obj.Get("senderEd25519Seed"), identifier);
    assert_length(senderEd25519Seed, 32, identifier);

    return senderEd25519Seed;
}

session::array_uc33 extractRecipientPubkeyAsArray(
        const Napi::Object& obj, const std::string identifier) {
    assertIsString(obj.Get("recipientPubkey"), identifier);
    auto recipientPubkeyHex = toCppString(obj.Get("recipientPubkey"), identifier);
    assert_length(recipientPubkeyHex, 66, identifier);

    return from_hex_to_array<33>(recipientPubkeyHex);
}

session::array_uc32 extractCommunityPubkeyAsArray(
        const Napi::Object& obj, const std::string identifier) {
    assertIsString(obj.Get("communityPubkey"), identifier);
    auto communityPubkeyHex = toCppString(obj.Get("communityPubkey"), identifier);
    assert_length(communityPubkeyHex, 64, identifier);

    return from_hex_to_array<32>(communityPubkeyHex);
}

session::array_uc33 extractGroupEd25519PubkeyAsArray(
        const Napi::Object& obj, const std::string identifier) {
    assertIsString(obj.Get("groupEd25519Pubkey"), identifier);
    std::string groupEd25519PubkeyHex = toCppString(obj.Get("groupEd25519Pubkey"), identifier);

    assert_length(groupEd25519PubkeyHex, 66, identifier);
    auto arr = from_hex_to_array<33>(groupEd25519PubkeyHex);

    return arr;
}

cleared_uc32 extractGroupEncKeyAsArray(const Napi::Object& obj, const std::string identifier) {
    assertIsString(obj.Get("groupEncKey"), identifier);

    auto groupEncKeyHex = toCppString(obj.Get("groupEncKey"), identifier);
    assert_length(groupEncKeyHex, 64, identifier);

    auto arr = from_hex_to_array<32>(groupEncKeyHex);
    cleared_uc32 result;

    std::copy(arr.begin(), arr.end(), result.begin());

    return result;
}

std::optional<std::vector<unsigned char>> extractProRotatingEd25519PrivKeyAsVector(
        const Napi::Object& obj, const std::string identifier) {
    assertIsStringOrNull(obj.Get("proRotatingEd25519PrivKey"), identifier);
    auto proRotatingEd25519PrivKeyHex =
            maybeNonemptyString(obj.Get("proRotatingEd25519PrivKey"), identifier);

    if (proRotatingEd25519PrivKeyHex.has_value() && proRotatingEd25519PrivKeyHex.value().size()) {

        auto ret = from_hex_to_vector(*proRotatingEd25519PrivKeyHex);

        assert_length(ret, 64, identifier);
        return ret;
    }

    return std::nullopt;
}

std::vector<unsigned char> extractContentOrEnvelope(
        const Napi::Object& obj, const std::string identifier) {
    assertIsUInt8Array(obj.Get("contentOrEnvelope"), identifier);
    std::vector<unsigned char> contentOrEnvelope =
            toCppBuffer(obj.Get("contentOrEnvelope"), identifier);

    return contentOrEnvelope;
}

uint32_t extractServerId(const Napi::Object& obj, const std::string identifier) {
    assertIsNumber(obj.Get("serverId"), identifier);
    auto serverId = toCppInteger(obj.Get("serverId"), identifier);

    return serverId;
}

std::string extractMessageHash(const Napi::Object& obj, const std::string identifier) {
    assertIsString(obj.Get("messageHash"), identifier);
    auto messageHash = toCppString(obj.Get("messageHash"), identifier);

    return messageHash;
}

std::vector<unsigned char> extractEnvelopePayload(
        const Napi::Object& obj, const std::string identifier) {
    assertIsUInt8Array(obj.Get("envelopePayload"), identifier);
    auto envelopePayload = toCppBuffer(obj.Get("envelopePayload"), identifier);

    return envelopePayload;
}

std::chrono::sys_time<std::chrono::milliseconds> extractNowSysMs(
        const Napi::Object& obj, const std::string identifier) {
    assertIsNumber(obj.Get("nowMs"), identifier);
    auto nowMs = toCppSysMs(obj.Get("nowMs"), identifier);

    return nowMs;
}

session::array_uc32 extractProBackendPubkeyHex(
        const Napi::Object& obj, const std::string identifier) {
    assertIsString(obj.Get("proBackendPubkeyHex"), identifier);

    auto proBackendPubkeyHex = toCppString(obj.Get("proBackendPubkeyHex"), identifier);
    assert_length(proBackendPubkeyHex, 64, identifier);

    auto arr = from_hex_to_array<32>(proBackendPubkeyHex);

    return arr;
}

session::array_uc32 extractEd25519PrivateKeyHex(
        const Napi::Object& obj, const std::string identifier) {
    assertIsString(obj.Get("ed25519PrivateKeyHex"), identifier);

    auto ed25519PrivateKeyHex = toCppString(obj.Get("ed25519PrivateKeyHex"), identifier);
    assert_length(ed25519PrivateKeyHex, 64, identifier);

    auto arr = from_hex_to_array<32>(ed25519PrivateKeyHex);

    return arr;
}

std::vector<unsigned char> extractEd25519GroupPubkeyHex(
        const Napi::Object& obj, const std::string& identifier) {
    assertIsString(obj.Get("ed25519GroupPubkeyHex"), identifier);
    auto ed25519GroupPubkeyHex = toCppString(obj.Get("ed25519GroupPubkeyHex"), identifier);
    assert_length(ed25519GroupPubkeyHex, 66, identifier);

    auto arr = from_hex_to_vector(ed25519GroupPubkeyHex);

    return arr;
}

std::vector<std::vector<unsigned char>> extractGroupEncKeys(
        const Napi::Object& obj, const std::string& identifier) {
    assertIsArray(obj.Get("groupEncKeys"), identifier);

    auto asArray = obj.Get("groupEncKeys").As<Napi::Array>();
    std::vector<std::vector<unsigned char>> groupEncKeys;
    groupEncKeys.reserve(asArray.Length());

    for (uint32_t i = 0; i < asArray.Length(); i++) {
        auto itemValue = asArray.Get(i);
        assertIsUInt8Array(itemValue, "extractGroupEncKeys");

        auto encKey = itemValue.As<Napi::Uint8Array>();

        std::vector<unsigned char> cppEncKey =
                toCppBuffer(encKey, "extractGroupEncKeys.groupEncKey");
        assert_length(cppEncKey, 32, "extractGroupEncKeys.groupEncKey");

        groupEncKeys.emplace_back(cppEncKey);
    }

    return groupEncKeys;
}

Napi::Value MultiEncryptWrapper::multiEncrypt(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        assertIsObject(info[0]);
        auto obj = info[0].As<Napi::Object>();

        if (obj.IsEmpty())
            throw std::invalid_argument("multiEncrypt received empty");

        assertIsUInt8Array(obj.Get("ed25519SecretKey"), "multiEncrypt.ed25519SecretKey");
        auto ed25519SecretKey =
                toCppBuffer(obj.Get("ed25519SecretKey"), "multiEncrypt.ed25519SecretKey");

        assertIsString(obj.Get("domain"), "multiEncrypt.domain");
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

        std::vector<std::span<const unsigned char>> messages_sv(messages.begin(), messages.end());
        std::vector<std::span<const unsigned char>> recipients_sv(
                recipients.begin(), recipients.end());

        // Note: this function needs the first 2 args to be vector of sv explicitly
        return session::encrypt_for_multiple_simple(
                messages_sv, recipients_sv, ed25519SecretKey, domain, random_nonce);
    });
};

Napi::Value MultiEncryptWrapper::multiDecryptEd25519(const Napi::CallbackInfo& info) {
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

        assertIsString(obj.Get("domain"), "multiDecryptEd25519.domain");
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

Napi::Value MultiEncryptWrapper::attachmentEncrypt(const Napi::CallbackInfo& info) {
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

        assertIsString(obj.Get("domain"), "attachmentEncrypt.domain");
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

Napi::Value MultiEncryptWrapper::attachmentDecrypt(const Napi::CallbackInfo& info) {
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
                reinterpret_cast<const std::byte*>(encrypted_data.data() + encrypted_data.size()));

        std::vector<std::byte> decryption_key_bytes(
                reinterpret_cast<const std::byte*>(decryption_key.data()),
                reinterpret_cast<const std::byte*>(decryption_key.data() + decryption_key.size()));

        if (decryption_key_bytes.size() != session::attachment::ENCRYPT_KEY_SIZE) {
            throw std::invalid_argument("Key size mismatch");
        }

        std::span<const std::byte, session::attachment::ENCRYPT_KEY_SIZE> decryption_key_span(
                decryption_key_bytes.data(), session::attachment::ENCRYPT_KEY_SIZE);

        auto decrypted = session::attachment::decrypt(encrypted_data_bytes, decryption_key_span);

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

Napi::Value MultiEncryptWrapper::encryptFor1o1(const Napi::CallbackInfo& info) {
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
                    extractSenderEd25519SeedAsVector(obj, "encryptFor1o1.obj.senderEd25519Seed"),
                    extractSentTimestampMs(obj, "encryptFor1o1.obj.sentTimestampMs"),
                    extractRecipientPubkeyAsArray(obj, "encryptFor1o1.obj.recipientPubkey"),
                    extractProRotatingEd25519PrivKeyAsVector(
                            obj, "encryptFor1o1.obj.proRotatingEd25519PrivKey"));
        }

        auto ret = Napi::Object::New(info.Env());
        ret.Set("encryptedData", toJs(info.Env(), ready_to_send));

        return ret;
    });
};

Napi::Value MultiEncryptWrapper::encryptForCommunityInbox(const Napi::CallbackInfo& info) {
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
                throw std::invalid_argument("encryptForCommunityInbox itemValue is not an object");
            }
            auto obj = itemValue.As<Napi::Object>();

            ready_to_send[i] = session::encode_for_community_inbox(
                    extractPlaintext(obj, "encryptForCommunityInbox.obj.plaintext"),
                    extractSenderEd25519SeedAsVector(
                            obj, "encryptForCommunityInbox.obj.senderEd25519Seed"),
                    extractSentTimestampMs(obj, "encryptForCommunityInbox.obj.sentTimestampMs"),
                    extractRecipientPubkeyAsArray(
                            obj, "encryptForCommunityInbox.obj.recipientPubkey"),
                    extractCommunityPubkeyAsArray(
                            obj, "encryptForCommunityInbox.obj.communityPubkey"),
                    extractProRotatingEd25519PrivKeyAsVector(
                            obj, "encryptForCommunityInbox.obj.proRotatingEd25519PrivKey"));
        }

        auto ret = Napi::Object::New(info.Env());
        ret.Set("encryptedData", toJs(info.Env(), ready_to_send));

        return ret;
    });
};

Napi::Value MultiEncryptWrapper::encryptForCommunity(const Napi::CallbackInfo& info) {
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
                    extractProRotatingEd25519PrivKeyAsVector(
                            obj, "encryptForCommunity.obj.proRotatingEd25519PrivKey"));
        }

        auto ret = Napi::Object::New(info.Env());
        ret.Set("encryptedData", toJs(info.Env(), ready_to_send));

        return ret;
    });
};

Napi::Value MultiEncryptWrapper::encryptForGroup(const Napi::CallbackInfo& info) {
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
            auto senderEd25519Seed =
                    extractSenderEd25519SeedAsVector(obj, "encryptForGroup.obj.senderEd25519Seed");

            auto sentTimestampMs =
                    extractSentTimestampMs(obj, "encryptForGroup.obj.sentTimestampMs");

            auto groupEd25519Pubkey =
                    extractGroupEd25519PubkeyAsArray(obj, "encryptForGroup.obj.recipientPubkey");

            auto groupEncKey = extractGroupEncKeyAsArray(obj, "encryptForGroup.obj.groupEncKey");
            auto proRotatingEd25519PrivKey = extractProRotatingEd25519PrivKeyAsVector(
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

Napi::Value MultiEncryptWrapper::decryptForCommunity(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        // we expect two arguments that match:
        // first: [{
        //   "contentOrEnvelope": Uint8Array,
        //   "serverId": number,
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

        std::vector<DecodedCommunityMessage> decrypted;
        std::vector<uint32_t> decryptedServerIds;

        for (uint32_t i = 0; i < first.Length(); i++) {
            auto itemValue = first.Get(i);
            if (!itemValue.IsObject()) {
                throw std::invalid_argument(
                        "decryptForCommunity itemValue is not an "
                        "object");
            }
            auto obj = itemValue.As<Napi::Object>();

            try {
                uint32_t serverId = extractServerId(obj, "decryptForCommunity.obj.serverId");

                auto contentOrEnvelope =
                        extractContentOrEnvelope(obj, "decryptForCommunity.obj.contentOrEnvelope");
                decrypted.push_back(
                        session::decode_for_community(
                                contentOrEnvelope, nowMs, proBackendPubkeyHex));
                decryptedServerIds.push_back(serverId);

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

            to_insert.Set(
                    "envelope", d.envelope ? toJs(info.Env(), *d.envelope) : info.Env().Null());
            to_insert.Set("contentPlaintextUnpadded", toJs(info.Env(), d.content_plaintext));
            to_insert.Set("serverId", toJs(info.Env(), decryptedServerIds[i]));

            to_insert.Set(
                    "proSigHex",
                    d.pro_sig ? toJs(info.Env(), oxenc::to_hex(*d.pro_sig)) : info.Env().Null());
            to_insert.Set("decodedPro", d.pro ? toJs(info.Env(), d.pro) : info.Env().Null());

            ret.Set(i, to_insert);
            i++;
        }

        return ret;
    });
};

Napi::Value MultiEncryptWrapper::decryptFor1o1(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        // we expect two arguments that match:
        // first: [{
        //   "envelopePayload": Uint8Array,
        //   "messageHash": string,
        // }],
        // second: {
        //   "nowMs": number,
        //   "proBackendPubkeyHex": Hexstring,
        //   "ed25519PrivateKeyHex": Hexstring,
        //  }
        //

        assertInfoLength(info, 2);
        assertIsArray(info[0], "decryptFor1o1 info[0]");
        assertIsObject(info[1]);

        auto first = info[0].As<Napi::Array>();

        if (first.IsEmpty())
            throw std::invalid_argument("decryptFor1o1 first received empty");

        auto second = info[1].As<Napi::Object>();

        if (second.IsEmpty())
            throw std::invalid_argument("decryptFor1o1 second received empty");

        auto nowMs = extractNowSysMs(second, "decryptFor1o1.second.nowMs");
        auto proBackendPubkeyHex =
                extractProBackendPubkeyHex(second, "decryptFor1o1.second.proBackendPubkeyHex");

        std::vector<DecodedEnvelope> decrypted;
        std::vector<std::string> decryptedMessageHashes;

        DecodeEnvelopeKey keys{};
        auto keySpan =
                extractEd25519PrivateKeyHex(second, "decryptFor1o1.second.ed25519PrivateKeyHex");

        std::vector<std::span<const unsigned char>> keySpans;
        keySpans.emplace_back(keySpan.data(), keySpan.size());
        keys.decrypt_keys = keySpans;

        for (uint32_t i = 0; i < first.Length(); i++) {
            auto itemValue = first.Get(i);
            if (!itemValue.IsObject()) {
                throw std::invalid_argument(
                        "decryptFor1o1 itemValue is not an "
                        "object");
            }
            auto obj = itemValue.As<Napi::Object>();

            try {
                std::string messageHash = extractMessageHash(obj, "decryptFor1o1.obj.messageHash");

                auto envelopePayload =
                        extractEnvelopePayload(obj, "decryptFor1o1.obj.envelopePayload");
                decrypted.push_back(
                        session::decode_envelope(
                                keys, envelopePayload, nowMs, proBackendPubkeyHex));
                decryptedMessageHashes.push_back(messageHash);
            } catch (const std::exception& e) {
                log::warning(
                        cat,
                        "decryptFor1o1: Failed to decrypt "
                        "message at index {}",
                        i);
            }
        }

        auto ret = Napi::Array::New(info.Env(), decrypted.size());
        uint32_t i = 0;

        for (auto& d : decrypted) {
            auto to_insert = Napi::Object::New(info.Env());

            to_insert.Set("decodedEnvelope", toJs(info.Env(), d));
            to_insert.Set("messageHash", toJs(info.Env(), decryptedMessageHashes[i]));

            ret.Set(i, to_insert);
            i++;
        }

        return ret;
    });
};

Napi::Value MultiEncryptWrapper::decryptForGroup(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        // we expect two arguments that match:
        // first: [{
        //   "envelopePayload": Uint8Array,
        //   "messageHash": string,
        // }],
        // second: {
        //   "nowMs": number,
        //   "proBackendPubkeyHex": Hexstring,
        //   "ed25519GroupPubkeyHex": Hexstring,
        //   "groupEncKeys": Array<Uint8Array>,
        //  }
        //

        assertInfoLength(info, 2);
        assertIsArray(info[0], "decryptForGroup info[0]");
        assertIsObject(info[1]);

        auto first = info[0].As<Napi::Array>();

        if (first.IsEmpty())
            throw std::invalid_argument("decryptForGroup first received empty");

        auto second = info[1].As<Napi::Object>();

        if (second.IsEmpty())
            throw std::invalid_argument("decryptForGroup second received empty");

        auto nowMs = extractNowSysMs(second, "decryptForGroup.second.nowMs");
        auto proBackendPubkeyHex =
                extractProBackendPubkeyHex(second, "decryptForGroup.second.proBackendPubkeyHex");

        std::vector<DecodedEnvelope> decrypted;
        std::vector<std::string> decryptedMessageHashes;

        DecodeEnvelopeKey keys{};
        auto groupPk = extractEd25519GroupPubkeyHex(
                second, "decryptForGroup.second.ed25519GroupPubkeyHex");

        // this has to be vector and not spans, the memory gets freed by the function
        std::vector<std::vector<unsigned char>> groupEncKeysVec =
                extractGroupEncKeys(second, "decryptForGroup.second.groupEncKeys");

        std::vector<std::span<const unsigned char>> span_group_enc_keys;
        span_group_enc_keys.reserve(span_group_enc_keys.size());
        for (const auto& inner : groupEncKeysVec) {
            span_group_enc_keys.emplace_back(inner);
        }

        // Create a span of spans
        std::span<std::span<const unsigned char>> groupEncKeys(span_group_enc_keys);

        keys.decrypt_keys = groupEncKeys;

        for (uint32_t i = 0; i < first.Length(); i++) {
            auto itemValue = first.Get(i);
            if (!itemValue.IsObject()) {
                throw std::invalid_argument(
                        "decryptForGroup itemValue is not an "
                        "object");
            }
            auto obj = itemValue.As<Napi::Object>();

            try {
                std::string messageHash =
                        extractMessageHash(obj, "decryptForGroup.obj.messageHash");

                auto envelopePayload =
                        extractEnvelopePayload(obj, "decryptForGroup.obj.envelopePayload");
                decrypted.push_back(
                        session::decode_envelope(
                                keys, envelopePayload, nowMs, proBackendPubkeyHex));
                decryptedMessageHashes.push_back(messageHash);
            } catch (const std::exception& e) {
                log::warning(
                        cat,
                        "decryptForGroup: Failed to decrypt "
                        "message at index {}",
                        i);
            }
        }

        auto ret = Napi::Array::New(info.Env(), decrypted.size());
        uint32_t i = 0;

        for (auto& d : decrypted) {
            auto to_insert = Napi::Object::New(info.Env());

            to_insert.Set("decodedEnvelope", toJs(info.Env(), d));
            to_insert.Set("messageHash", toJs(info.Env(), decryptedMessageHashes[i]));

            ret.Set(i, to_insert);
            i++;
        }

        return ret;
    });
};

};  // namespace session::nodeapi
