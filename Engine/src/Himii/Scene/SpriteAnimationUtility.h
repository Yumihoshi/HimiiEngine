#pragma once

#include "Himii/Core/Timestep.h"
#include "Himii/Scene/Components.h"
#include "Himii/Scene/SpriteAnimation.h"

namespace Himii
{

    void ResetSpriteAnimationPlayback(SpriteAnimationComponent& animationComponent);

    void AdvanceSpriteAnimation(SpriteAnimationComponent& animationComponent,
                                const SpriteAnimation& animation,
                                float deltaTimeSeconds);

    const char* SpriteAnimationLoopModeToString(SpriteAnimationLoopMode loopMode);

    SpriteAnimationLoopMode SpriteAnimationLoopModeFromString(const char* text);

} // namespace Himii
