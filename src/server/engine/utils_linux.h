#ifndef SERVER_ENGINE_UTILS_LINUX_H
#define SERVER_ENGINE_UTILS_LINUX_H

#include "event_loop.h"

namespace engine {

[[nodiscard]] auto
EventLoopAddOptionsToEpollEvents(const EventLoopAddOptions &options) noexcept
    -> uint32_t;

}  // namespace engine

#endif  // SERVER_ENGINE_UTILS_LINUX_H
