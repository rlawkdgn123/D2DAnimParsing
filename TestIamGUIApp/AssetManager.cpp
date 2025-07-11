#include "pch.h"
#include "AnimationClip.h"
#include "AsepriteParser.h"
#include "D2DRender.h"
#include "AssetManager.h"

void AssetManager::LoadTexture(sample::D2DRenderer* renderer, std::wstring keyWide, filesystem::path path)
{
    if (m_textures.find(keyWide) != m_textures.end())
        return; // �̹� �ε�� �ؽ�ó�� ����

    ID2D1Bitmap1* rawBitmap = nullptr;

    path.replace_extension(L".png"); // .png ����
    renderer->CreateBitmapFromFile(path.c_str(), rawBitmap); // bitmap ����

    if (!rawBitmap) {
        std::wcerr << L"�ؽ�ó �ε� ����: " << path.c_str() << L"\n";
        return;
    }

    Microsoft::WRL::ComPtr<ID2D1Bitmap1> bitmap; 
    bitmap.Attach(rawBitmap); // ��Ʈ�� ���纻�� ��ġ��

    m_textures[keyWide] = bitmap;
    
}
// ���Ĳ�
void AssetManager::LoadAseprite(sample::D2DRenderer* renderer, std::wstring keyWide, filesystem::path path)
{ 
    if (m_textures.find(keyWide) != m_textures.end())
        return; // �̹� �ε�� �ؽ�ó�� ����

    filesystem::path jsonPath = path;
    jsonPath.replace_extension(L".json");

    AnimationClips clips = AsepriteParser::Load(jsonPath);

    filesystem::path pngPath = path;
    pngPath.replace_extension(L".png");

    ID2D1Bitmap1* rawBitmap = nullptr;

    renderer->CreateBitmapFromFile(pngPath.c_str(), rawBitmap); // bitmap ����

    std::cout <<"--------------" << rawBitmap << std::endl;



    Microsoft::WRL::ComPtr<ID2D1Bitmap1> bitmap;
    bitmap.Attach(rawBitmap); // ��Ʈ�� ���纻�� ��ġ��

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
        return it->second; // second�� ���� : AnimationClips ���°� (String / AnimationClip)�̶� Ŭ�� ��ȯ�ؾ� �ؼ�
    }
    AnimationClips emptyClips;
    cout << "�߰� ���� ! �� Ŭ�� ��ȯ" << endl;
    return emptyClips;
}

Microsoft::WRL::ComPtr<ID2D1Bitmap1> AssetManager::GetTexture(std::wstring selectedAssetKey) const
{
    auto it = m_textures.find(selectedAssetKey);
    if (it != m_textures.end()) {
        return it->second; // second�� ���� : AnimationClips ���°� (String / AnimationClip)�̶� Ŭ�� ��ȯ�ؾ� �ؼ�
    }
    Microsoft::WRL::ComPtr<ID2D1Bitmap1> emptyClips;
    cout << "�߰� ���� ! �� ��Ʈ�� ��ȯ" << endl;
    return emptyClips;
}
