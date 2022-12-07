#pragma once

#include <nan.h>

#include "base_config.hpp"

#include "session/config/user_profile.hpp"

using session::ustring_view;

class UserConfigWrapper : public ConfigBaseWrapper {
public:
  static NAN_MODULE_INIT(Init);

private:
  explicit UserConfigWrapper(ustring_view ed25519_secretkey,
                             std::optional<ustring_view> dumped) {
    initWithConfig(new session::config::UserProfile(ed25519_secretkey, dumped));
  }

  static NAN_METHOD(New);
  static NAN_METHOD(GetName);
  static NAN_METHOD(SetName);
  static NAN_METHOD(GetProfilePic);
  static NAN_METHOD(SetProfilePic);
};

NODE_MODULE(session_util_wrapper, UserConfigWrapper::Init)