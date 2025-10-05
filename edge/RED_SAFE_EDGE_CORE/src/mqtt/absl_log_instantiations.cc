#ifdef __APPLE__
#include "absl/log/internal/log_message.h"
// Ensure the Abseil log message stream overloads for unsigned integral types
// are emitted even when the toolchain misses Abseil's explicit instantiations.
// This prevents unresolved references to the templated operator<< symbols
// during the final link step on certain macOS + LLVM combinations.
namespace absl {
ABSL_NAMESPACE_BEGIN
namespace log_internal {

template LogMessage& LogMessage::operator<<<unsigned long>(
    const unsigned long&);
template LogMessage& LogMessage::operator<<<unsigned long long>(
    const unsigned long long&);

}  // namespace log_internal
ABSL_NAMESPACE_END
}  // namespace absl
#endif
