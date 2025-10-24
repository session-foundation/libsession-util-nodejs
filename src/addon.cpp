#include <napi.h>

#include <oxen/log.hpp>

#include "blinding/blinding.hpp"
#include "constants.hpp"
#include "contacts_config.hpp"
#include "convo_info_volatile_config.hpp"
#include "groups/meta_group_wrapper.hpp"
#include "multi_encrypt/multi_encrypt.hpp"
#include "pro/pro.hpp"
#include "user_config.hpp"
#include "user_groups_config.hpp"

Napi::ThreadSafeFunction tsfn;

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {

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
                    Napi::String jsStr = Napi::String::New(env, "libsession: " + *msg);
                    consoleLog.Call({jsStr});
                    delete msg;
                });
    });
    oxen::log::set_level_default(oxen::log::Level::info);

    session::nodeapi::ConstantsWrapper::Init(env, exports);

    // Group wrappers init
    session::nodeapi::MetaGroupWrapper::Init(env, exports);

    // User wrappers init
    session::nodeapi::UserConfigWrapper::Init(env, exports);
    session::nodeapi::ContactsConfigWrapper::Init(env, exports);
    session::nodeapi::UserGroupsWrapper::Init(env, exports);
    session::nodeapi::ConvoInfoVolatileWrapper::Init(env, exports);

    // Fully static wrappers init
    session::nodeapi::MultiEncryptWrapper::Init(env, exports);
    session::nodeapi::ProWrapper::Init(env, exports);
    session::nodeapi::BlindingWrapper::Init(env, exports);

    return exports;
}

NODE_API_MODULE(libsession_util_nodejs, InitAll);
