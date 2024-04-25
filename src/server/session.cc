#include "session.h"

#include <cassert>
#include <iostream>

#if defined(__linux__)
#include "file_descriptor_linux.h"
using SessionImpl = FileDescriptorLinux;
#elif defined(_WIN32)
#error "Not implemented"
#elif defined(__APPLE__)
#error "Not implemented"
#else
#error "Unsupported platform"
#endif

auto CastImpl(void *impl_raw) noexcept -> SessionImpl * {
  assert(impl_raw != nullptr && "impl must not be nullptr");
  return reinterpret_cast<SessionImpl *>(impl_raw);
}

auto SessionImplDeleter::operator()(void *impl_raw) const noexcept -> void {
  if (impl_raw == nullptr) {
    return;
  }

  delete CastImpl(impl_raw);
}

Session::Session(SessionImplPtr &&impl) noexcept : impl_{std::move(impl)} {
  assert(impl_.get() != nullptr && "impl must not be nullptr");
}

auto Session::Id() const noexcept -> IdType {
  assert(impl_.get() != nullptr && "impl must not be nullptr");
  return CastImpl(impl_.get())->AsRaw();
}

auto Session::Hash() const noexcept -> size_t {
  assert(impl_.get() != nullptr && "impl must not be nullptr");
  return std::hash<IdType>{}(Id());
}

auto operator==(const Session &lhs, const Session &rhs) noexcept -> bool {
  return lhs.Hash() == rhs.Hash();
}

auto operator<<(std::ostream &os, const Session &session) -> std::ostream & {
  os << "Session{";
  os << "id=";
  os << session.Id();
  os << "}";
  return os;
}
