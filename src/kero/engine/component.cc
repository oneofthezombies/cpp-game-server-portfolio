#include "component.h"

using namespace kero;

kero::Component::Component(std::string&& name) noexcept
    : name_{std::move(name)} {}
