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
    // FIXME: wrap those in the extra data field of UserConfig instead
    std::optional<config::ProConfig> pro_config;
    int64_t pro_user_features = 0;

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
    Napi::Value getProFeaturesBitset(const Napi::CallbackInfo& info);
    void setProFeaturesBitset(const Napi::CallbackInfo& info);

    Napi::Value generateProMasterKey(const Napi::CallbackInfo& info);
    Napi::Value generateRotatingPrivKeyHex(const Napi::CallbackInfo& info);
};
};  // namespace session::nodeapi
