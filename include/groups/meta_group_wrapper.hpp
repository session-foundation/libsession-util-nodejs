#pragma once

#include <napi.h>

#include "../meta/meta_base_wrapper.hpp"
#include "../profile_pic.hpp"
#include "../utilities.hpp"
#include "./meta_group.hpp"
#include "oxenc/bt_producer.h"
#include "session/config/groups/members.hpp"
#include "session/config/user_groups.hpp"

namespace session::nodeapi {
using config::groups::Members;
using session::config::GROUP_DESTROYED;
using session::config::KICKED_FROM_GROUP;
using session::config::NOT_REMOVED;
using session::config::groups::member;
using session::nodeapi::MetaGroup;

Napi::Object member_to_js(const Napi::Env& env, const member& info, const member::Status& status);

template <>
struct toJs_impl<Keys::swarm_auth> {
    Napi::Object operator()(const Napi::Env& env, const Keys::swarm_auth& auth) {
        auto obj = Napi::Object::New(env);

        obj["subaccount"] = toJs(env, auth.subaccount);
        obj["subaccount_sig"] = toJs(env, auth.subaccount_sig);
        obj["signature"] = toJs(env, auth.signature);
        return obj;
    }
};

class MetaGroupWrapper : public Napi::ObjectWrap<MetaGroupWrapper> {
  public:
    static void Init(Napi::Env env, Napi::Object exports);

    explicit MetaGroupWrapper(const Napi::CallbackInfo& info);

  private:
    std::unique_ptr<MetaGroup> meta_group;

    /* Shared Actions */
    Napi::Value needsPush(const Napi::CallbackInfo& info);
    Napi::Value push(const Napi::CallbackInfo& info);
    Napi::Value needsDump(const Napi::CallbackInfo& info);
    Napi::Value metaDump(const Napi::CallbackInfo& info);
    Napi::Value metaMakeDump(const Napi::CallbackInfo& info);
    void metaConfirmPushed(const Napi::CallbackInfo& info);
    Napi::Value metaMerge(const Napi::CallbackInfo& info);

    /** Info Actions */
    Napi::Value infoGet(const Napi::CallbackInfo& info);
    Napi::Value infoSet(const Napi::CallbackInfo& info);
    Napi::Value infoDestroy(const Napi::CallbackInfo& info);

    /** Members Actions */
    Napi::Value memberGetAll(const Napi::CallbackInfo& info);
    Napi::Value memberGetAllPendingRemovals(const Napi::CallbackInfo& info);
    Napi::Value memberGet(const Napi::CallbackInfo& info);
    Napi::Value memberGetOrConstruct(const Napi::CallbackInfo& info);
    Napi::Value memberConstructAndSet(const Napi::CallbackInfo& info);

    void memberSetNameTruncated(const Napi::CallbackInfo& info);
    void memberSetInviteFailed(const Napi::CallbackInfo& info);
    void memberSetInviteSent(const Napi::CallbackInfo& info);
    void memberSetInviteNotSent(const Napi::CallbackInfo& info);
    void memberSetInviteAccepted(const Napi::CallbackInfo& info);
    void memberSetPromoted(const Napi::CallbackInfo& info);
    void memberSetPromotionSent(const Napi::CallbackInfo& info);
    void memberSetPromotionFailed(const Napi::CallbackInfo& info);
    void memberSetPromotionAccepted(const Napi::CallbackInfo& info);
    void memberSetProfilePicture(const Napi::CallbackInfo& info);
    void memberSetProfileUpdatedSeconds(const Napi::CallbackInfo& info);
    Napi::Value memberResetAllSendingState(const Napi::CallbackInfo& info);
    void memberSetSupplement(const Napi::CallbackInfo& info);
    Napi::Value memberEraseAndRekey(const Napi::CallbackInfo& info);
    void membersMarkPendingRemoval(const Napi::CallbackInfo& info);

    /** Keys Actions */
    Napi::Value keysNeedsRekey(const Napi::CallbackInfo& info);
    Napi::Value keyRekey(const Napi::CallbackInfo& info);
    Napi::Value keyGetAll(const Napi::CallbackInfo& info);
    Napi::Value loadKeyMessage(const Napi::CallbackInfo& info);
    Napi::Value keyGetCurrentGen(const Napi::CallbackInfo& info);
    Napi::Value activeHashes(const Napi::CallbackInfo& info);
    Napi::Value encryptMessages(const Napi::CallbackInfo& info);
    Napi::Value decryptMessage(const Napi::CallbackInfo& info);
    Napi::Value makeSwarmSubAccount(const Napi::CallbackInfo& info);
    Napi::Value swarmSubAccountToken(const Napi::CallbackInfo& info);
    Napi::Value generateSupplementKeys(const Napi::CallbackInfo& info);
    Napi::Value swarmSubaccountSign(const Napi::CallbackInfo& info);
    Napi::Value swarmVerifySubAccount(const Napi::CallbackInfo& info);
    Napi::Value loadAdminKeys(const Napi::CallbackInfo& info);
    Napi::Value keysAdmin(const Napi::CallbackInfo& info);
};

}  // namespace session::nodeapi
