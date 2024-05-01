#include "log_builder.h"

#include "kero/log/core.h"
#include "kero/log/local_context.h"

using namespace kero;

kero::log::LogBuilder::LogBuilder(std::string&& message,
                                  std::source_location&& location,
                                  const Level level) noexcept
    : log_{std::make_unique<kero::Log>(
          std::move(message), std::move(location), level)} {}

auto
kero::log::LogBuilder::Log() noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (!log_) {
    return ResultT::Err(Error::From(
        Dict{}
            .Set("message", std::string{"Log already consumed, cannot log."})
            .Take()));
  }

  if (auto& local_context = GetLocalContext()) {
    local_context->SendLog(std::move(log_));
    return OkVoid;
  } else {
    return ResultT::Err(Error::From(
        Dict{}
            .Set("message",
                 std::string{"Failed to get LocalContext, cannot log."})
            .Take()));
  }
}

auto
kero::log::Debug(std::string&& message, std::source_location&& location)
    -> LogBuilder {
  return LogBuilder{std::move(message), std::move(location), Level::kDebug};
}

auto
kero::log::Info(std::string&& message, std::source_location&& location)
    -> LogBuilder {
  return LogBuilder{std::move(message), std::move(location), Level::kInfo};
}

auto
kero::log::Warn(std::string&& message, std::source_location&& location)
    -> LogBuilder {
  return LogBuilder{std::move(message), std::move(location), Level::kWarn};
}

auto
kero::log::Error(std::string&& message, std::source_location&& location)
    -> LogBuilder {
  return LogBuilder{std::move(message), std::move(location), Level::kError};
}
