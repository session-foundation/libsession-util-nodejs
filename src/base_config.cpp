#include "base_config.hpp"

#include "session/config/base.hpp"
#include "session/config/encrypt.hpp"

namespace session::nodeapi {

using config::ConfigBase;

Napi::Value ConfigBaseImpl::needsDump(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return get_config<ConfigBase>().needs_dump(); });
}

Napi::Value ConfigBaseImpl::needsPush(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return get_config<ConfigBase>().needs_push(); });
}

Napi::Value ConfigBaseImpl::activeHashes(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        std::unordered_set<std::string> hashes = get_config<ConfigBase>().active_hashes();
        std::vector<std::string> hashesVec(hashes.begin(), hashes.end());
        return hashesVec;
    });
}

Napi::Value ConfigBaseImpl::push(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&]() {
        assertInfoLength(info, 0);
        auto& conf = get_config<ConfigBase>();
        auto to_push = conf.push();

        return push_result_to_JS(info.Env(), to_push, conf.storage_namespace());
    });
}

Napi::Value ConfigBaseImpl::dump(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&]() {
        assertInfoLength(info, 0);
        return get_config<ConfigBase>().dump();
    });
}

Napi::Value ConfigBaseImpl::makeDump(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&]() {
        assertInfoLength(info, 0);
        return get_config<ConfigBase>().make_dump();
    });
}

void ConfigBaseImpl::confirmPushed(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&]() {
        assertInfoLength(info, 1);
        assertIsObject(info[0]);
        auto obj = info[0].As<Napi::Object>();

        auto confirmed_pushed_entry = confirm_pushed_entry_from_JS(info.Env(), obj);

        get_config<ConfigBase>().confirm_pushed(
                std::get<0>(confirmed_pushed_entry), std::get<1>(confirmed_pushed_entry));
    });
}

Napi::Value ConfigBaseImpl::merge(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&]() {
        assertInfoLength(info, 1);
        assertIsArray(info[0], "ConfigBaseImpl::merge");
        Napi::Array asArray = info[0].As<Napi::Array>();

        std::vector<std::pair<std::string, std::vector<unsigned char>>> conf_strs;
        conf_strs.reserve(asArray.Length());

        for (uint32_t i = 0; i < asArray.Length(); i++) {
            Napi::Value item = asArray[i];
            assertIsObject(item);
            if (item.IsEmpty())
                throw std::invalid_argument("Merge.item received empty");

            Napi::Object itemObject = item.As<Napi::Object>();
            conf_strs.emplace_back(
                    toCppString(itemObject.Get("hash"), "base.merge"),
                    toCppBuffer(itemObject.Get("data"), "base.merge"));
        }
        std::unordered_set<std::string> merged = get_config<ConfigBase>().merge(conf_strs);
        std::vector<std::string> mergedVec(merged.begin(), merged.end());
        return mergedVec;
    });
}

}  // namespace session::nodeapi
