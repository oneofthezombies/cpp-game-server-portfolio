#include "mail_center.h"

#include <iostream>
#include <mutex>

#include "common.h"
#include "spsc_channel.h"

MailBox::MailBox(Tx<Mail> &&tx, Rx<Mail> &&rx) noexcept
    : tx(std::move(tx)), rx(std::move(rx)) {}

MailCenter::MailCenter(Tx<MessageBody> &&run_tx) noexcept
    : run_tx_{std::move(run_tx)} {}

auto MailCenter::Shutdown() noexcept -> void {
  run_tx_.Send({{"shutdown", ""}});
  run_thread_.join();
}

auto MailCenter::Create(const std::string_view name) noexcept
    -> Result<MailBox> {
  using ResultT = Result<MailBox>;

  std::string name_str{name};
  {
    std::lock_guard lock{mutex_};
    if (const auto it = mail_boxes_.find(name_str); it != mail_boxes_.end()) {
      return ResultT{Error{Symbol::kMailBoxAlreadyExists,
                           SB{}.Add("name", name_str).Build()}};
    }

    auto [from_peer_tx, to_office_rx] = Channel<Mail>::Builder{}.Build();
    auto [from_office_tx, to_peer_rx] = Channel<Mail>::Builder{}.Build();

    mail_boxes_.emplace(std::string{name},
                        MailBox{Tx<Mail>{std::move(from_office_tx)},
                                Rx<Mail>{std::move(to_office_rx)}});

    return ResultT{MailBox{
        Tx<Mail>{std::move(from_peer_tx)},
        Rx<Mail>{std::move(to_peer_rx)},
    }};
  }
}

auto MailCenter::Delete(const std::string_view name) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  std::string name_str{name};
  {
    std::lock_guard lock{mutex_};
    if (const auto it = mail_boxes_.find(name_str); it == mail_boxes_.end()) {
      return ResultT{
          Error{Symbol::kMailBoxNotFound, SB{}.Add("name", name_str).Build()}};
    }

    mail_boxes_.erase(name_str);
  }
  return ResultT{Void{}};
}

auto MailCenter::RunOnThread(Rx<MessageBody> &&run_rx) noexcept -> void {
  while (true) {
    auto run_event = run_rx.TryReceive();
    if (run_event) {
      if (auto shutdown = run_event->find("shutdown");
          shutdown != run_event->end()) {
        break;
      }
    }

    {
      std::lock_guard lock{mutex_};
      for (auto &[name, mail_box] : mail_boxes_) {
        auto mail = mail_box.rx.TryReceive();
        if (!mail) {
          continue;
        }

        const auto to = mail_boxes_.find(mail->to);
        if (to == mail_boxes_.end()) {
          std::cout << "MailBox not found: " << mail->to << std::endl;
          continue;
        }

        to->second.tx.Send(std::move(*mail));
      }
    }
  }
}

auto MailCenter::StartRunThread(Rx<MessageBody> &&run_rx) noexcept -> void {
  run_thread_ = std::thread{RunThreadMain, std::ref(*this), std::move(run_rx)};
}

auto MailCenter::RunThreadMain(MailCenter &mail_center,
                               Rx<MessageBody> &&run_rx) noexcept -> void {
  mail_center.RunOnThread(std::move(run_rx));
}

auto MailCenter::Global() noexcept -> MailCenter & {
  static std::unique_ptr<MailCenter> instance{nullptr};
  static std::once_flag flag;
  std::call_once(flag, [] {
    auto [tx, rx] = Channel<MessageBody>::Builder{}.Build();
    instance.reset(new MailCenter{std::move(tx)});
    instance->StartRunThread(std::move(rx));
  });
  return *instance;
}
