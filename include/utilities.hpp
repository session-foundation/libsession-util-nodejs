#pragma once

#include <fmt/format.h>
#include <napi.h>

#include <chrono>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <unordered_set>
#include <vector>

#include "oxen/log/catlogger.hpp"
#include "oxenc/base64.h"
#include "oxenc/hex.h"
#include "session/config/namespaces.hpp"
#include "session/config/profile_pic.hpp"
#include "session/session_protocol.hpp"
#include "session/types.h"
#include "session/types.hpp"

namespace session::nodeapi {

using namespace std::literals;

#ifdef _MSC_VER
#define UNREACHABLE() __assume(0)
#else
#define UNREACHABLE() __builtin_unreachable()
#endif

inline auto cat = oxen::log::Cat("nodeapi");

static void checkOrThrow(bool condition, const char* msg) {
    if (!condition)
        throw std::invalid_argument{msg};
}

void assertInfoLength(const Napi::CallbackInfo& info, const int expected);

void assertInfoMinLength(const Napi::CallbackInfo& info, const int minLength);

void assertIsStringOrNull(const Napi::Value& value, const std::string& identifier = "");
void assertIsNumber(const Napi::Value& value, const std::string& identifier);
void assertIsNumberOrNull(const Napi::Value& val, const std::string& identifier);
void assertIsBigint(const Napi::Value& val, const std::string& identifier);
void assertIsArray(const Napi::Value& value, const std::string& identifier);
void assertIsObject(const Napi::Value& value);
void assertIsUInt8ArrayOrNull(const Napi::Value& value);
void assertIsUInt8Array(const Napi::Value& value, const std::string& identifier);
void assertIsString(const Napi::Value& value, const std::string& identifier = "");
void assertIsBoolean(const Napi::Value& value, const std::string& identifier = "");

// Checks for and returns exactly N string arguments.  If N == 1 this return just a string; if > 1
// this returns an std::array of strings of size N.
template <size_t N>
auto getStringArgs(const Napi::CallbackInfo& info) {
    assertInfoLength(info, N);
    std::array<std::string, N> args;
    for (int i = 0; i < args.size(); i++) {
        auto arg = info[i];
        assertIsString(arg, "getStringArgs");
        args[i] = arg.As<Napi::String>().Utf8Value();
    }
    if constexpr (N == 1)
        return std::move(args[0]);
    else
        return args;
}

std::string toCppString(Napi::Value x, const std::string& identifier);
std::span<const unsigned char> toCppBufferView(Napi::Value x, const std::string& identifier);
std::vector<unsigned char> toCppBuffer(Napi::Value x, const std::string& identifier);

int64_t toCppInteger(Napi::Value x, const std::string& identifier, bool allowUndefined = false);

/**
 * Same as toCppInteger, but for BigInt
 */
int64_t toCppIntegerB(Napi::Value x, const std::string& identifier, bool allowUndefined = false);

std::optional<int64_t> maybeNonemptyInt(Napi::Value x, const std::string& identifier);
std::optional<int64_t> maybeNonemptyIntB(Napi::Value x, const std::string& identifier);

std::optional<bool> maybeNonemptyBoolean(Napi::Value x, const std::string& identifier);
std::optional<std::chrono::sys_seconds> maybeNonemptySysSeconds(
        Napi::Value x, const std::string& identifier);

std::optional<std::chrono::sys_time<std::chrono::milliseconds>> maybeNonemptyTimeMs(
        Napi::Value x, const std::string& identifier);

std::chrono::sys_seconds toCppSysSeconds(Napi::Value x, const std::string& identifier);
std::chrono::sys_time<std::chrono::milliseconds> toCppSysMs(
        Napi::Value x, const std::string& identifier);
std::chrono::milliseconds toCppMs(Napi::Value x, const std::string& identifier);

bool toCppBoolean(Napi::Value x, const std::string& identifier);

// If the object is null/undef/empty returns nullopt, otherwise if a String returns a
// std::string of the value.  Throws if something else.
std::optional<std::string> maybeNonemptyString(Napi::Value x, const std::string& identifier);

// If the object is null/undef/empty returns nullopt, otherwise if a Uint8Array returns a
// std::vector<unsigned char> of the value.  Throws if something else.
std::optional<std::vector<unsigned char>> maybeNonemptyBuffer(
        Napi::Value x, const std::string& identifier);

// Implementation struct of toJs(); we add specializations of this for any C++ types we want to
// be able to convert into JS types.
template <typename T, typename SFINAE = void>
struct toJs_impl {
    // If this gets instantiated it means we're missing a specialization and so fail to compile:
    static_assert(!std::is_same_v<T, T>, "toJs() does not have an implementation for type T");
};

// Converts C++ values of various types to a Napi value; in particular if you just include this
// header then you get:
//
// - bool -> Boolean
// - other arithmetic types -> Number
// - string, string_view -> String
// - std::vector<unsigned char>, std::span<const unsigned char> -> Buffer
// - std::vector<T> -> Array, where elements are created via toJs calls on the vector elements.
// - std::optional<T> -> Null if empty, otherwise the result of toJs on the contained value
// - Napi::Value (or derived) -> itself (this is mainly so that you can return a std::vector or
//   std::optional of a Napi::Value and get the Right Thing).
//
// but others can be added in other headers by adding additional specializations of toJs_impl.
template <typename T>
auto toJs(const Napi::Env& env, const T& val) {
    return toJs_impl<T>{}(env, val);
}

template <>
struct toJs_impl<bool> {
    auto operator()(const Napi::Env& env, bool b) const { return Napi::Boolean::New(env, b); }
};

template <>
struct toJs_impl<session::config::Namespace> {
    auto operator()(const Napi::Env& env, session::config::Namespace b) const {
        return Napi::Number::New(env, static_cast<int16_t>(b));
    }
};

template <>
struct toJs_impl<size_t> {
    auto operator()(const Napi::Env& env, size_t b) const { return Napi::Number::New(env, (b)); }
};

template <typename T>
struct toJs_impl<T, std::enable_if_t<std::is_arithmetic_v<T>>> {
    auto operator()(const Napi::Env& env, T n) const { return Napi::Number::New(env, n); }
};

template <typename T>
struct toJs_impl<T, std::enable_if_t<std::is_convertible_v<T, std::string_view>>> {
    auto operator()(const Napi::Env& env, std::string_view s) const {
        return Napi::String::New(env, s.data(), s.size());
    }
};

template <>
struct toJs_impl<string8> {
    auto operator()(const Napi::Env& env, string8 s) const {
        return Napi::String::New(env, s.data, s.size);
    }
};

template <typename T>
struct toJs_impl<
        T,
        std::enable_if_t<
                std::is_convertible_v<T, std::span<const unsigned char>> &&
                !std::is_same_v<std::remove_cv_t<T>, std::vector<unsigned char>>>> {
    auto operator()(const Napi::Env& env, std::span<const unsigned char> b) const {
        return Napi::Buffer<uint8_t>::Copy(env, b.data(), b.size());
    }
};

// this wrap std::vector<unsigned char> to Uint8array in the js world
template <>
struct toJs_impl<std::vector<unsigned char>> {
    auto operator()(const Napi::Env& env, std::vector<unsigned char> b) const {
        return Napi::Buffer<uint8_t>::Copy(env, b.data(), b.size());
    }
};

// this wrap std::vector<std::byte> to Uint8array in the js world
template <>
struct toJs_impl<std::vector<std::byte>> {
    auto operator()(const Napi::Env& env, std::vector<std::byte> b) const {
        return Napi::Buffer<uint8_t>::Copy(
                env, reinterpret_cast<const unsigned char*>(b.data()), b.size());
    }
};

// this wrap std::array<std::byte> to Uint8array in the js world
template <std::size_t N>
struct toJs_impl<std::array<std::byte, N>> {
    auto operator()(const Napi::Env& env, const std::array<std::byte, N>& b) const {
        const auto* data_uchar = reinterpret_cast<const uint8_t*>(b.data());
        return Napi::Buffer<uint8_t>::Copy(env, data_uchar, b.size());
    }
};

// this wrap std::span<std::byte> to Uint8array in the js world
template <>
struct toJs_impl<std::span<std::byte>> {
    auto operator()(const Napi::Env& env, std::span<std::byte> b) const {
        auto data = b.data();
        auto data_uchar = reinterpret_cast<const unsigned char*>(data, b.size());

        return Napi::Buffer<uint8_t>::Copy(env, data_uchar, b.size());
    }
};

template <typename T>
struct toJs_impl<T, std::enable_if_t<std::is_base_of_v<Napi::Value, T>>> {
    auto operator()(const Napi::Env& env, const T& val) { return val; }
};

template <typename T>
struct toJs_impl<std::vector<T>> {
    auto operator()(const Napi::Env& env, const std::vector<T>& val) {
        auto arr = Napi::Array::New(env, val.size());
        for (size_t i = 0; i < val.size(); i++)
            arr[i] = toJs(env, val[i]);
        return arr;
    }
};

template <typename T>
struct toJs_impl<std::unordered_set<T>> {
    auto operator()(const Napi::Env& env, const std::unordered_set<T>& set) {
        std::vector<T> as_array(set.begin(), set.end());

        auto arr = Napi::Array::New(env, as_array.size());
        for (size_t i = 0; i < as_array.size(); i++)
            arr[i] = toJs(env, as_array[i]);
        return arr;
    }
};

template <typename T>
struct toJs_impl<std::optional<T>> {
    Napi::Value operator()(const Napi::Env& env, const std::optional<T>& val) {
        if (val)
            return toJs(env, *val);
        return env.Null();
    }
};

template <>
struct toJs_impl<std::chrono::sys_seconds> {
    auto operator()(const Napi::Env& env, std::chrono::sys_seconds t) const {
        return Napi::Number::New(env, t.time_since_epoch().count());
    }
};

template <>
struct toJs_impl<std::chrono::milliseconds> {
    auto operator()(const Napi::Env& env, std::chrono::milliseconds t) const {
        return Napi::Number::New(env, t.count());
    }
};

template <>
struct toJs_impl<std::chrono::sys_time<std::chrono::milliseconds>> {
    auto operator()(
            const Napi::Env& env, std::chrono::sys_time<std::chrono::milliseconds> t) const {
        return Napi::Number::New(env, t.time_since_epoch().count());
    }
};

// Returns {"url": "...", "key": buffer} object; both values will be Null if the pic is not set.

template <>
struct toJs_impl<config::profile_pic> {
    auto operator()(const Napi::Env& env, const config::profile_pic& pic) const {
        auto obj = Napi::Object::New(env);
        if (pic) {
            obj["url"] = toJs(env, pic.url);
            obj["key"] = toJs(env, pic.key);
        } else {
            obj["url"] = env.Null();
            obj["key"] = env.Null();
        }
        return obj;
    }
};

// Helper for various "get_all" functions that copy [it...end) into a Napi::Array via toJs().
// Throws a Napi::Error on any exception.
template <typename It, typename EndIt>
static Napi::Array get_all_impl(const Napi::CallbackInfo& info, size_t size, It it, EndIt end) {
    auto env = info.Env();
    return wrapResult(env, [&] {
        assertInfoLength(info, 0);
        auto result = Napi::Array::New(env, size);
        int i = 0;
        for (; it != end; it++)
            result[i++] = toJs(env, *it);

        return result;
    });
}

// Wraps a string in an optional<string_view> which will be nullopt if the input string is
// empty. This is particularly useful with `toJs` to convert empty strings into Null.
inline std::optional<std::string_view> maybe_string(std::string_view val) {
    if (val.empty())
        return std::nullopt;
    return val;
}

// Calls the given callable and wraps it to be suitable for a Napi call:
// - Napi::Error exceptions will be uncaught
// - Other std::exceptions will be caught, converted to Napi::Errors, and rethrown
// - The return value will be returned as-is if it is already a Napi::Value (or subtype)
// - The return will be void if void
// - Otherwise the return value will be passed through toJs() to convert it to a Napi::Value.
// See toJs below, but generally this supports numeric types, bools, strings, spans, and vectors
// of any of those primitives.
//
// General use is:
//
//     return wrapResult(env, [&] { return foo(); });
//
template <typename Call>
auto wrapResult(const Napi::Env& env, Call&& call) {
    using Result = decltype(call());
    try {
        if constexpr (std::is_void_v<Result>) {
            call();
        } else {
            auto res = call();
            if constexpr (std::is_base_of_v<Napi::Value, Result>)
                return res;
            else
                return toJs<Result>(env, std::move(res));
        }
    } catch (const std::exception& e) {
        throw Napi::Error::New(env, e.what());
    }
}

// Similar to wrapResult(), but a small shortcut to allow passing `info` instead of `info.Env()`
// as the first argument.
template <typename Call>
auto wrapResult(const Napi::CallbackInfo& info, Call&& call) {
    return wrapResult(info.Env(), std::forward<Call>(call));
}

// Similar to wrapResult(), but this only applies the exception wrapping (i.e. no wrapping of
// the result: we return it exactly as-is).
template <typename Call>
auto wrapExceptions(const Napi::Env& env, Call&& call) {
    try {
        return call();
    } catch (const std::exception& e) {
        throw Napi::Error::New(env, e.what());
    }
}
template <typename Call>
auto wrapExceptions(const Napi::CallbackInfo& info, Call&& call) {
    return wrapExceptions(info.Env(), std::forward<Call>(call));
}

std::string printable(std::string_view x);
std::string printable(const char* x) = delete;
std::string printable(const char* x, size_t n);
std::string printable(std::span<const unsigned char> x);

/**
 * Keep the current priority if a wrapper
 */
int64_t toPriority(Napi::Value x, int64_t currentPriority);
int64_t toPriority(int64_t newPriority, int64_t currentPriority);

std::optional<session::config::profile_pic> maybeNonemptyProfilePic(
        Napi::Value x, const std::string& identifier);

int64_t unix_timestamp_now();

using push_entry_t = std::tuple<
        session::config::seqno_t,
        std::vector<std::vector<unsigned char>>,
        std::vector<std::string>>;

using confirm_pushed_entry_t =
        std::tuple<session::config::seqno_t, std::unordered_set<std::string>>;

Napi::Object push_result_to_JS(
        const Napi::Env& env,
        const push_entry_t& push_entry,
        const session::config::Namespace& push_namespace);

Napi::Object push_key_entry_to_JS(
        const Napi::Env& env,
        const std::span<const unsigned char>& key_data,
        const session::config::Namespace& push_namespace);

Napi::Object decrypt_result_to_JS(
        const Napi::Env& env, const std::pair<std::string, std::vector<unsigned char>> decrypted);

confirm_pushed_entry_t confirm_pushed_entry_from_JS(const Napi::Env& env, const Napi::Object& obj);

Napi::BigInt proProfileBitsetToJS(const Napi::Env& env, const ProProfileBitset bitset);

Napi::BigInt proMessageBitsetToJS(const Napi::Env& env, const ProMessageBitset bitset);

std::span<const uint8_t> from_hex_to_span(std::string_view x);

template <std::size_t N>
std::array<uint8_t, N> spanToArray(std::span<const unsigned char> span);

template <std::size_t N>
std::array<uint8_t, N> from_hex_to_array(std::string x) {
    std::string as_hex = oxenc::from_hex(x);
    if (as_hex.size() != N) {
        throw std::invalid_argument(fmt::format(
                "from_hex_to_array: Decoded hex size mismatch: expected {}, got {}",
                N,
                as_hex.size()));
    }

    std::array<uint8_t, N> result;
    std::memcpy(result.data(), as_hex.data(), N);
    return result;
}

template <std::size_t N>
std::array<uint8_t, N> from_base64_to_array(std::string x) {
    std::string as_b64 = oxenc::from_base64(x);
    if (as_b64.size() != N) {
        throw std::invalid_argument(fmt::format(
                "from_base64_to_array: Decoded v64 size mismatch: expected {}, got {}",
                N,
                as_b64.size()));
    }

    std::array<uint8_t, N> result;
    std::memcpy(result.data(), as_b64.data(), N);
    return result;
}

std::vector<unsigned char> from_hex_to_vector(std::string_view x);

std::span<const uint8_t> from_base64_to_span(std::string_view x);
std::vector<unsigned char> from_base64_to_vector(std::string_view x);

// Concept to match containers with a size() method
template <typename T>
concept HasSize = requires(T t) {
    {
        t.size()
    } -> std::convertible_to<size_t>;
};

template <HasSize T>
void assert_length(const T& x, size_t n, std::string_view base_identifier) {
    if (x.size() != n) {
        throw std::invalid_argument(fmt::format(
                "assert_length: expected {}, got {} for {}", n, x.size(), base_identifier));
    }
}

}  // namespace session::nodeapi
