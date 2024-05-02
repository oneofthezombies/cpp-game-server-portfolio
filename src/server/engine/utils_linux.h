#ifndef SERVER_ENGINE_UTILS_LINUX_H
#define SERVER_ENGINE_UTILS_LINUX_H

#include "event_loop.h"

namespace engine {

[[nodiscard]] auto
EventLoopAddOptionsToEpollEvents(const EventLoopAddOptions &options) noexcept
    -> u32;

}  // namespace engine

#endif  // SERVER_ENGINE_UTILS_LINUX_H
