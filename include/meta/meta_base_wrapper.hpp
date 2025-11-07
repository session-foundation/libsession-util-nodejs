#pragma once

#include <napi.h>

#include <memory>
#include <vector>

#include "../base_config.hpp"
#include "groups/meta_group.hpp"

namespace session::nodeapi {

class MetaBaseWrapper {

  public:
    explicit MetaBaseWrapper() {};

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
            const Napi::CallbackInfo& info, const std::string& class_name);
};

}  // namespace session::nodeapi
