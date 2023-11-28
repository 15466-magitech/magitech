#pragma once

#include "../HandlerComponent.hpp"

/*
 * A component marking things to draw *after* all the Scene::Drawables.
 * Note that confusingly, Scene::Drawables do not have this component.
 */
struct Draw : HandlerComponent<Draw, void> {
    using HandlerComponent<Draw, void>::HandlerComponent;
};
