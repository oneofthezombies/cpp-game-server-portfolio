#include "config_service.h"

#include "kero/core/args_scanner.h"
#include "kero/core/utils.h"
#include "kero/log/log_builder.h"

using namespace kero;

kero::ConfigService::ConfigService(const Borrow<RunnerContext> runner_context,
                                   FlatJson&& config) noexcept
    : Service{runner_context, {}}, config_{std::move(config)} {}

auto
kero::ConfigService::OnCreate() noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  log::Debug("ConfigService created").Data("config", config_).Log();

  return OkVoid();
}

auto
kero::ConfigService::GetConfig() const noexcept -> const FlatJson& {
  return config_;
}

auto
kero::ConfigService::GetConfig() noexcept -> FlatJson& {
  return config_;
}

kero::ConfigServiceFactory::ConfigServiceFactory(int argc, char** argv) noexcept
    : args_{Args{argv, argv + argc}} {}

auto
kero::ConfigServiceFactory::Create(
    const Borrow<RunnerContext> runner_context) noexcept
    -> Result<Own<Service>> {
  using ResultT = Result<Own<Service>>;

  FlatJson config{};
  ArgsScanner scanner{args_};

  // Skip the first argument which is the program name
  scanner.Eat();
  while (true) {
    const auto current = scanner.Current();
    if (!current) {
      break;
    }

    const auto& token = current.Unwrap();
    if (token == "--port") {
      const auto next = scanner.Next();
      if (!next) {
        return ResultT::Err(Error::From(kPortNotFound));
      }

      auto port_str = next.Unwrap();
      auto res = ParseNumberString<u16>(port_str);
      if (res.IsErr()) {
        return ResultT::Err(
            Error::From(kPortParsingFailed,
                        FlatJson{}.Set("port", std::move(port_str)).Take(),
                        res.TakeErr()));
      };

      (void)config.Set("port", res.TakeOk());
      scanner.Eat();
    } else {
      return ResultT::Err(
          Error::From(kUnknownArgument,
                      FlatJson{}.Set("token", std::string{token}).Take()));
    }

    scanner.Eat();
  }

  return ResultT::Ok(
      std::make_unique<ConfigService>(runner_context, std::move(config)));
}
