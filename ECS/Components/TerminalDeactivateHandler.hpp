#pragma once

#include "../HandlerComponent.hpp"

/* Trigger a callback on deactivation of the main terminal */
struct TerminalDeactivateHandler : HandlerComponent<TerminalDeactivateHandler, void> {
    using HandlerComponent<TerminalDeactivateHandler, void>::HandlerComponent;
};
