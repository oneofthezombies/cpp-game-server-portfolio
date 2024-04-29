#include "config_service.h"

#include "kero/core/args_scanner.h"
#include "kero/core/utils.h"
#include "kero/engine/constants.h"
#include "kero/log/log_builder.h"

using namespace kero;

kero::ConfigService::ConfigService(Dict&& config) noexcept
    : Service{ServiceKind::kConfig, {}}, config_{std::move(config)} {}

auto
kero::ConfigService::OnCreate(Agent& agent) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  log::Debug("ConfigService created").Data("config", config_).Log();

  return ResultT::Ok(Void{});
}

auto
kero::ConfigService::OnDestroy(Agent& agent) noexcept -> void {}

auto
kero::ConfigService::OnUpdate(Agent& agent) noexcept -> void {}

auto
kero::ConfigService::GetConfig() const noexcept -> const Dict& {
  return config_;
}

auto
kero::ConfigService::GetConfig() noexcept -> Dict& {
  return config_;
}

kero::ConfigServiceFactory::ConfigServiceFactory(int argc, char** argv) noexcept
    : args_{Args{argv, argv + argc}} {}

auto
kero::ConfigServiceFactory::Create() noexcept -> Result<ServicePtr> {
  using ResultT = Result<ServicePtr>;

  Dict config{};
  ArgsScanner scanner{std::move(args_)};

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
      auto res = ParseNumberString<uint16_t>(port_str);
      if (res.IsErr()) {
        return ResultT::Err(
            Error::From(kPortParsingFailed,
                        Dict{}.Set("port", std::move(port_str)).Take(),
                        res.TakeErr()));
      };

      (void)config.Set("port", res.TakeOk());
      scanner.Eat();
    } else {
      return ResultT::Err(
          Error::From(kUnknownArgument,
                      Dict{}.Set("token", std::string{token}).Take()));
    }

    scanner.Eat();
  }

  return ServicePtr{new ConfigService{std::move(config)}};
}
