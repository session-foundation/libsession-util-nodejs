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

/**
 * Note: After days of trying to get the libsession-util logger to play nice with console.log,
 * I've decided to just not log anything from libsession for now.
 * Note for my future self:
 * - we need a thread-safe function to be called in the add_logger callback here because if
 *   the `env` is not valid it will crash the whole app without an error at all,
 * - macos crashes a lot more easily than linux, so better to debug this issue on macOS,
 * - using the thread-safe function below is working for linux, and kind of working for macOS, but
 *   it also prevents the normal logging from being written to the inspector (macOS only)
 *
 * For all those reasons, and because we want a point release soon, there is no logging from
 * libsession-util for now.
 */
// Napi::ThreadSafeFunction tsfn;
// tsfn = Napi::ThreadSafeFunction::New(
//         env,
//         Napi::Function::New(env, [](const Napi::CallbackInfo& info) {}),
//         "LoggerCallback",
//         0,
//         1);

// session::add_logger([](std::string_view msg) {
//     tsfn.BlockingCall(
//             new std::string(msg),
//             [](Napi::Env env, Napi::Function jsCallback, std::string* msg) {
//                 Napi::HandleScope scope(env);
//                 Napi::Function consoleLog = env.Global()
//                                                     .Get("console")
//                                                     .As<Napi::Object>()
//                                                     .Get("log")
//                                                     .As<Napi::Function>();
//                 Napi::String jsStr = Napi::String::New(env, "libsession-util: " + *msg);
//                 consoleLog.Call({jsStr});
//                 delete msg;
//             });
// });
// oxen::log::set_level_default(oxen::log::Level::info);

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    using namespace session::nodeapi;

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
