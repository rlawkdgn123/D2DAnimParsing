#include "pch.h"
#include "AnimationClip.h"
#include "AsepriteParser.h"
#include "D2DRender.h"
#include "AssetManager.h"

void AssetManager::LoadTexture(sample::D2DRenderer* renderer, std::wstring keyWide, filesystem::path path)
{
    if (m_textures.find(keyWide) != m_textures.end())
        return; // 이미 로드된 텍스처면 무시

    ID2D1Bitmap1* rawBitmap = nullptr;

    path.replace_extension(L".png"); // .png 제거
    renderer->CreateBitmapFromFile(path.c_str(), rawBitmap); // bitmap 생성

    if (!rawBitmap) {
        std::wcerr << L"텍스처 로드 실패: " << path.c_str() << L"\n";
        return;
    }

    Microsoft::WRL::ComPtr<ID2D1Bitmap1> bitmap; 
    bitmap.Attach(rawBitmap); // 비트맵 복사본에 합치기

    m_textures[keyWide] = bitmap;
    
}
// 장후꺼
void AssetManager::LoadAseprite(sample::D2DRenderer* renderer, std::wstring keyWide, filesystem::path path)
{ 
    if (m_textures.find(keyWide) != m_textures.end())
        return; // 이미 로드된 텍스처면 무시

    filesystem::path jsonPath = path;
    jsonPath.replace_extension(L".json");

    AnimationClips clips = AsepriteParser::Load(jsonPath);

    filesystem::path pngPath = path;
    pngPath.replace_extension(L".png");

    ID2D1Bitmap1* rawBitmap = nullptr;

    renderer->CreateBitmapFromFile(pngPath.c_str(), rawBitmap); // bitmap 생성

    std::cout <<"--------------" << rawBitmap << std::endl;



    Microsoft::WRL::ComPtr<ID2D1Bitmap1> bitmap;
    bitmap.Attach(rawBitmap); // 비트맵 복사본에 합치기

    for (auto& [_name, _clip] : clips)
    {
        _clip.SetBitmap(bitmap);
    }

    m_textures[keyWide] = bitmap;
    m_clipsMap[keyWide] = clips;
}

AnimationClips& AssetManager::GetClips(std::wstring selectedAssetKey)
{
    auto it = m_clipsMap.find(selectedAssetKey);
    if (it != m_clipsMap.end()) {
        return it->second; // second인 이유 : AnimationClips 형태가 (String / AnimationClip)이라 클립 반환해야 해서
    }
    AnimationClips emptyClips;
    cout << "발견 실패 ! 빈 클립 반환" << endl;
    return emptyClips;
}

Microsoft::WRL::ComPtr<ID2D1Bitmap1> AssetManager::GetTexture(std::wstring selectedAssetKey) const
{
    auto it = m_textures.find(selectedAssetKey);
    if (it != m_textures.end()) {
        return it->second; // second인 이유 : AnimationClips 형태가 (String / AnimationClip)이라 클립 반환해야 해서
    }
    Microsoft::WRL::ComPtr<ID2D1Bitmap1> emptyClips;
    cout << "발견 실패 ! 빈 비트맵 반환" << endl;
    return emptyClips;
}
