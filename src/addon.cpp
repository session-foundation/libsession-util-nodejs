#include <napi.h>

#include "blinding/blinding.hpp"
#include "constants.hpp"
#include "contacts_config.hpp"
#include "convo_info_volatile_config.hpp"
#include "groups/meta_group_wrapper.hpp"
#include "multi_encrypt/multi_encrypt.hpp"
#include "user_config.hpp"
#include "user_groups_config.hpp"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    using namespace session::nodeapi;

    session::add_logger([env](std::string_view msg) {
        std::string toLog = std::string("libsession-util: ") + std::string(msg) + "\n";

        Napi::Function consoleLog =
                env.Global().Get("console").As<Napi::Object>().Get("log").As<Napi::Function>();
        consoleLog.Call({Napi::String::New(env, toLog)});
    });

    // session::logger_set_level_default(session::LogLevel::debug);

    ConstantsWrapper::Init(env, exports);

    // Group wrappers init
    MetaGroupWrapper::Init(env, exports);

    // User wrappers init
    UserConfigWrapper::Init(env, exports);
    ContactsConfigWrapper::Init(env, exports);
    UserGroupsWrapper::Init(env, exports);
    ConvoInfoVolatileWrapper::Init(env, exports);

    // Fully static wrappers init
    MultiEncryptWrapper::Init(env, exports);
    BlindingWrapper::Init(env, exports);

    return exports;
}

NODE_API_MODULE(libsession_util_nodejs, InitAll);
