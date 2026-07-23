#pragma once

#include "Himii/Core/Core.h"
#include "Himii/Renderer/Shader.h"

namespace Himii::FontRegression
{
    bool RunTextLayoutSmokeTests();
    bool RunShaderValiditySmokeTest(const Ref<Shader> &textShader);
}
