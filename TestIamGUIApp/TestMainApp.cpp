#include "pch.h"
#include "GameTimer.h"
#include "D2DRender.h"

////////////////////////////////////////////////////////////////////////////
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////
#include <shobjidl.h>            // IFileOpenDialog
#include <filesystem>            // C++17 std::filesystem

/////////////////////////////////////////////////////////////////////////
#include "TestMainApp.h"

using AnimationClips = std::vector<std::pair<std::string, AnimationClip>>;
using namespace sample;
/////////////////////////////////////////////////////////////////////////////
//
std::wstring ConvertToWString(const std::string& str)
{
    size_t len = 0;
    mbstowcs_s(&len, nullptr, 0, str.c_str(), _TRUNCATE);
    if (len == 0)
        return L"";

    std::wstring wstr(len, L'\0');
    mbstowcs_s(&len, &wstr[0], len, str.c_str(), _TRUNCATE);
    wstr.resize(len - 1); // Remove the null terminator added by mbstowcs_s  
    return wstr;
}

std::string WStringToString(const std::wstring& wstr)
{
    size_t len = 0;
    wcstombs_s(&len, nullptr, 0, wstr.c_str(), _TRUNCATE);
    if (len == 0)
        return "";
    std::string str(len, '\0');
    wcstombs_s(&len, &str[0], len, wstr.c_str(), _TRUNCATE);
    str.resize(len - 1); // Remove the null terminator added by wcstombs_s
    return str;
}


/////////////////////////////////////////////////////////////////////////////
// 
bool TestMainApp::Initialize()
{
    const wchar_t* className = L"D2DLesson2";
    const wchar_t* windowName = L"D2DLesson2";

    if (false == __super::Create(className, windowName, 1024, 800))
    {
        return false;
    }

    m_Renderer = std::make_shared<sample::D2DRenderer>();
    m_Renderer->Initialize(m_hWnd);

    // [ImGUI] 컨텍스트 & 백엔드 초기화
    // 3-1) ImGui 컨텍스트 생성
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    
    ImGui_ImplWin32_Init(m_hWnd);

    ID3D11Device* pd3dDevice = m_Renderer->GetD3DDevice();

    // 2) 즉시 컨텍스트 얻기
    ID3D11DeviceContext* pd3dDeviceContext = nullptr;
    pd3dDeviceContext = m_Renderer->GetD3DContext();

    // [ImGUI] DirectX 11 백엔드 초기화
    ImGui_ImplDX11_Init(pd3dDevice, pd3dDeviceContext);

    // 타이머 초기화
    m_GameTimer.Reset();

    return true;
}

void TestMainApp::Run()
{
    MSG msg = { 0 };

    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
               
            DispatchMessage(&msg);
        }
        else
        {
            UpdateTime();
            UpdateInput();
            UpdateLogic();
            Render();        
        }
    }
}
void TestMainApp::Finalize()
{
    // [ImGUI] DirectX 11 백엔드 정리
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    if (m_Renderer != nullptr)
    {
        m_Renderer->Uninitialize();
        m_Renderer.reset();
    }
}

bool TestMainApp::OnWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
    {
        return true; // ImGui가 메시지를 처리했으면 true 반환
    }

    return false;
}


void TestMainApp::UpdateTime()
{
    m_GameTimer.Tick();
}

void TestMainApp::UpdateInput()
{
}

void TestMainApp::UpdateLogic()
{
    LoadAssets();

    if (m_selectedAssetKey.empty()) return; // 선택된 애셋 키가 비어있으면 리턴

    

    if (m_bChangedFile)
    {
        // 파일이 변경되었으면 애니메이션 정보 초기화
        m_curSprites.clear();
        m_bChangedFile = false;
        m_pngSprite.Reset();

        if (m_checkPngKey) {
            m_pngSprite = m_AssetManager.GetTexture(m_selectedAssetKey);
        }
        else {
            //m_pngSprite = m_AssetManager.GetTexture(m_selectedAssetKey);
            const AnimationClips& clips = m_AssetManager.GetClips(m_selectedAssetKey);
            // 애니메이션 플레이어 생성 및 클립 설정
            for (const auto& [name, clip] : clips)
            {
                SpriteAnimator ap;

                ap.SetClip(&clip);

                m_curSprites.push_back(ap);
            }
        }
    }
    // 애니메이션 플레이어 업데이트
    for (auto& sprite : m_curSprites)
    {
        sprite.Update(m_GameTimer.DeltaTime());
    }
}


void TestMainApp::Render()
{
    if (m_Renderer == nullptr) return;

   
    m_Renderer->RenderBegin();

    // 여러 애니메이션을 모두 보여주는 렌더링
    // 화면 중앙을 기준으로 애니메이션을 나열
    int animationIndex = 0;

    D2D1_SIZE_U size;
    float xOffset = 200.f;
    float yOffset = 300.f;

    if (m_checkPngKey) { // png이면
        size = m_pngSprite.Get()->GetPixelSize();
        D2D1_RECT_F renderRect = D2D1::RectF(xOffset, yOffset, xOffset + size.width, yOffset + size.height);
        m_Renderer->DrawBitmap(m_pngSprite.Get() , renderRect);
    }
    else { // 애니메이션이면
        for (auto& ap : m_curSprites)
        {
            int count = m_curSprites.size();

            if (ap.IsValid() == false) continue; // 유효하지 않은 플레이어는 스킵

            const Frame& frame = ap.GetCurrentFrame();

            int xOffset = static_cast<float>(animationIndex * frame.Width()) + 200.f;
            int yOffset = 300;

            D2D1_RECT_F renderRect = D2D1::RectF(xOffset, yOffset, xOffset + frame.Width(), yOffset + frame.Height());
            //D2D1_RECT_F renderRect = D2D1::RectF(0, 0, frame.Width(),frame.Height());
            if (!ap.GetClip()) { cout << "에러!!" << endl; }
            if (!ap.GetClip()->GetBitmap()) { cout << "에러22!!" << endl; }

            std::cout << "카운트 체크" << std::endl;
            std::cout<< xOffset<<std::endl;
            std::cout << animationIndex << std::endl;

            m_Renderer->DrawBitmap(ap.GetClip()->GetBitmap(), renderRect, frame.ToRectF());

            animationIndex++;
        }
    }
    

    m_Renderer->RenderEnd(false);

    RenderImGUI();

    // Present, ImGUI 때문에 enddraw 와 분리
    m_Renderer->Present();
}


void TestMainApp::RenderImGUI()
{
    ID3D11DeviceContext* pd3dDeviceContext = nullptr;
    pd3dDeviceContext = m_Renderer->GetD3DContext();
    ID3D11RenderTargetView* rtvs[] = { m_Renderer->GetD3DRenderTargetView() };

    if (pd3dDeviceContext == nullptr || rtvs[0] == nullptr)
    {
        return; // 렌더링 컨텍스트나 뷰가 없으면 리턴
    }
    m_Renderer->GetD3DContext()->OMSetRenderTargets(1, rtvs, nullptr);

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // 데모 UI 창 표시
    //static bool showDemo = true;
    //ImGui::ShowDemoWindow(&showDemo);

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            // 메뉴 아이템을 누르면 패널 열기 플래그 토글
            if (ImGui::MenuItem("Open Folder", "Ctrl+O"))
            {
                m_showFolderPanel = true;
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    // 모달리스 패널로 변경
    if (m_showFolderPanel)
    {
        ImGui::Begin("Folder Browser", &m_showFolderPanel, ImGuiWindowFlags_AlwaysAutoResize);

        if (ImGui::Button("Browse..."))
        {
            BrowseForFolder();
            m_pathInput = std::filesystem::path(m_folderPath).u8string();
        }

        ImGui::SameLine();
        if (ImGui::Button("Load"))
        {
            if (false == m_folderPath.empty())
            {
                UpdateFileList();
                m_selectedFile.clear();
            }
        }

        ImGui::Text("Folder: %ls", m_folderPath.c_str());

        // 파일 목록 렌더링
        if (false == m_folderPath.empty())
        {
            if (ImGui::BeginListBox("Files", ImVec2(300.0f, 8 * ImGui::GetTextLineHeightWithSpacing())))
            {
                for (const auto& name : m_fileList)
                {
                    bool isSelected = (m_selectedFile == name);
                    if (ImGui::Selectable(WStringToString(name).c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick))
                    {
                        m_selectedFile = name;
                    }

                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
                    {
                        m_selectedFile = name;
                    }
                }

                ImGui::EndListBox();
            }

            if (!m_selectedFile.empty())
            {
                ImGui::Text("Selected File: %ls", m_selectedFile.c_str());
            }
        }

        ImGui::End(); // Folder Browser 패널 끝
    }

    // ImGui 렌더링
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void TestMainApp::LoadAssets()
{
    std::filesystem::path fullPath =
        m_folderPath / std::filesystem::path(m_selectedFile);

    auto ext = fullPath.extension();
    if (ext.empty()) return; // 확장자가 없으면 리턴

    std::filesystem::path keyPath = fullPath;
    keyPath.replace_extension(); // 확장자 제거하고 키로 쓸꺼에요

    std::wstring keyWide = keyPath.wstring();

    if (keyWide == m_selectedAssetKey)
    {
        // 이미 로드된 파일이면 다시 로드하지 않음
        return;
    }
    m_selectedAssetKey = keyWide; // 선택된 파일 키 저장
  
    if (ext == L".png")
    {
        m_AssetManager.LoadTexture(m_Renderer.get(), keyWide, fullPath);
        m_checkPngKey = true;
    }
    else if (ext == L".json")
    {
        m_AssetManager.LoadAseprite(m_Renderer.get(), keyWide, fullPath);
        m_checkPngKey = false;
    }

    m_bChangedFile = true; // 파일이 변경되었음을 표시
}


void TestMainApp::OnResize(int width, int height)
{
    __super::OnResize(width, height);

    if (m_Renderer != nullptr) m_Renderer->Resize(width, height);
}

void TestMainApp::OnClose()
{
    std::cout << "OnClose" << std::endl;
}

void TestMainApp::BrowseForFolder()
{
    HRESULT hr;
    IFileOpenDialog* pDialog = nullptr;

    // COM 대화상자 생성
    hr = CoCreateInstance(
        CLSID_FileOpenDialog, nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pDialog)
    );

    if (FAILED(hr) || !pDialog) return;

    // 폴더 선택 모드로 설정
    DWORD opts = 0;
    if (SUCCEEDED(pDialog->GetOptions(&opts)))
        pDialog->SetOptions(opts | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);

    // 다이얼로그 표시
    hr = pDialog->Show(m_hWnd);

    if (SUCCEEDED(hr))
    {
        IShellItem* pItem = nullptr;
        hr = pDialog->GetResult(&pItem);

        if (SUCCEEDED(hr) && pItem)
        {
            PWSTR pszFolder = nullptr;
            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFolder);
            if (SUCCEEDED(hr) && pszFolder)
            {
                m_folderPath = pszFolder;        // 선택된 폴더 경로 저장
                CoTaskMemFree(pszFolder);
            }
            pItem->Release();
        }   
    }

    pDialog->Release();
}

void TestMainApp::UpdateFileList()
{
    m_fileList.clear();
    for (const auto& entry : std::filesystem::directory_iterator(m_folderPath))
    {
        if (entry.is_regular_file())
        {
            // 파일만 추가, 디렉토리는 제외
            if (entry.path().extension() == L".png" ||
                entry.path().extension() == L".json")
                // 이미지 파일만 추가
                m_fileList.push_back(entry.path().filename().wstring());
        }
    }
}

