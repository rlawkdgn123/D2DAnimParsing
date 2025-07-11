#pragma once

#include <unordered_map>
#include <filesystem>
#include <wrl/client.h>
#include "AnimationClip.h"

using namespace std;
namespace sample{ class D2DRenderer; }
//class sample::D2DRenderer;

// �����Դϴ�.
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
    // �� �Լ��� �����ߵ��ݾ�? key�� �Ű������� �Ѱ��ְ� �ٽ��ĸ� ���Ϲ޴� �Լ��� ����� ��
    //m_selectedAssetKey �� >> ���ϳ���

private:

    std::unordered_map<std::wstring, Microsoft::WRL::ComPtr<ID2D1Bitmap1>> m_textures; // PNG ����ִ³� 
    std::unordered_map<std::wstring, AnimationClips> m_clipsMap; /// Ŭ�� ����ִ� ��
};

