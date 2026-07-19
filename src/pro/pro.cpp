#include "pro/pro.hpp"

namespace session::nodeapi {

// Provider, plan and payment-status are no longer fixed enums on the wire: provider/plan are opaque
// strings (emitted verbatim) and the response structs expose payment/user status as small enums that
// we surface to JS as their raw integer values (they line up 1:1 with the desktop numeric enums), so
// the only enum->string mapping still needed is for the pro-features result status.
std::string_view proBackendEnumToString(session::ProFeaturesForMsgStatus v) {
    switch (v) {
        case session::ProFeaturesForMsgStatus::Success: return "SUCCESS";
        case session::ProFeaturesForMsgStatus::UTFDecodingError: return "UTF_DECODING_ERROR";
        case session::ProFeaturesForMsgStatus::ExceedsCharacterLimit:
            return "EXCEEDS_CHARACTER_LIMIT";
    }
    UNREACHABLE();
}

}  // namespace session::nodeapi
