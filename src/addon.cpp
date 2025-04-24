#include <napi.h>

#include <oxen/log.hpp>

#include "blinding/blinding.hpp"
#include "constants.hpp"
#include "contacts_config.hpp"
#include "convo_info_volatile_config.hpp"
#include "groups/meta_group_wrapper.hpp"
#include "multi_encrypt/multi_encrypt.hpp"
#include "user_config.hpp"
#include "user_groups_config.hpp"

Napi::ThreadSafeFunction tsfn;

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    using namespace session::nodeapi;

    tsfn = Napi::ThreadSafeFunction::New(
            env,
            Napi::Function::New(env, [](const Napi::CallbackInfo& info) {}),
            "LoggerCallback",
            0,
            1);

    session::add_logger([](std::string_view msg) {
        tsfn.BlockingCall(
                new std::string(msg),
                [](Napi::Env env, Napi::Function jsCallback, std::string* msg) {
                    Napi::HandleScope scope(env);
                    Napi::Function consoleLog = env.Global()
                                                        .Get("console")
                                                        .As<Napi::Object>()
                                                        .Get("log")
                                                        .As<Napi::Function>();
                    Napi::String jsStr = Napi::String::New(env, "libsession-util: " + *msg);
                    consoleLog.Call({jsStr});
                    delete msg;
                });
    });
    oxen::log::set_level_default(oxen::log::Level::info);

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
