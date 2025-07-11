#pragma once

#include <unordered_map>
#include <filesystem>
#include <wrl/client.h>
#include "AnimationClip.h"

using namespace std;
namespace sample{ class D2DRenderer; }
//class sample::D2DRenderer;

// 예시입니다.
class AssetManager
{
public:
    using AnimationClips = std::vector<std::pair<std::string, AnimationClip>>;
   
    AssetManager() = default;
    ~AssetManager() = default;
    void LoadTexture(sample::D2DRenderer * renderer, std::wstring keyWide, filesystem::path path);
    void LoadAseprite(sample::D2DRenderer* renderer, std::wstring keyWide, filesystem::path path);
    AnimationClips& GetClips(std::wstring selectedAssetKey);
    Microsoft::WRL::ComPtr<ID2D1Bitmap1> GetTexture(std::wstring selectedAssetKey) const;
    // 겟 함수를 만들어야되잖아? key를 매개변수로 넘겨주고 텐스쳐를 리턴받는 함수를 만들면 돼
    //m_selectedAssetKey 즉 >> 파일네임

private:

    std::unordered_map<std::wstring, Microsoft::WRL::ComPtr<ID2D1Bitmap1>> m_textures; // PNG 담겨있는놈 
    std::unordered_map<std::wstring, AnimationClips> m_clipsMap; /// 클림 담겨있는 놈
};

