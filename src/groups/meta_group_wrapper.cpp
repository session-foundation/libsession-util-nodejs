#include "groups/meta_group_wrapper.hpp"

#include <napi.h>
#include <oxenc/bt_producer.h>

#include <memory>
#include <session/types.hpp>
#include <session/util.hpp>
#include <span>
#include <vector>

namespace session::nodeapi {

Napi::Object member_to_js(const Napi::Env& env, const member& info, const member::Status& status) {
    auto obj = Napi::Object::New(env);

    obj["pubkeyHex"] = toJs(env, info.session_id);
    obj["name"] = toJs(env, info.name);
    obj["profilePicture"] = toJs(env, info.profile_picture);
    obj["profileUpdatedSeconds"] = toJs(env, info.profile_updated);
    obj["supplement"] = toJs(env, info.supplement);

    switch (status) {
        // invite statuses
        case member::Status::invite_unknown:
            obj["memberStatus"] = toJs(env, "INVITE_UNKNOWN");
            break;
        case member::Status::invite_not_sent:
            obj["memberStatus"] = toJs(env, "INVITE_NOT_SENT");
            break;
        case member::Status::invite_sending:
            obj["memberStatus"] = toJs(env, "INVITE_SENDING");
            break;
        case member::Status::invite_failed: obj["memberStatus"] = toJs(env, "INVITE_FAILED"); break;
        case member::Status::invite_sent: obj["memberStatus"] = toJs(env, "INVITE_SENT"); break;
        case member::Status::invite_accepted:
            obj["memberStatus"] = toJs(env, "INVITE_ACCEPTED");
            break;

        // promotion statuses
        case member::Status::promotion_unknown:
            obj["memberStatus"] = toJs(env, "PROMOTION_UNKNOWN");
            break;
        case member::Status::promotion_not_sent:
            obj["memberStatus"] = toJs(env, "PROMOTION_NOT_SENT");
            break;
        case member::Status::promotion_sending:
            obj["memberStatus"] = toJs(env, "PROMOTION_SENDING");
            break;
        case member::Status::promotion_failed:
            obj["memberStatus"] = toJs(env, "PROMOTION_FAILED");
            break;
        case member::Status::promotion_sent:
            obj["memberStatus"] = toJs(env, "PROMOTION_SENT");
            break;
        case member::Status::promotion_accepted:
            obj["memberStatus"] = toJs(env, "PROMOTION_ACCEPTED");
            break;

        // removed statuses
        case member::Status::removed_unknown:
            obj["memberStatus"] = toJs(env, "REMOVED_UNKNOWN");
            break;
        case member::Status::removed: obj["memberStatus"] = toJs(env, "REMOVED_MEMBER"); break;
        case member::Status::removed_including_messages:
            obj["memberStatus"] = toJs(env, "REMOVED_MEMBER_AND_MESSAGES");
            break;

        default: throw std::runtime_error{"Invalid member status got as an enum"};
    }

    // we display the "crown" on top of the member's avatar when this field is true
    obj["nominatedAdmin"] = toJs(env, info.admin);

    return obj;
};

MetaGroupWrapper::MetaGroupWrapper(const Napi::CallbackInfo& info) :
        meta_group{std::move(MetaBaseWrapper::constructGroupWrapper(info, "MetaGroupWrapper"))},
        Napi::ObjectWrap<MetaGroupWrapper>{info} {}

void MetaGroupWrapper::Init(Napi::Env env, Napi::Object exports) {
    MetaBaseWrapper::NoBaseClassInitHelper<MetaGroupWrapper>(
            env,
            exports,
            "MetaGroupWrapperNode",
            {
                    // shared exposed functions
                    InstanceMethod("needsPush", &MetaGroupWrapper::needsPush),
                    InstanceMethod("push", &MetaGroupWrapper::push),
                    InstanceMethod("needsDump", &MetaGroupWrapper::needsDump),
                    InstanceMethod("metaDump", &MetaGroupWrapper::metaDump),
                    InstanceMethod("metaMakeDump", &MetaGroupWrapper::metaMakeDump),
                    InstanceMethod("metaConfirmPushed", &MetaGroupWrapper::metaConfirmPushed),
                    InstanceMethod("metaMerge", &MetaGroupWrapper::metaMerge),

                    // infos exposed functions
                    InstanceMethod("infoGet", &MetaGroupWrapper::infoGet),
                    InstanceMethod("infoSet", &MetaGroupWrapper::infoSet),
                    InstanceMethod("infoDestroy", &MetaGroupWrapper::infoDestroy),

                    // members exposed functions
                    InstanceMethod("memberGet", &MetaGroupWrapper::memberGet),
                    InstanceMethod("memberGetOrConstruct", &MetaGroupWrapper::memberGetOrConstruct),
                    InstanceMethod(
                            "memberConstructAndSet", &MetaGroupWrapper::memberConstructAndSet),
                    InstanceMethod("memberGetAll", &MetaGroupWrapper::memberGetAll),
                    InstanceMethod(
                            "memberGetAllPendingRemovals",
                            &MetaGroupWrapper::memberGetAllPendingRemovals),
                    InstanceMethod(
                            "membersMarkPendingRemoval",
                            &MetaGroupWrapper::membersMarkPendingRemoval),
                    InstanceMethod("memberSetSupplement", &MetaGroupWrapper::memberSetSupplement),
                    InstanceMethod("memberSetInviteSent", &MetaGroupWrapper::memberSetInviteSent),
                    InstanceMethod(
                            "memberSetInviteNotSent", &MetaGroupWrapper::memberSetInviteNotSent),
                    InstanceMethod(
                            "memberSetInviteFailed", &MetaGroupWrapper::memberSetInviteFailed),
                    InstanceMethod(
                            "memberSetInviteAccepted", &MetaGroupWrapper::memberSetInviteAccepted),
                    InstanceMethod("memberSetPromoted", &MetaGroupWrapper::memberSetPromoted),
                    InstanceMethod(
                            "memberSetPromotionSent", &MetaGroupWrapper::memberSetPromotionSent),
                    InstanceMethod(
                            "memberSetPromotionFailed",
                            &MetaGroupWrapper::memberSetPromotionFailed),
                    InstanceMethod(
                            "memberSetPromotionAccepted",
                            &MetaGroupWrapper::memberSetPromotionAccepted),
                    InstanceMethod(
                            "memberSetProfileDetails", &MetaGroupWrapper::memberSetProfileDetails),
                    InstanceMethod(
                            "memberResetAllSendingState",
                            &MetaGroupWrapper::memberResetAllSendingState),
                    InstanceMethod("memberEraseAndRekey", &MetaGroupWrapper::memberEraseAndRekey),

                    // keys exposed functions
                    InstanceMethod("keysNeedsRekey", &MetaGroupWrapper::keysNeedsRekey),
                    InstanceMethod("keyRekey", &MetaGroupWrapper::keyRekey),
                    InstanceMethod("keyGetAll", &MetaGroupWrapper::keyGetAll),
                    InstanceMethod(
                            "keyGetEncryptionKeyHex", &MetaGroupWrapper::keyGetEncryptionKeyHex),
                    InstanceMethod("activeHashes", &MetaGroupWrapper::activeHashes),
                    InstanceMethod("loadKeyMessage", &MetaGroupWrapper::loadKeyMessage),
                    InstanceMethod("keyGetCurrentGen", &MetaGroupWrapper::keyGetCurrentGen),
                    InstanceMethod("encryptMessages", &MetaGroupWrapper::encryptMessages),
                    InstanceMethod("decryptMessage", &MetaGroupWrapper::decryptMessage),
                    InstanceMethod("makeSwarmSubAccount", &MetaGroupWrapper::makeSwarmSubAccount),
                    InstanceMethod("swarmSubAccountToken", &MetaGroupWrapper::swarmSubAccountToken),
                    InstanceMethod(
                            "swarmVerifySubAccount", &MetaGroupWrapper::swarmVerifySubAccount),
                    InstanceMethod("loadAdminKeys", &MetaGroupWrapper::loadAdminKeys),
                    InstanceMethod("keysAdmin", &MetaGroupWrapper::keysAdmin),
                    InstanceMethod("swarmSubaccountSign", &MetaGroupWrapper::swarmSubaccountSign),
                    InstanceMethod(
                            "generateSupplementKeys", &MetaGroupWrapper::generateSupplementKeys),
            });
}

/* #region SHARED ACTIONS */

Napi::Value MetaGroupWrapper::needsPush(const Napi::CallbackInfo& info) {

    return wrapResult(info, [&] {
        return this->meta_group->members->needs_push() || this->meta_group->info->needs_push() ||
               this->meta_group->keys->pending_config();
    });
}

Napi::Value MetaGroupWrapper::push(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        auto env = info.Env();
        auto to_push = Napi::Object::New(env);

        if (this->meta_group->members->needs_push())
            to_push["groupMember"s] = push_result_to_JS(
                    env,
                    this->meta_group->members->push(),
                    this->meta_group->members->storage_namespace());
        else
            to_push["groupMember"s] = env.Null();

        if (this->meta_group->info->needs_push())
            to_push["groupInfo"s] = push_result_to_JS(
                    env,
                    this->meta_group->info->push(),
                    this->meta_group->info->storage_namespace());
        else
            to_push["groupInfo"s] = env.Null();

        if (auto pending_config = this->meta_group->keys->pending_config())
            to_push["groupKeys"s] = push_key_entry_to_JS(
                    env, *(pending_config), this->meta_group->keys->storage_namespace());
        else
            to_push["groupKeys"s] = env.Null();

        return to_push;
    });
}

Napi::Value MetaGroupWrapper::needsDump(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        return this->meta_group->members->needs_dump() || this->meta_group->info->needs_dump() ||
               this->meta_group->keys->needs_dump();
    });
}

Napi::Value MetaGroupWrapper::metaDump(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        oxenc::bt_dict_producer combined;

        // NOTE: the keys have to be in ascii-sorted order:
        combined.append("info", session::to_string(this->meta_group->info->dump()));
        combined.append("keys", session::to_string(this->meta_group->keys->dump()));
        combined.append("members", session::to_string(this->meta_group->members->dump()));
        auto to_dump = std::move(combined).str();

        return session::to_vector(to_dump);
    });
}

Napi::Value MetaGroupWrapper::metaMakeDump(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        oxenc::bt_dict_producer combined;

        // NOTE: the keys have to be in ascii-sorted order:
        combined.append("info", session::to_string(this->meta_group->info->make_dump()));
        combined.append("keys", session::to_string(this->meta_group->keys->make_dump()));
        combined.append("members", session::to_string(this->meta_group->members->make_dump()));
        auto to_dump = std::move(combined).str();

        return session::to_vector(to_dump);
    });
}

void MetaGroupWrapper::metaConfirmPushed(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&]() {
        assertInfoLength(info, 1);
        auto arg = info[0];
        assertIsObject(arg);
        auto obj = arg.As<Napi::Object>();

        auto groupInfo = obj.Get("groupInfo");
        auto groupMember = obj.Get("groupMember");

        if (!groupInfo.IsNull() && !groupInfo.IsUndefined()) {
            assertIsObject(groupInfo);
            auto groupInfoObj = groupInfo.As<Napi::Object>();
            auto groupInfoConfirmed = confirm_pushed_entry_from_JS(info.Env(), groupInfoObj);

            this->meta_group->info->confirm_pushed(
                    std::get<0>(groupInfoConfirmed), std::get<1>(groupInfoConfirmed));
        }

        if (!groupMember.IsNull() && !groupMember.IsUndefined()) {
            assertIsObject(groupMember);
            auto groupMemberObj = groupMember.As<Napi::Object>();
            auto groupMemberConfirmed = confirm_pushed_entry_from_JS(info.Env(), groupMemberObj);

            this->meta_group->members->confirm_pushed(
                    std::get<0>(groupMemberConfirmed), std::get<1>(groupMemberConfirmed));
        }
    });
};

Napi::Value MetaGroupWrapper::metaMerge(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        auto arg = info[0];
        assertIsObject(arg);
        auto obj = arg.As<Napi::Object>();
        auto groupInfo = obj.Get("groupInfo");
        auto groupMember = obj.Get("groupMember");
        auto groupKeys = obj.Get("groupKeys");

        auto count_merged = 0;

        // Note: we need to process keys first as they might allow us the incoming info+members
        // details
        if (!groupKeys.IsNull() && !groupKeys.IsUndefined()) {
            assertIsArray(groupKeys, "metaMerge groupKeys");
            auto asArr = groupKeys.As<Napi::Array>();

            for (uint32_t i = 0; i < asArr.Length(); i++) {
                Napi::Value item = asArr[i];
                assertIsObject(item);
                if (item.IsEmpty())
                    throw std::invalid_argument("MetaMerge.item groupKeys received empty");

                Napi::Object itemObject = item.As<Napi::Object>();
                assertIsString(itemObject.Get("hash"));
                assertIsUInt8Array(itemObject.Get("data"), "groupKeys merge");
                assertIsNumber(itemObject.Get("timestampMs"), "timestampMs groupKeys");

                auto hash = toCppString(itemObject.Get("hash"), "meta.merge keys hash");
                auto data = toCppBuffer(itemObject.Get("data"), "meta.merge keys data");
                auto timestamp_ms = toCppInteger(
                        itemObject.Get("timestampMs"), "meta.merge keys timestampMs", false);

                this->meta_group->keys->load_key_message(
                        hash,
                        data,
                        timestamp_ms,
                        *(this->meta_group->info),
                        *(this->meta_group->members));
                count_merged++;  // load_key_message doesn't necessarily merge something as not
                                 // all keys are for us.
            }
        }

        if (!groupInfo.IsNull() && !groupInfo.IsUndefined()) {
            assertIsArray(groupInfo, "metaMerge groupInfo");
            auto asArr = groupInfo.As<Napi::Array>();

            std::vector<std::pair<std::string, std::vector<unsigned char>>> conf_strs;
            conf_strs.reserve(asArr.Length());

            for (uint32_t i = 0; i < asArr.Length(); i++) {
                Napi::Value item = asArr[i];
                assertIsObject(item);
                if (item.IsEmpty())
                    throw std::invalid_argument("MetaMerge.item groupInfo received empty");

                Napi::Object itemObject = item.As<Napi::Object>();
                assertIsString(itemObject.Get("hash"));
                assertIsUInt8Array(itemObject.Get("data"), "groupInfo merge");
                conf_strs.emplace_back(
                        toCppString(itemObject.Get("hash"), "meta.merge"),
                        toCppBuffer(itemObject.Get("data"), "meta.merge"));
            }

            if (conf_strs.size()) {
                auto info_merged = this->meta_group->info->merge(conf_strs);
                count_merged += info_merged.size();
            }
        }
        if (!groupMember.IsNull() && !groupMember.IsUndefined()) {
            assertIsArray(groupMember, "metaMerge groupMember");
            auto asArr = groupMember.As<Napi::Array>();

            std::vector<std::pair<std::string, std::vector<unsigned char>>> conf_strs;
            conf_strs.reserve(asArr.Length());

            for (uint32_t i = 0; i < asArr.Length(); i++) {
                Napi::Value item = asArr[i];
                assertIsObject(item);
                if (item.IsEmpty())
                    throw std::invalid_argument("MetaMerge.item groupMember received empty");

                Napi::Object itemObject = item.As<Napi::Object>();
                assertIsString(itemObject.Get("hash"));
                assertIsUInt8Array(itemObject.Get("data"), "groupMember merge");
                conf_strs.emplace_back(
                        toCppString(itemObject.Get("hash"), "meta.merge"),
                        toCppBuffer(itemObject.Get("data"), "meta.merge"));
            }

            if (conf_strs.size()) {
                auto member_merged = this->meta_group->members->merge(conf_strs);
                count_merged += member_merged.size();
            }
        }
        if (this->meta_group->keys->needs_rekey()) {
            this->meta_group->keys->rekey(*(this->meta_group->info), *(this->meta_group->members));
        }
        return count_merged;
    });
}

/* #endregion */

/* #region INFO ACTIONS */

Napi::Value MetaGroupWrapper::infoGet(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        auto env = info.Env();
        auto obj = Napi::Object::New(env);

        obj["name"] = toJs(env, this->meta_group->info->get_name());
        obj["createdAtSeconds"] = toJs(env, this->meta_group->info->get_created());
        obj["deleteAttachBeforeSeconds"] =
                toJs(env, this->meta_group->info->get_delete_attach_before());
        obj["deleteBeforeSeconds"] = toJs(env, this->meta_group->info->get_delete_before());

        if (auto expiry = this->meta_group->info->get_expiry_timer(); expiry)
            obj["expirySeconds"] = toJs(env, expiry->count());
        else
            obj["expirySeconds"] = env.Null();

        obj["isDestroyed"] = toJs(env, this->meta_group->info->is_destroyed());
        obj["profilePicture"] = toJs(env, this->meta_group->info->get_profile_pic());
        obj["description"] = toJs(env, this->meta_group->info->get_description().value_or(""));

        return obj;
    });
}

Napi::Value MetaGroupWrapper::infoSet(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        auto arg = info[0];
        assertIsObject(arg);
        auto obj = arg.As<Napi::Object>();

        // we want to not throw if the name is too long, but just truncate it
        if (auto name = maybeNonemptyString(obj.Get("name"), "MetaGroupWrapper::setInfo name"))
            this->meta_group->info->set_name_truncated(*name);

        if (auto created = maybeNonemptyInt(
                    obj.Get("createdAtSeconds"), "MetaGroupWrapper::setInfo set_created"))
            this->meta_group->info->set_created(std::move(*created));

        if (auto expiry = maybeNonemptyInt(
                    obj.Get("expirySeconds"), "MetaGroupWrapper::setInfo set_expiry_timer"))
            this->meta_group->info->set_expiry_timer(std::chrono::seconds{*expiry});

        if (auto deleteBefore = maybeNonemptyInt(
                    obj.Get("deleteBeforeSeconds"), "MetaGroupWrapper::setInfo set_delete_before"))
            this->meta_group->info->set_delete_before(std::move(*deleteBefore));

        if (auto deleteAttachBefore = maybeNonemptyInt(
                    obj.Get("deleteAttachBeforeSeconds"),
                    "MetaGroupWrapper::setInfo set_delete_attach_before"))
            this->meta_group->info->set_delete_attach_before(std::move(*deleteAttachBefore));

        if (auto profilePicture = obj.Get("profilePicture")) {
            auto profilePic = profile_pic_from_object(profilePicture);
            this->meta_group->info->set_profile_pic(profilePic);
        }

        // Note: maybeNonemptyString returns nullopt when the string is null, undefined or empty.
        // in the context of infoSet, `description` is a bit of a custom one as:
        //  - null/undefined means no change to the current value stored,
        //  - empty string means set to empty string (i.e. clear it).
        // Because of this custom behavior, we need those manual checks in place.
        if (auto description = obj.Get("description")) {
            if (description.IsString()) {
                this->meta_group->info->set_description_truncated(
                        description.ToString().Utf8Value());
            }
        }

        return this->infoGet(info);
    });
}

Napi::Value MetaGroupWrapper::infoDestroy(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        meta_group->info->destroy_group();
        return this->infoGet(info);
    });
}

/* #endregion */

/* #region MEMBERS ACTIONS */

Napi::Value MetaGroupWrapper::memberGetAll(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        std::vector<Napi::Object> allMembersJs;
        for (auto& member : *this->meta_group->members) {
            allMembersJs.push_back(
                    member_to_js(info.Env(), member, meta_group->members->get_status(member)));
        }
        return allMembersJs;
    });
}

Napi::Value MetaGroupWrapper::memberGetAllPendingRemovals(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        std::vector<Napi::Object> allMembersRemovedJs;
        for (auto& member : *this->meta_group->members) {
            auto memberStatus = this->meta_group->members->get_status(member);
            if (memberStatus == member::Status::removed_unknown ||
                memberStatus == member::Status::removed ||
                memberStatus == member::Status::removed_including_messages) {
                allMembersRemovedJs.push_back(
                        member_to_js(info.Env(), member, meta_group->members->get_status(member)));
            }
        }
        return allMembersRemovedJs;
    });
}

Napi::Value MetaGroupWrapper::memberGet(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        assertIsString(info[0]);

        auto pubkeyHex = toCppString(info[0], "memberGet");
        auto existing = meta_group->members->get(pubkeyHex);

        return existing ? member_to_js(
                                  info.Env(), *existing, meta_group->members->get_status(*existing))
                        : info.Env().Null();
    });
}

Napi::Value MetaGroupWrapper::memberGetOrConstruct(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        assertIsString(info[0]);

        auto pubkeyHex = toCppString(info[0], "memberGetOrConstruct");
        auto created = meta_group->members->get_or_construct(pubkeyHex);
        return member_to_js(info.Env(), created, meta_group->members->get_status(created));
    });
}

Napi::Value MetaGroupWrapper::memberConstructAndSet(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        assertIsString(info[0]);

        auto pubkeyHex = toCppString(info[0], "memberConstructAndSet");
        auto created = meta_group->members->get_or_construct(pubkeyHex);
        meta_group->members->set(created);
        return member_to_js(info.Env(), created, meta_group->members->get_status(created));
    });
}

void MetaGroupWrapper::memberSetSupplement(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertIsString(info[0]);

        auto pubkeyHex = toCppString(info[0], "memberSetSupplement pubkeyHex");
        auto m = this->meta_group->members->get(pubkeyHex);
        if (m) {
            m->supplement = true;
            this->meta_group->members->set(*m);
        }
    });
}

void MetaGroupWrapper::memberSetInviteFailed(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertIsString(info[0]);
        auto pubkeyHex = toCppString(info[0], "memberSetInviteFailed");

        auto m = this->meta_group->members->get(pubkeyHex);
        if (m) {
            m->set_invite_failed();
            this->meta_group->members->set(*m);
        }
    });
}

void MetaGroupWrapper::memberSetInviteSent(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertIsString(info[0]);
        auto pubkeyHex = toCppString(info[0], "memberSetInviteSent");

        auto m = this->meta_group->members->get(pubkeyHex);
        if (m) {
            m->set_invite_sent();
            this->meta_group->members->set(*m);
        }
    });
}

void MetaGroupWrapper::memberSetInviteNotSent(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertIsString(info[0]);
        auto pubkeyHex = toCppString(info[0], "memberSetInviteNotSent");

        auto m = this->meta_group->members->get(pubkeyHex);
        if (m) {
            m->set_invite_not_sent();
            this->meta_group->members->set(*m);
        }
    });
}

void MetaGroupWrapper::memberSetInviteAccepted(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 1);
        assertIsString(info[0]);

        auto pubkeyHex = toCppString(info[0], "memberSetInviteAccepted");
        auto m = this->meta_group->members->get(pubkeyHex);
        if (m) {
            m->set_invite_accepted();
            this->meta_group->members->set(*m);
        }
    });
}

void MetaGroupWrapper::memberSetPromoted(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 1);
        assertIsString(info[0]);
        auto pubkeyHex = toCppString(info[0], "memberSetPromoted");
        auto m = this->meta_group->members->get(pubkeyHex);
        if (m) {
            m->set_promoted();
            this->meta_group->members->set(*m);
        }
    });
}

void MetaGroupWrapper::memberSetPromotionSent(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 1);
        assertIsString(info[0]);
        auto pubkeyHex = toCppString(info[0], "memberSetPromotionSent");
        auto m = this->meta_group->members->get(pubkeyHex);
        if (m) {
            m->set_promotion_sent();
            this->meta_group->members->set(*m);
        }
    });
}

void MetaGroupWrapper::memberSetPromotionFailed(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 1);
        assertIsString(info[0]);
        auto pubkeyHex = toCppString(info[0], "memberSetPromotionFailed");
        auto m = this->meta_group->members->get(pubkeyHex);
        if (m) {
            m->set_promotion_failed();
            this->meta_group->members->set(*m);
        }
    });
}

void MetaGroupWrapper::memberSetPromotionAccepted(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 1);
        assertIsString(info[0]);
        auto pubkeyHex = toCppString(info[0], "memberSetPromotionAccepted");
        auto m = this->meta_group->members->get(pubkeyHex);
        if (m) {
            m->set_promotion_accepted();
            this->meta_group->members->set(*m);
        }
    });
}

void MetaGroupWrapper::memberSetProfileDetails(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 2);
        assertIsString(info[0]);
        assertIsObject(info[1]);

        auto pubkeyHex = toCppString(info[0], "memberSetProfileDetails");

        auto m = this->meta_group->members->get(pubkeyHex);
        auto argsAsObj = info[1].As<Napi::Object>();
        auto updatedAtSeconds =
                toCppSysSeconds(argsAsObj.Get("profileUpdatedSeconds"), "memberSetProfileDetails");

        // if the profile details provided are more recent that the ones saved, update them.
        // we also allow anything when our current value is 0, as it means we haven't got an updated
        // profileDetails yet
        if (m && (updatedAtSeconds > m->profile_updated ||
                  m->profile_updated.time_since_epoch().count() == 0)) {
            m->profile_updated = updatedAtSeconds;

            auto profilePicture = profile_pic_from_object(argsAsObj.Get("profilePicture"));
            m->profile_picture = profilePicture;

            // this will truncate silently if the name is too long
            auto newName = toCppString(argsAsObj.Get("name"), "memberSetProfileDetails newName");
            m->set_name_truncated(newName);

            this->meta_group->members->set(*m);
        }
    });
}

Napi::Value MetaGroupWrapper::memberResetAllSendingState(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        bool changed = false;
        for (auto& member : *this->meta_group->members) {
            auto sending = this->meta_group->members->has_pending_send(member.session_id);
            if (sending) {
                this->meta_group->members->set_pending_send(member.session_id, false);
                changed = true;
            }
        }
        return changed;
    });
}

void MetaGroupWrapper::membersMarkPendingRemoval(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 2);
        auto toUpdateJSValue = info[0];
        auto withMessageJSValue = info[1];

        assertIsArray(toUpdateJSValue, "membersMarkPendingRemoval");
        assertIsBoolean(withMessageJSValue);
        bool withMessages = toCppBoolean(withMessageJSValue, "membersMarkPendingRemoval");

        auto toUpdateJS = toUpdateJSValue.As<Napi::Array>();
        for (uint32_t i = 0; i < toUpdateJS.Length(); i++) {
            auto pubkeyHex = toCppString(toUpdateJS[i], "membersMarkPendingRemoval");
            auto existing = this->meta_group->members->get(pubkeyHex);
            if (existing) {
                existing->set_removed(withMessages);
                this->meta_group->members->set(*existing);
            }
        }
    });
}

Napi::Value MetaGroupWrapper::memberEraseAndRekey(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        auto toRemoveJSValue = info[0];

        assertIsArray(toRemoveJSValue, "memberEraseAndRekey");

        auto toRemoveJS = toRemoveJSValue.As<Napi::Array>();
        auto rekeyed = false;
        for (uint32_t i = 0; i < toRemoveJS.Length(); i++) {
            auto pubkeyHex = toCppString(toRemoveJS[i], "memberEraseAndRekey");
            rekeyed |= this->meta_group->members->erase(pubkeyHex);
        }

        if (rekeyed) {
            meta_group->keys->rekey(*(this->meta_group->info), *(this->meta_group->members));
        }

        return rekeyed;
    });
}

/* #endregion */

/* #region KEYS ACTIONS */
Napi::Value MetaGroupWrapper::keysNeedsRekey(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return meta_group->keys->needs_rekey(); });
}

Napi::Value MetaGroupWrapper::keyRekey(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        return meta_group->keys->rekey(*(meta_group->info), *(meta_group->members));
    });
}

Napi::Value MetaGroupWrapper::keyGetAll(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return meta_group->keys->group_keys(); });
}

Napi::Value MetaGroupWrapper::keyGetEncryptionKeyHex(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return to_hex(meta_group->keys->group_enc_key()); });
}

Napi::Value MetaGroupWrapper::loadKeyMessage(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 3);
        assertIsString(info[0]);
        assertIsUInt8Array(info[1], "loadKeyMessage");
        assertIsNumber(info[2], "loadKeyMessage");

        auto hash = toCppString(info[0], "loadKeyMessage");
        auto data = toCppBuffer(info[1], "loadKeyMessage");
        auto timestamp_ms = toCppInteger(info[2], "loadKeyMessage");

        return meta_group->keys->load_key_message(
                hash, data, timestamp_ms, *(this->meta_group->info), *(this->meta_group->members));
    });
}

Napi::Value MetaGroupWrapper::keyGetCurrentGen(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 0);
        return meta_group->keys->current_generation();
    });
}

Napi::Value MetaGroupWrapper::activeHashes(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        auto keysHashes = meta_group->keys->active_hashes();
        auto infoHashes = meta_group->info->active_hashes();
        auto memberHashes = meta_group->members->active_hashes();
        std::vector<std::string> merged;
        std::copy(std::begin(keysHashes), std::end(keysHashes), std::back_inserter(merged));
        std::copy(std::begin(infoHashes), std::end(infoHashes), std::back_inserter(merged));
        std::copy(std::begin(memberHashes), std::end(memberHashes), std::back_inserter(merged));

        return merged;
    });
}

Napi::Value MetaGroupWrapper::encryptMessages(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        assertIsArray(info[0], "encryptMessages");

        auto plaintextsJS = info[0].As<Napi::Array>();
        uint32_t arrayLength = plaintextsJS.Length();
        std::vector<std::vector<unsigned char>> encryptedMessages;
        encryptedMessages.reserve(arrayLength);

        for (uint32_t i = 0; i < plaintextsJS.Length(); i++) {
            auto plaintext = toCppBuffer(plaintextsJS[i], "encryptMessages");

            encryptedMessages.push_back(this->meta_group->keys->encrypt_message(plaintext));
        }
        return encryptedMessages;
    });
}

Napi::Value MetaGroupWrapper::decryptMessage(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        assertIsUInt8Array(info[0], "decryptMessage");

        auto ciphertext = toCppBuffer(info[0], "decryptMessage");
        auto decrypted = this->meta_group->keys->decrypt_message(ciphertext);

        return decrypt_result_to_JS(info.Env(), decrypted);
    });
}

Napi::Value MetaGroupWrapper::makeSwarmSubAccount(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        assertIsString(info[0]);

        auto memberPk = toCppString(info[0], "makeSwarmSubAccount");
        std::vector<unsigned char> subaccount =
                this->meta_group->keys->swarm_make_subaccount(memberPk);

        session::nodeapi::checkOrThrow(
                subaccount.size() == 100, "expected subaccount to be 100 bytes long");

        return subaccount;
    });
}

Napi::Value MetaGroupWrapper::swarmSubAccountToken(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        assertIsString(info[0]);

        auto memberPk = toCppString(info[0], "swarmSubAccountToken");
        std::vector<unsigned char> subaccount =
                this->meta_group->keys->swarm_subaccount_token(memberPk);

        session::nodeapi::checkOrThrow(
                subaccount.size() == 36, "expected subaccount token to be 36 bytes long");

        return oxenc::to_hex(subaccount.begin(), subaccount.end());
    });
}

Napi::Value MetaGroupWrapper::swarmVerifySubAccount(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        assertIsUInt8Array(info[0], "swarmVerifySubAccount");

        auto signingValue = toCppBuffer(info[0], "swarmVerifySubAccount");
        return this->meta_group->keys->swarm_verify_subaccount(signingValue);
    });
}

Napi::Value MetaGroupWrapper::loadAdminKeys(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        assertIsUInt8Array(info[0], "loadAdminKeys");

        auto secret = toCppBuffer(info[0], "loadAdminKeys");
        this->meta_group->keys->load_admin_key(
                secret, *(this->meta_group->info), *(this->meta_group->members));
        return info.Env().Null();
    });
}

Napi::Value MetaGroupWrapper::keysAdmin(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 0);
        return this->meta_group->keys->admin();
    });
}

Napi::Value MetaGroupWrapper::generateSupplementKeys(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        auto membersJSValue = info[0];
        assertIsArray(membersJSValue, "generateSupplementKeys");

        auto membersJS = membersJSValue.As<Napi::Array>();
        uint32_t arrayLength = membersJS.Length();
        std::vector<std::string> membersToAdd;
        membersToAdd.reserve(arrayLength);
        std::vector<std::string> membersCpp;
        membersCpp.reserve(arrayLength);

        for (uint32_t i = 0; i < membersJS.Length(); i++) {
            auto memberPk = toCppString(membersJS[i], "generateSupplementKeys");
            membersCpp.push_back(memberPk);
        }
        return this->meta_group->keys->key_supplement(membersCpp);
    });
}

Napi::Value MetaGroupWrapper::swarmSubaccountSign(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 2);
        assertIsUInt8Array(info[0], "swarmSubaccountSign 0");
        assertIsUInt8Array(info[1], "swarmSubaccountSign 1");

        auto message = toCppBuffer(info[0], "swarmSubaccountSign message");
        auto authdata = toCppBuffer(info[1], "swarmSubaccountSign authdata");
        auto subaccountSign = this->meta_group->keys->swarm_subaccount_sign(message, authdata);

        return subaccountSign;
    });
}
/* #endregion */

}  // namespace session::nodeapi
