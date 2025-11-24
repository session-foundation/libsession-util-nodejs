#include "convo_info_volatile_config.hpp"

#include <oxenc/base64.h>

#include <optional>

#include "base_config.hpp"
#include "community.hpp"
#include "session/config/convo_info_volatile.hpp"
#include "utilities.hpp"

namespace session::nodeapi {

namespace convo = config::convo;

using config::ConvoInfoVolatile;

struct ParsedBaseValues {
    int64_t lastReadTsMs;
    bool forcedUnread;

    Napi::Object obj;
};

void addBaseValues(const Napi::Env& env, Napi::Object obj, const convo::base& base) {
    obj["lastReadTsMs"] = toJs(env, base.last_read);
    obj["forcedUnread"] = toJs(env, base.unread);
}

ParsedBaseValues parseBaseValues(
        const Napi::CallbackInfo& info, convo::base& base, const std::string fnName) {
    assertInfoLength(info, 2);
    auto baseObj = info[1];
    assertIsObject(baseObj);

    auto obj = baseObj.As<Napi::Object>();
    auto lastReadTsMsJs = obj.Get("lastReadTsMs");
    assertIsNumber(lastReadTsMsJs, fnName + "lastReadTsMs");
    auto lastReadTsMsCpp = toCppInteger(lastReadTsMsJs, fnName + "lastReadTsMs");

    auto forcedUnreadJs = obj.Get("forcedUnread");
    assertIsBoolean(forcedUnreadJs, fnName + "forcedUnread");
    auto lastReadTsMs = toCppBoolean(forcedUnreadJs, fnName + "forcedUnread");

    ParsedBaseValues result;
    result.lastReadTsMs = lastReadTsMsCpp;
    result.forcedUnread = lastReadTsMs;
    result.obj = obj;
    return result;
}

template <>
struct toJs_impl<convo::one_to_one> {
    Napi::Object operator()(const Napi::Env& env, const convo::one_to_one& info_1o1) {

        auto obj = Napi::Object::New(env);

        obj["pubkeyHex"] = toJs(env, info_1o1.session_id);
        addBaseValues(env, obj, info_1o1);

        if (info_1o1.pro_gen_index_hash->empty() ||
            !info_1o1.pro_expiry_unix_ts.time_since_epoch().count()) {
            obj["proGenIndexHashB64"] = env.Null();
            obj["proExpiryTsMs"] = env.Null();
        } else {
            obj["proGenIndexHashB64"] = toJs(env, to_base64(*info_1o1.pro_gen_index_hash));
            obj["proExpiryTsMs"] = toJs(env, info_1o1.pro_expiry_unix_ts);
        }

        return obj;
    }
};

template <>
struct toJs_impl<convo::legacy_group> {
    Napi::Object operator()(const Napi::Env& env, const convo::legacy_group info_legacy) {
        auto obj = Napi::Object::New(env);

        obj["pubkeyHex"] = toJs(env, info_legacy.id);
        addBaseValues(env, obj, info_legacy);

        return obj;
    }
};

template <>
struct toJs_impl<convo::community> : toJs_impl<config::community> {
    Napi::Object operator()(const Napi::Env& env, const convo::community info_comm) {
        auto obj = toJs_impl<config::community>::operator()(env, info_comm);
        addBaseValues(env, obj, info_comm);

        return obj;
    }
};

template <>
struct toJs_impl<convo::group> {
    Napi::Object operator()(const Napi::Env& env, const convo::group group_info) {
        auto obj = Napi::Object::New(env);

        obj["pubkeyHex"] = toJs(env, group_info.id);
        addBaseValues(env, obj, group_info);

        return obj;
    }
};

void ConvoInfoVolatileWrapper::Init(Napi::Env env, Napi::Object exports) {
    InitHelper<ConvoInfoVolatileWrapper>(
            env,
            exports,
            "ConvoInfoVolatileWrapperNode",
            {
                    // 1o1 related methods
                    InstanceMethod("get1o1", &ConvoInfoVolatileWrapper::get1o1),
                    InstanceMethod("getAll1o1", &ConvoInfoVolatileWrapper::getAll1o1),
                    InstanceMethod("set1o1", &ConvoInfoVolatileWrapper::set1o1),
                    InstanceMethod("erase1o1", &ConvoInfoVolatileWrapper::erase1o1),

                    // legacy group related methods
                    InstanceMethod("getLegacyGroup", &ConvoInfoVolatileWrapper::getLegacyGroup),
                    InstanceMethod(
                            "getAllLegacyGroups", &ConvoInfoVolatileWrapper::getAllLegacyGroups),
                    InstanceMethod("setLegacyGroup", &ConvoInfoVolatileWrapper::setLegacyGroup),
                    InstanceMethod("eraseLegacyGroup", &ConvoInfoVolatileWrapper::eraseLegacyGroup),

                    // group related methods
                    InstanceMethod("getGroup", &ConvoInfoVolatileWrapper::getGroup),
                    InstanceMethod("getAllGroups", &ConvoInfoVolatileWrapper::getAllGroups),
                    InstanceMethod("setGroup", &ConvoInfoVolatileWrapper::setGroup),
                    InstanceMethod("eraseGroup", &ConvoInfoVolatileWrapper::eraseGroup),

                    // communities related methods
                    InstanceMethod("getCommunity", &ConvoInfoVolatileWrapper::getCommunity),
                    InstanceMethod(
                            "getAllCommunities", &ConvoInfoVolatileWrapper::getAllCommunities),
                    InstanceMethod(
                            "setCommunityByFullUrl",
                            &ConvoInfoVolatileWrapper::setCommunityByFullUrl),
                    InstanceMethod(
                            "eraseCommunityByFullUrl",
                            &ConvoInfoVolatileWrapper::eraseCommunityByFullUrl),
            });
}

ConvoInfoVolatileWrapper::ConvoInfoVolatileWrapper(const Napi::CallbackInfo& info) :
        ConfigBaseImpl{construct<ConvoInfoVolatile>(info, "ConvoInfoVolatile")},
        Napi::ObjectWrap<ConvoInfoVolatileWrapper>{info} {}

/**
 * =================================================
 * ====================== 1o1 ======================
 * =================================================
 */

Napi::Value ConvoInfoVolatileWrapper::get1o1(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return config.get_1to1(getStringArgs<1>(info)); });
}

Napi::Value ConvoInfoVolatileWrapper::getAll1o1(const Napi::CallbackInfo& info) {
    return get_all_impl(info, config.size_1to1(), config.begin_1to1(), config.end());
}

void ConvoInfoVolatileWrapper::set1o1(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 2);
        auto first = info[0];
        assertIsString(first);

        std::string fnName = "ConvoInfoVolatileWrapper::set1o1.";
        auto convo = config.get_or_construct_1to1(toCppString(first, fnName + "convoInfo"));

        auto parsed = parseBaseValues(info, convo, fnName);
        if (parsed.lastReadTsMs > convo.last_read)
            convo.last_read = parsed.lastReadTsMs;
        convo.unread = parsed.forcedUnread;

        // 1o1 also have a pro gen index hash & pro expiry
        auto proGenIndexHashB64Js = parsed.obj.Get("proGenIndexHashB64");
        assertIsStringOrNull(proGenIndexHashB64Js, fnName + "proGenIndexHashB64Js");
        auto proGenIndexHashB64Cpp =
                maybeNonemptyString(proGenIndexHashB64Js, fnName + "proGenIndexHashB64Cpp");

        auto proExpiryUnixTsMsJs = parsed.obj.Get("proExpiryTsMs");
        assertIsNumberOrNull(proExpiryUnixTsMsJs, fnName + "proExpiryUnixTsMsJs");
        auto proExpiryUnixTsMsCpp =
                maybeNonemptyInt(proExpiryUnixTsMsJs, fnName + "proExpiryUnixTsMsCpp");
        // Note: null is used to ignore an update. i.e. if the field is unset, we do not want to
        // overwrite the current value.
        // To reset it, set it to empty string
        if (proGenIndexHashB64Cpp.has_value()) {
            if (proGenIndexHashB64Cpp->empty()) {
                // if the first is set, but empty, we want to reset the field
                convo.pro_gen_index_hash = std::nullopt;
            } else {
                // this throws if the size is wrong
                convo.pro_gen_index_hash = from_base64_to_array<32>(*proGenIndexHashB64Cpp);
            }
        }
        if (proExpiryUnixTsMsCpp.has_value()) {
            // if the field is set (not null), we want to write the change as is
            convo.pro_expiry_unix_ts = std::chrono::sys_time<std::chrono::milliseconds>(
                    std::chrono::milliseconds(*proExpiryUnixTsMsCpp));
        }

        config.set(convo);
    });
}

Napi::Value ConvoInfoVolatileWrapper::erase1o1(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return config.erase_1to1(getStringArgs<1>(info)); });
}

/**
 * =================================================
 * ================= Legacy groups =================
 * =================================================
 */

Napi::Value ConvoInfoVolatileWrapper::getLegacyGroup(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return config.get_legacy_group(getStringArgs<1>(info)); });
}

Napi::Value ConvoInfoVolatileWrapper::getAllLegacyGroups(const Napi::CallbackInfo& info) {
    return get_all_impl(
            info, config.size_legacy_groups(), config.begin_legacy_groups(), config.end());
}

void ConvoInfoVolatileWrapper::setLegacyGroup(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 2);
        auto first = info[0];
        assertIsString(first);

        std::string fnName = "ConvoInfoVolatileWrapper::setLegacyGroup.";
        auto convo = config.get_or_construct_legacy_group(toCppString(first, fnName + "convoInfo"));

        auto parsed = parseBaseValues(info, convo, fnName);
        if (parsed.lastReadTsMs > convo.last_read)
            convo.last_read = parsed.lastReadTsMs;
        convo.unread = parsed.forcedUnread;

        config.set(convo);
    });
}

Napi::Value ConvoInfoVolatileWrapper::eraseLegacyGroup(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return config.erase_legacy_group(getStringArgs<1>(info)); });
}

/**
 * =================================================
 * ===================== Groups ====================
 * =================================================
 */

Napi::Value ConvoInfoVolatileWrapper::getGroup(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return config.get_group(getStringArgs<1>(info)); });
}

Napi::Value ConvoInfoVolatileWrapper::getAllGroups(const Napi::CallbackInfo& info) {
    return get_all_impl(info, config.size_groups(), config.begin_groups(), config.end());
}

void ConvoInfoVolatileWrapper::setGroup(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 2);
        auto first = info[0];
        assertIsString(first);

        std::string fnName = "ConvoInfoVolatileWrapper::setGroup.";
        auto convo = config.get_or_construct_group(toCppString(first, fnName + "convoInfo"));

        auto parsed = parseBaseValues(info, convo, fnName);
        if (parsed.lastReadTsMs > convo.last_read)
            convo.last_read = parsed.lastReadTsMs;
        convo.unread = parsed.forcedUnread;

        config.set(convo);
    });
}

Napi::Value ConvoInfoVolatileWrapper::eraseGroup(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return config.erase_group(getStringArgs<1>(info)); });
}

/**
 * =================================================
 * ================== Communities ==================
 * =================================================
 */

Napi::Value ConvoInfoVolatileWrapper::getCommunity(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return config.get_community(getStringArgs<1>(info)); });
}

Napi::Value ConvoInfoVolatileWrapper::getAllCommunities(const Napi::CallbackInfo& info) {
    return get_all_impl(info, config.size_communities(), config.begin_communities(), config.end());
}

// TODO maybe make the setXXX   return the update value so we avoid having to
// fetch again updated values from the renderer

void ConvoInfoVolatileWrapper::setCommunityByFullUrl(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 2);
        auto first = info[0];
        assertIsString(first);

        std::string fnName = "ConvoInfoVolatileWrapper::setCommunityByFullUrl.";

        auto convo = config.get_or_construct_community(toCppString(first, fnName + "convoInfo"));

        auto parsed = parseBaseValues(info, convo, fnName);
        if (parsed.lastReadTsMs > convo.last_read)
            convo.last_read = parsed.lastReadTsMs;
        convo.unread = parsed.forcedUnread;

        // Note: we only keep the messages read when their timestamp is not older
        // than 30 days or so (see libsession util PRUNE constant). so this `set()`
        // here might actually not create an entry
        config.set(convo);
    });
}

Napi::Value ConvoInfoVolatileWrapper::eraseCommunityByFullUrl(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        auto [base, room, pubkey] = config::community::parse_full_url(getStringArgs<1>(info));
        return config.erase_community(base, room);
    });
}

}  // namespace session::nodeapi
