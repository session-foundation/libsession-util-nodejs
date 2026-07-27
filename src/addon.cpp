#include <napi.h>

#include <mutex>
#include <oxen/log.hpp>

#include "blinding/blinding.hpp"
#include "constants.hpp"
#include "contacts_config.hpp"
#include "convo_info_volatile_config.hpp"
#include "encrypt_decrypt/encrypt_decrypt.hpp"
#include "groups/meta_group_wrapper.hpp"
#include "pro/pro.hpp"
#include "user_config.hpp"
#include "user_groups_config.hpp"

Napi::ThreadSafeFunction tsfn;
// Guards `tsfn` against the race between libsession's background log
// threads (which read it and BlockingCall through it) and N-API env
// teardown (which Releases and nulls it). The check + call in the
// logger and the check + release in the cleanup hook must each be
// atomic w.r.t. the other, or we risk a torn read / use-after-free on
// the wrapper.
std::mutex tsfn_mutex;

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {

    {
        std::lock_guard<std::mutex> lock(tsfn_mutex);
        tsfn = Napi::ThreadSafeFunction::New(
                env,
                Napi::Function::New(env, [](const Napi::CallbackInfo& info) {}),
                "LoggerCallback",
                0,
                1);
        // The logger callback is fire-and-forget. Without Unref(), the
        // TSFN keeps a strong ref on the loop and a `require()` from a
        // short-lived CLI hangs forever waiting on the TSFN.
        tsfn.Unref(env);
    }

    // Release the TSFN when the N-API env tears down. Without this, a
    // later libsession log from a background thread could BlockingCall
    // into a destroyed env (abort / UAF). Taking the lock makes the
    // check + release atomic w.r.t. the logger callback below.
    env.AddCleanupHook([]() {
        std::lock_guard<std::mutex> lock(tsfn_mutex);
        if (tsfn) {
            tsfn.Release();
            tsfn = nullptr;
        }
    });

    // Register the libsession -> console.log bridge exactly once per
    // process. Re-running InitAll (multiple Workers / vm contexts) must
    // not stack duplicate callbacks onto libsession's global logger
    // registry — the single persistent callback always reads whatever
    // the current `tsfn` is, under the lock.
    static std::once_flag logger_once;
    std::call_once(logger_once, [] {
        session::add_logger([](std::string_view msg) {
            // Hold the lock across the check + BlockingCall so env
            // teardown can't Release/null `tsfn` underneath us.
            std::lock_guard<std::mutex> lock(tsfn_mutex);
            if (!tsfn)
                return;
            auto* payload = new std::string(msg);
            napi_status status = tsfn.BlockingCall(
                    payload, [](Napi::Env env, Napi::Function jsCallback, std::string* msg) {
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
            // On a rejected call (e.g. napi_closing if a Release slipped
            // in) the finalize callback never runs, so the payload would
            // leak — free it here instead.
            if (status != napi_ok)
                delete payload;
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
