#include "utils_linux.h"

#include <sys/epoll.h>

using namespace engine;

auto
engine::EventLoopAddOptionsToEpollEvents(
    const EventLoopAddOptions &options) noexcept -> u32 {
  u32 events{0};

  if (options.in) {
    events |= EPOLLIN;
  }

  if (options.out) {
    events |= EPOLLOUT;
  }

  if (options.edge_trigger) {
    events |= EPOLLET;
  }

  return events;
}