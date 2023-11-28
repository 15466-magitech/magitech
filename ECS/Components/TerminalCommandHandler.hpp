#pragma once

#include "../HandlerComponent.hpp"

/*
 * The possible commands that can be entered into the terminal
 */
enum struct Command {
    OpenSesame,
    Mirage
};

struct TerminalCommandHandler : HandlerComponent<TerminalCommandHandler, void, Command> {
    using HandlerComponent<TerminalCommandHandler, void, Command>::HandlerComponent;
};
