#pragma once

#include <filesystem>
#include "AnimationClip.h" 

using AnimationClips = std::vector<std::pair<std::string, AnimationClip>>;
class AsepriteParser
{
public:
    AsepriteParser() = default;
    ~AsepriteParser() = default;

    static AnimationClips Load(const std::filesystem::path& jsonPath);
};
