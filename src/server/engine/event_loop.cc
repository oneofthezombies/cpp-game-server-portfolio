#include "event_loop.h"

using namespace engine;

engine::EventLoopContext::EventLoopContext(MailBox &&mail_box,
                                           std::string &&name) noexcept
    : mail_box(std::move(mail_box)), name(std::move(name)) {}

engine::EventLoop::EventLoop(EventLoopContext &&context,
                             EventLoopHandlerPtr &&handler) noexcept
    : context_(std::move(context)), handler_(std::move(handler)) {}
