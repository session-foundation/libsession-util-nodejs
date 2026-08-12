#pragma once

#include <napi.h>

#include "base_config.hpp"
#include "session/config/pro.hpp"
#include "session/config/user_profile.hpp"
#include "utilities.hpp"

namespace session::nodeapi {

class UserConfigWrapper : public ConfigBaseImpl, public Napi::ObjectWrap<UserConfigWrapper> {
  public:
    static void Init(Napi::Env env, Napi::Object exports);

    explicit UserConfigWrapper(const Napi::CallbackInfo& info);

  private:
    config::UserProfile& config{get_config<config::UserProfile>()};

    Napi::Value getPriority(const Napi::CallbackInfo& info);
    Napi::Value getName(const Napi::CallbackInfo& info);
    Napi::Value getProfilePic(const Napi::CallbackInfo& info);

    void setPriority(const Napi::CallbackInfo& info);
    void setName(const Napi::CallbackInfo& info);
    void setNameTruncated(const Napi::CallbackInfo& info);
    void setNewProfilePic(const Napi::CallbackInfo& info);

    void setReuploadProfilePic(const Napi::CallbackInfo& info);
    Napi::Value getProfileUpdatedSeconds(const Napi::CallbackInfo& info);

    Napi::Value getEnableBlindedMsgRequest(const Napi::CallbackInfo& info);
    void setEnableBlindedMsgRequest(const Napi::CallbackInfo& info);

    Napi::Value getNoteToSelfExpiry(const Napi::CallbackInfo& info);
    void setNoteToSelfExpiry(const Napi::CallbackInfo& info);

    Napi::Value getProConfig(const Napi::CallbackInfo& info);
    void setProConfig(const Napi::CallbackInfo& info);
    Napi::Value removeProConfig(const Napi::CallbackInfo& info);

    Napi::Value getProAccessExpiry(const Napi::CallbackInfo& info);
    Napi::Value getProProfileBitset(const Napi::CallbackInfo& info);
    void setProBadge(const Napi::CallbackInfo& info);
    void setAnimatedAvatar(const Napi::CallbackInfo& info);
    void setProAccessExpiry(const Napi::CallbackInfo& info);

    Napi::Value generateProMasterKey(const Napi::CallbackInfo& info);
    Napi::Value generateRotatingPrivKeyHex(const Napi::CallbackInfo& info);

    // Derive the rotating seed (and its ed25519 keypair) for `now` from the Pro master key
    // (libsession owns the rotation schedule), so every device converges on the same key. Replaces
    // ad-hoc/random rotating-key generation.
    Napi::Value deriveProRotatingKey(const Napi::CallbackInfo& info);

    // Refund-requested (config key R) and pro-prepaid / purchase-in-flight (config key I) markers,
    // and the renewal-target poll that decides when to (re)request a proof.
    Napi::Value getRefundRequested(const Napi::CallbackInfo& info);
    void setRefundRequested(const Napi::CallbackInfo& info);
    Napi::Value getProPrepaid(const Napi::CallbackInfo& info);
    void setProPrepaid(const Napi::CallbackInfo& info);
    // Auto-renewing (config key A). Presence-only in core: set_pro_auto_renewing uses
    // set_nonzero_int, so writing false ERASES the key — absent means terminal/unknown, and a
    // caller must compare the getter's value rather than the key's presence.
    Napi::Value getProAutoRenewing(const Napi::CallbackInfo& info);
    void setProAutoRenewing(const Napi::CallbackInfo& info);
    // Grace period (config key G), in ms on the JS side. Synced alongside E so any linked device
    // can derive when coverage ends as E + G: E is the account's true paid-through expiry, and the
    // backend keeps serving for G past it, so [E, E + G) is expired-but-still-served. This is the
    // ACCOUNT-level grace, not the per-payment field of the same name. Deliberately not optional --
    // the backend sends 0 when not auto-renewing, and E + 0 == E.
    Napi::Value getProGracePeriod(const Napi::CallbackInfo& info);
    void setProGracePeriod(const Napi::CallbackInfo& info);
    Napi::Value getProRenewalTarget(const Napi::CallbackInfo& info);
};
};  // namespace session::nodeapi
