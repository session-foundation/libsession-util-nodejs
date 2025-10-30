#include "pro/pro.hpp"

namespace session::nodeapi {
std::string_view ProBackendEnumToString(SESSION_PRO_BACKEND_PAYMENT_PROVIDER v) {
    switch (v) {
        case SESSION_PRO_BACKEND_PAYMENT_PROVIDER_NIL: return "NIL";
        case SESSION_PRO_BACKEND_PAYMENT_PROVIDER_GOOGLE_PLAY_STORE: return "GOOGLE_PLAY_STORE";
        case SESSION_PRO_BACKEND_PAYMENT_PROVIDER_IOS_APP_STORE: return "IOS_APP_STORE";
        case SESSION_PRO_BACKEND_PAYMENT_PROVIDER_COUNT:
            throw std::invalid_argument("SESSION_PRO_BACKEND_PAYMENT_PROVIDER_COUNT");
    }
    __builtin_unreachable();
}

std::string_view ProBackendEnumToString(SESSION_PRO_BACKEND_PAYMENT_STATUS v) {
    switch (v) {
        case SESSION_PRO_BACKEND_PAYMENT_STATUS_NIL: return "NIL";
        case SESSION_PRO_BACKEND_PAYMENT_STATUS_UNREDEEMED: return "UNREDEEMED";
        case SESSION_PRO_BACKEND_PAYMENT_STATUS_REDEEMED: return "REDEEMED";
        case SESSION_PRO_BACKEND_PAYMENT_STATUS_EXPIRED: return "EXPIRED";
        case SESSION_PRO_BACKEND_PAYMENT_STATUS_REFUNDED: return "REFUNDED";
        case SESSION_PRO_BACKEND_PAYMENT_STATUS_COUNT:
            throw std::invalid_argument("SESSION_PRO_BACKEND_PAYMENT_STATUS_COUNT");
    }
    __builtin_unreachable();
}

std::string_view proBackendEnumToString(SESSION_PRO_BACKEND_PLAN v) {
    switch (v) {
        case SESSION_PRO_BACKEND_PLAN_NIL: return "NIL";
        case SESSION_PRO_BACKEND_PLAN_ONE_MONTH: return "ONE_MONTH";
        case SESSION_PRO_BACKEND_PLAN_THREE_MONTHS: return "THREE_MONTHS";
        case SESSION_PRO_BACKEND_PLAN_TWELVE_MONTHS: return "TWELVE_MONTHS";
        case SESSION_PRO_BACKEND_PLAN_COUNT:
            throw std::invalid_argument("SESSION_PRO_BACKEND_PLAN_COUNT");
    }
    __builtin_unreachable();
}

std::string_view proBackendEnumToString(SESSION_PRO_BACKEND_USER_PRO_STATUS v) {
    switch (v) {
        case SESSION_PRO_BACKEND_USER_PRO_STATUS_NEVER_BEEN_PRO: return "NEVER_BEEN_PRO";
        case SESSION_PRO_BACKEND_USER_PRO_STATUS_ACTIVE: return "ACTIVE";
        case SESSION_PRO_BACKEND_USER_PRO_STATUS_EXPIRED: return "EXPIRED";
        case SESSION_PRO_BACKEND_USER_PRO_STATUS_COUNT:
            throw std::invalid_argument("SESSION_PRO_BACKEND_USER_PRO_STATUS_COUNT");
    }
    __builtin_unreachable();
}

std::string_view ProBackendEnumToString(SESSION_PRO_BACKEND_GET_PRO_STATUS_ERROR_REPORT v) {
    switch (v) {
        case SESSION_PRO_BACKEND_GET_PRO_STATUS_ERROR_REPORT_SUCCESS: return "SUCCESS";
        case SESSION_PRO_BACKEND_GET_PRO_STATUS_ERROR_REPORT_GENERIC_ERROR: return "GENERIC_ERROR";
        case SESSION_PRO_BACKEND_GET_PRO_STATUS_ERROR_REPORT_COUNT:
            throw std::invalid_argument("SESSION_PRO_BACKEND_GET_PRO_STATUS_ERROR_REPORT_COUNT");
    }
    __builtin_unreachable();
}

std::string_view proBackendEnumPlanToString(SESSION_PRO_BACKEND_PLAN v) {
    switch (v) {
        case SESSION_PRO_BACKEND_PLAN_NIL: return "NIL";
        case SESSION_PRO_BACKEND_PLAN_ONE_MONTH: return "ONE_MONTH";
        case SESSION_PRO_BACKEND_PLAN_THREE_MONTHS: return "THREE_MONTHS";
        case SESSION_PRO_BACKEND_PLAN_TWELVE_MONTHS: return "TWELVE_MONTHS";
        case SESSION_PRO_BACKEND_PLAN_COUNT:
            throw std::invalid_argument("SESSION_PRO_BACKEND_PLAN_COUNT");
    }
    __builtin_unreachable();
}

std::string_view proBackendEnumPaymentProviderToString(SESSION_PRO_BACKEND_PAYMENT_PROVIDER v) {
    switch (v) {
        // Note: we want those to map ProOriginatingPlatform keys
        case SESSION_PRO_BACKEND_PAYMENT_PROVIDER_NIL: return "Nil";
        case SESSION_PRO_BACKEND_PAYMENT_PROVIDER_GOOGLE_PLAY_STORE: return "GooglePlayStore";
        case SESSION_PRO_BACKEND_PAYMENT_PROVIDER_IOS_APP_STORE: return "iOSAppStore";
        case SESSION_PRO_BACKEND_PAYMENT_PROVIDER_COUNT:
            throw std::invalid_argument("SESSION_PRO_BACKEND_PAYMENT_PROVIDER_COUNT");
    }
    __builtin_unreachable();
}

std::string_view proBackendEnumPaymentStatusToString(SESSION_PRO_BACKEND_PAYMENT_STATUS v) {
    switch (v) {
        case SESSION_PRO_BACKEND_PAYMENT_STATUS_NIL: return "NIL";
        case SESSION_PRO_BACKEND_PAYMENT_STATUS_UNREDEEMED: return "UNREDEEMED";
        case SESSION_PRO_BACKEND_PAYMENT_STATUS_REDEEMED: return "REDEEMED";
        case SESSION_PRO_BACKEND_PAYMENT_STATUS_EXPIRED: return "EXPIRED";
        case SESSION_PRO_BACKEND_PAYMENT_STATUS_REFUNDED: return "REFUNDED";
        case SESSION_PRO_BACKEND_PAYMENT_STATUS_COUNT:
            throw std::invalid_argument("SESSION_PRO_BACKEND_PAYMENT_STATUS_COUNT");
    }
    __builtin_unreachable();
}

}  // namespace session::nodeapi
