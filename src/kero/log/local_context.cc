#include "local_context.h"

#include <memory>
#include <sstream>

#include "kero/log/global_context.h"

using namespace kero;

auto
kero::LocalContext::Builder::Build() const noexcept
    -> Result<std::unique_ptr<LocalContext>> {
  using ResultT = Result<std::unique_ptr<LocalContext>>;

  auto thread_id = ThreadIdToString(std::this_thread::get_id());
  auto [log_tx, log_rx] =
      spsc::Channel<std::unique_ptr<kero::Log>>::Builder{}.Build();
  if (!GetGlobalContext().AddLogRx(thread_id, std::move(log_rx))) {
    return ResultT::Err(Error::From(
        Dict{}
            .Set("message",
                 std::string{"Failed to add log rx, thread_id already exists."})
            .Take()));
  }

  return ResultT{std::unique_ptr<LocalContext>{
      new LocalContext{std::move(log_tx), std::move(thread_id)}}};
}

kero::LocalContext::LocalContext(spsc::Tx<std::unique_ptr<kero::Log>>&& log_tx,
                                 std::string&& thread_id) noexcept
    : log_tx_{std::move(log_tx)}, thread_id_{std::move(thread_id)} {}

kero::LocalContext::~LocalContext() noexcept {
  if (!GetGlobalContext().RemoveLogRx(thread_id_)) {
    std::stringstream ss{};
    ss << "Failed to remove log rx, thread_id not found: " << thread_id_;
    GetGlobalContext().LogSystemError(ss.str());
  }
}

auto
kero::LocalContext::SendLog(std::unique_ptr<kero::Log>&& log) const noexcept
    -> void {
  log_tx_.Send(std::move(log));
}

auto
kero::GetLocalContext() -> std::unique_ptr<LocalContext>& {
  thread_local std::unique_ptr<LocalContext> local_context{nullptr};
  thread_local std::once_flag flag{};

  std::call_once(flag, [&]() {
    auto res = LocalContext::Builder{}.Build();
    if (res.IsOk()) {
      local_context = res.TakeOk();
    } else {
      std::stringstream ss{};
      ss << "Failed to build LocalContext: " << res.Err();
      GetGlobalContext().LogSystemError(ss.str());
    }
  });

  return local_context;
}
