#pragma once

#include <napi.h>

#include <span>
#include <vector>

#include "../base_config.hpp"
#include "../groups/meta_group.hpp"

namespace session::nodeapi {

class MetaBaseWrapper {

  public:
    explicit MetaBaseWrapper(){};

    virtual ~MetaBaseWrapper() = default;

    template <typename T, std::enable_if_t<is_derived_napi_wrapper<T>, int> = 0>
    static void NoBaseClassInitHelper(
            Napi::Env env,
            Napi::Object exports,
            const char* class_name,
            std::vector<typename T::PropertyDescriptor> properties) {

        // not adding the baseMethods here from withBaseMethods()
        Napi::Function cls = T::DefineClass(env, class_name, std::move(properties));

        auto ref = std::make_unique<Napi::FunctionReference>();
        *ref = Napi::Persistent(cls);
        env.SetInstanceData(ref.release());

        exports.Set(class_name, cls);
    }

    static std::unique_ptr<session::nodeapi::MetaGroup> constructGroupWrapper(
            const Napi::CallbackInfo& info, const std::string& class_name) {
        return wrapExceptions(info, [&] {
            if (!info.IsConstructCall())
                throw std::invalid_argument{
                        "You need to call the constructor with the `new` syntax"};

            assertInfoLength(info, 1);
            auto arg = info[0];
            assertIsObject(arg);
            auto obj = arg.As<Napi::Object>();

            if (obj.IsEmpty())
                throw std::invalid_argument("constructGroupWrapper received empty");

            assertIsUInt8Array(obj.Get("userEd25519Secretkey"), "constructGroupWrapper userEd");
            auto user_ed25519_secretkey = toCppBuffer(
                    obj.Get("userEd25519Secretkey"),
                    class_name + ":constructGroupWrapper.userEd25519Secretkey");

            assertIsUInt8Array(obj.Get("groupEd25519Pubkey"), "constructGroupWrapper groupEd");
            auto group_ed25519_pubkey = toCppBuffer(
                    obj.Get("groupEd25519Pubkey"),
                    class_name + ":constructGroupWrapper.groupEd25519Pubkey");

            std::optional<std::vector<unsigned char>> group_ed25519_secretkey = maybeNonemptyBuffer(
                    obj.Get("groupEd25519Secretkey"),
                    class_name + ":constructGroupWrapper.groupEd25519Secretkey");

            std::optional<std::vector<unsigned char>> dumped_meta = maybeNonemptyBuffer(
                    obj.Get("metaDumped"), class_name + ":constructGroupWrapper.metaDumped");

            std::optional<std::string> dumped_info;
            std::optional<std::string> dumped_members;
            std::optional<std::string> dumped_keys;

            if (dumped_meta) {
                auto dumped_meta_str = to_string(*dumped_meta);

                oxenc::bt_dict_consumer combined{dumped_meta_str};
                // NB: must read in ascii-sorted order:
                if (!combined.skip_until("info"))
                    throw std::runtime_error{"info dump not found in combined dump!"};
                dumped_info = combined.consume_string();

                if (!combined.skip_until("keys"))
                    throw std::runtime_error{"keys dump not found in combined dump!"};
                dumped_keys = combined.consume_string();

                if (!combined.skip_until("members"))
                    throw std::runtime_error{"members dump not found in combined dump!"};
                dumped_members = combined.consume_string();
            }

            // Note, we keep shared_ptr for those as the Keys one need a reference to Members and
            // Info on its own currently.
            auto info = std::make_shared<config::groups::Info>(
                    group_ed25519_pubkey,
                    group_ed25519_secretkey,
                    (dumped_info ? std::make_optional(session::to_span(*dumped_info))
                                 : std::nullopt));

            auto members = std::make_shared<config::groups::Members>(
                    group_ed25519_pubkey,
                    group_ed25519_secretkey,
                    (dumped_members ? std::make_optional(session::to_span(*dumped_members))
                                    : std::nullopt));

            auto keys = std::make_shared<config::groups::Keys>(
                    user_ed25519_secretkey,
                    group_ed25519_pubkey,
                    group_ed25519_secretkey,
                    (dumped_keys ? std::make_optional(session::to_span(*dumped_keys))
                                 : std::nullopt),
                    *info,
                    *members);

            return std::make_unique<session::nodeapi::MetaGroup>(
                    info, members, keys, group_ed25519_pubkey, group_ed25519_secretkey);
        });
    }
};

}  // namespace session::nodeapi
