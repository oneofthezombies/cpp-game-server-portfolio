#include "config_component.h"

#include <type_traits>

#include "kero/core/args_scanner.h"
#include "kero/core/utils.h"
#include "kero/engine/constants.h"

using namespace kero;

kero::ConfigComponent::ConfigComponent(Dict&& config) noexcept
    : Component{ComponentKind::kConfig}, config_{std::move(config)} {}

auto
kero::ConfigComponent::OnCreate(Agent& agent) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  return ResultT::Ok(Void{});
}

auto
kero::ConfigComponent::OnDestroy(Agent& agent) noexcept -> void {}

auto
kero::ConfigComponent::OnUpdate(Agent& agent) noexcept -> void {}

auto
kero::ConfigComponent::GetConfig() const noexcept -> const Dict& {
  return config_;
}

auto
kero::ConfigComponent::FromArgs(int argc, char** argv) noexcept
    -> Result<ComponentPtr> {
  using ResultT = Result<ComponentPtr>;

  Dict config{};
  ArgsScanner scanner{ArgsScanner::FromArgs(argc, argv)};

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
  }

  return ComponentPtr{new ConfigComponent{std::move(config)}};
}
