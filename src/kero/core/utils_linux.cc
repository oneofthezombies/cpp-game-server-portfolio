#include "utils_linux.h"

#include <fcntl.h>
#include <unistd.h>

#include <cstring>

#include "kero/core/dict.h"
#include "kero/core/utils.h"

using namespace kero;

auto
kero::Fd::IsValid(const Value fd) noexcept -> bool {
  return fd >= 0;
}

auto
kero::Fd::Close(const Value fd) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (!IsValid(fd)) {
    return ResultT::Err(Error::From(kInvalidValue));
  }

  if (close(fd) == -1) {
    return ResultT::Err(Error::From(Errno::FromErrno().IntoDict()));
  }

  return OkVoid();
}

auto
kero::Fd::UpdateNonBlocking(const Value fd) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  const int opts = fcntl(fd, F_GETFL);
  if (opts < 0) {
    return ResultT{
        Error::From(kGetStatusFailed, Errno::FromErrno().IntoDict())};
  }

  if (fcntl(fd, F_SETFL, (opts | O_NONBLOCK)) < 0) {
    return ResultT{
        Error::From(kSetStatusFailed, Errno::FromErrno().IntoDict())};
  }

  return OkVoid();
}

kero::Errno::Errno(const Value code,
                   const std::string_view description) noexcept
    : code{code}, description{description} {}

auto
kero::Errno::FromErrno() noexcept -> Errno {
  const auto code = errno;
  return Errno{code, std::string_view{strerror(code)}};
}

auto
kero::Errno::IntoDict() const noexcept -> Dict {
  return Dict{}
      .Set("error_kind", std::string{"errno"})
      .Set("errno_code", static_cast<double>(code))
      .Set("errno_description", std::string{description})
      .Take();
}

auto
kero::operator<<(std::ostream& os, const Errno& err) -> std::ostream& {
  return os << "Errno{code: " << err.code
            << ", description: " << err.description << "}";
}