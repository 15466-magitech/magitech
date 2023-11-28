#pragma once

#include "Component.hpp"

/*
 * It seems that many components consist mainly of a single callback.
 * HandlerComponent is a struct to generate this sort of component.
 * To use it, inherit it with the component's name as T the callback's
 * return type as R, and the callback's arguments as Args.
 * Then, inherit the constructor with `using` syntax, as demonstrated
 * with TestHandler in HandlerComponent.cpp.
 * (Constructors are not automatically inherited.)
 *
 * There is no good way to generally collect the results of multiple callbacks,
 * so most inheriting structs will need to make their own system methods.
 * However, HandlerComponent does have `handle_all`, which runs all handlers on
 * the same input and throws away the output.
 */
template<typename T, typename R, typename... Args>
struct HandlerComponent : Component<T> {
    /* Create a handler_callback component with a callback */
    explicit HandlerComponent(const std::function<R(Args...)> &f) : handler_callback(f) {}
    
    /* Invoke the callback */
    R handle(Args... args) {
        return handler_callback(args...);
    }
    
    static void handle_all(Args... args) {
        Component<T>::system([&](T &handler) {
            handler.handle(args...);
        });
    }

private:
    /*
     * The handler's callback.
     * (This was at first named `handler`, but it turns out that `handler`
     * is a useful variable name, and that caused shadowing conflicts.)
     */
    std::function<R(Args...)> handler_callback;
};
