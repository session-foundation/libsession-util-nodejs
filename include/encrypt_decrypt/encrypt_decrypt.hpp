#pragma once

#include <napi.h>
#include <oxenc/base64.h>
#include <oxenc/hex.h>

#include "meta/meta_base_wrapper.hpp"

namespace session::nodeapi {

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
                        StaticMethod<&MultiEncryptWrapper::decryptForCommunity>(
                                "decryptForCommunity",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),
                        StaticMethod<&MultiEncryptWrapper::decryptFor1o1>(
                                "decryptFor1o1",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),
                        StaticMethod<&MultiEncryptWrapper::decryptForGroup>(
                                "decryptForGroup",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),
                });
    }

  private:
    static Napi::Value multiEncrypt(const Napi::CallbackInfo& info);
    static Napi::Value multiDecryptEd25519(const Napi::CallbackInfo& info);

    /**
     * ===========================================
     * =========== ATTACHMENTS CALLS =============
     * ===========================================
     */

    static Napi::Value attachmentEncrypt(const Napi::CallbackInfo& info);
    static Napi::Value attachmentDecrypt(const Napi::CallbackInfo& info);

    /**
     * ===========================================
     * ============= ENCRYPT CALLS ===============
     * ===========================================
     */

    static Napi::Value encryptFor1o1(const Napi::CallbackInfo& info);
    static Napi::Value encryptForCommunityInbox(const Napi::CallbackInfo& info);
    static Napi::Value encryptForCommunity(const Napi::CallbackInfo& info);
    static Napi::Value encryptForGroup(const Napi::CallbackInfo& info);
    /**
     * ===========================================
     * ============= DECRYPT CALLS ===============
     * ===========================================
     */

    static Napi::Value decryptForCommunity(const Napi::CallbackInfo& info);
    static Napi::Value decryptFor1o1(const Napi::CallbackInfo& info);
    static Napi::Value decryptForGroup(const Napi::CallbackInfo& info);
};
};  // namespace session::nodeapi
