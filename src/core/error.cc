#include "error.h"

using namespace core;

core::Error::Error(const ErrorCode code, std::string &&message) noexcept
    : code(code), message(std::move(message)) {}
