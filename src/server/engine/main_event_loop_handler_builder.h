#ifndef SERVER_ENGINE_MAIN_EVENT_LOOP_HANDLER_BUILDER_H
#define SERVER_ENGINE_MAIN_EVENT_LOOP_HANDLER_BUILDER_H

#include "core/core.h"

#include "event_loop_handler.h"

namespace engine {

class MainEventLoopHandlerBuilder final {
public:
  explicit MainEventLoopHandlerBuilder(
      std::string &&primary_event_loop_name) noexcept;
  ~MainEventLoopHandlerBuilder() noexcept = default;
  CLASS_KIND_MOVABLE(MainEventLoopHandlerBuilder);

  [[nodiscard]] auto Build() noexcept -> Result<EventLoopHandlerPtr>;

private:
  std::string primary_event_loop_name_;
};

} // namespace engine

#endif // SERVER_ENGINE_MAIN_EVENT_LOOP_HANDLER_BUILDER_H
