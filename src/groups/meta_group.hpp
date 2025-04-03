#pragma once

#include <napi.h>
#include <vector>

#include "session/config/groups/info.hpp"
#include "session/config/groups/keys.hpp"
#include "session/config/groups/members.hpp"

namespace session::nodeapi {

using config::groups::Info;
using config::groups::Keys;
using config::groups::Members;
using session::config::profile_pic;
using std::pair;
using std::string;
using std::tuple;
using std::vector;

using std::make_shared;
using std::shared_ptr;

class MetaGroup {
  public:
    shared_ptr<config::groups::Info> info;
    shared_ptr<config::groups::Members> members;
    shared_ptr<config::groups::Keys> keys;
    string edGroupPubKey;
    std::optional<string> edGroupSecKey;

    MetaGroup(
            shared_ptr<config::groups::Info> info,
            shared_ptr<config::groups::Members> members,
            shared_ptr<config::groups::Keys> keys,
            std::vector<unsigned char> edGroupPubKey,
            std::optional<std::vector<unsigned char>> edGroupSecKey) :
            info{info}, members{members}, keys{keys}, edGroupPubKey{oxenc::to_hex(edGroupPubKey.begin(), edGroupPubKey.end())} {

        if (edGroupSecKey.has_value()) {
            this->edGroupSecKey = oxenc::to_hex(edGroupSecKey->begin(), edGroupSecKey->end());
        } else {
            this->edGroupSecKey = std::nullopt;
        }
    }
};
}  // namespace session::nodeapi