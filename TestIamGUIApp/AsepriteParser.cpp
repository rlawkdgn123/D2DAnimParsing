#include "pch.h"
#include "AnimationClip.h"
#include "D2DRender.h"
#include <fstream>
#include "json.hpp"
#include "AsepriteParser.h"

AnimationClips AsepriteParser::Load(const std::filesystem::path& jsonPath)
{
    /*
     Aseprite JSON 파서를 통해 애니메이션 데이터를 구성하는 과정:

     1. JSON의 "frames" 항목을 파싱하여, 개별 스프라이트 프레임들을 Frame 구조체로 생성하고,
        순서대로 frames 벡터에 저장한다. (모든 프레임 데이터를 로드하는 1차 처리)

     2. JSON의 "meta.frameTags" 항목을 기준으로, 지정된 from~to 범위에 해당하는 Frame들을 모아
        AnimationClip 단위로 재구성한다. (논리적 애니메이션 단위 분할 처리)

     3. 이름별로 분류된 AnimationClip들을 clips[태그이름] 형태로 저장하여 반환한다.
        clips는 최종적으로 AssetManager 또는 GameObject에 전달되어 애니메이션 재생에 사용된다.
     */

    // 1) JSON 로드하세요.
    std::ifstream ifs(jsonPath); // 읽기 전용 스트림에 가져옴

    if (!ifs.is_open()) // 열리지 않으면
    {
        std::cerr << "파일을 열 수 없습니다.\n";
        return {};
    }

    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    auto parsed = nlohmann::json::parse(content, nullptr, false);  // 마지막 인자 'false'는 예외 비활성화

    if (parsed.is_discarded()) // 파싱 파일에 문제가 있으면
    {
        std::cerr << "JSON 파싱 실패: 유효하지 않은 문서입니다.\n";
        return {};
    }
    const auto& root = parsed; // 파싱된 지점 루트로 표시

    // 1) Frames: object or array 형태 모두 지원

    std::vector<Frame> frames;

    auto& framesNode = root["frames"]; // frames 라는 행을 찾아 적기
    if (framesNode.is_object())
    {
        for (auto it = framesNode.begin(); it != framesNode.end(); ++it)
        {
            Frame fr; // 프레임 만들고 시작
            const auto& f = it.value();
            fr.m_filename = it.key();
            
            UINT32 x = f["frame"]["x"];
            UINT32 y = f["frame"]["y"];
            UINT32 w = f["frame"]["w"];
            UINT32 h = f["frame"]["h"];

            fr.srcRect.left = x;
            fr.srcRect.top = y;
            fr.srcRect.right = x+w;
            fr.srcRect.bottom = y+h;

            fr.m_rotated = f["rotated"];
            fr.m_trimmed = f["trimmed"];

            UINT32 sx = f["spriteSourceSize"]["x"];
            UINT32 sy = f["spriteSourceSize"]["y"];
            UINT32 sw = f["spriteSourceSize"]["w"];
            UINT32 sh = f["spriteSourceSize"]["h"];

            fr.m_spriteSourceSize.left = sx;
            fr.m_spriteSourceSize.top = sy;
            fr.m_spriteSourceSize.right = sx + sw;
            fr.m_spriteSourceSize.bottom = sy + sh;

            fr.m_sourceSize.x = f["sourceSize"]["w"]; // xy가 아님 주의
            fr.m_sourceSize.y = f["sourceSize"]["h"];
            fr.m_duration = f["duration"];
            frames.push_back(fr);
        }
    }
    else if (framesNode.is_array())
    {
        for (const auto& f : framesNode)
        {
            Frame fr; // 프레임 만들고 시작
            fr.m_filename = f["filename"];

            UINT32 x = f["frame"]["x"];
            UINT32 y = f["frame"]["y"];
            UINT32 w = f["frame"]["w"];
            UINT32 h = f["frame"]["h"];

            fr.srcRect.left = x;
            fr.srcRect.top = y;
            fr.srcRect.right = x + w;
            fr.srcRect.bottom = y + h;
            
            fr.m_rotated = f["rotated"];
            fr.m_trimmed = f["trimmed"];

            UINT32 sx = f["spriteSourceSize"]["x"];
            UINT32 sy = f["spriteSourceSize"]["y"];
            UINT32 sw = f["spriteSourceSize"]["w"];
            UINT32 sh = f["spriteSourceSize"]["h"];

            fr.m_spriteSourceSize.left = sx;
            fr.m_spriteSourceSize.top = sy;
            fr.m_spriteSourceSize.right = sx + sw;
            fr.m_spriteSourceSize.bottom = sy + sh;

            fr.m_sourceSize.x = f["sourceSize"]["w"]; // xy가 아님 주의
            fr.m_sourceSize.y = f["sourceSize"]["h"];
            fr.m_duration = f["duration"] / 1000.f;
            frames.push_back(fr);
        }
    }
    else
    {
        std::cerr << "Unsupported 'frames' format" << '\n';
        return {};
    }


    // 2) 태그별로  AnimationClips 생성합니다. ( frameTags & slices )

    AnimationClips clips; // 리턴할 clips 객체

    if (root["meta"].contains("frameTags"))
    {
        for (const auto& t : root["meta"]["frameTags"]) {
            int start = t["from"];
            int end = t["to"];
            AnimationClip clip; // clips에 태그에 묶인 frame들을 모아서 보내줄 clip

            for (int i = start; i <= end; ++i) {
                clip.AddFrame(frames[i]);
            }
            /*Tag tag;
            tag.m_name = t["name"];
            tag.m_from = t["from"];
            tag.m_to = t["to"];
            tag.m_direction = t["direction"];
            outData.m_frameTags.push_back(tag);*/
            clips.emplace_back(t["name"], clip);
        }
    }

    // 3)  (옵션)
    /*if (root["meta"].contains("slices"))
    {
        for (const auto& s : root["meta"]["slices"]) {
            Slice slice;
            slice.m_name = s["name"];
            for (const auto& key : s["keys"]) {
                SliceKey sk;
                sk.m_frame = key["frame"];
                sk.m_bounds.m_x = key["bounds"]["x"];
                sk.m_bounds.m_y = key["bounds"]["y"];
                sk.m_bounds.m_w = key["bounds"]["w"];
                sk.m_bounds.m_h = key["bounds"]["h"];
                sk.m_pivot.m_x = key["pivot"]["x"];
                sk.m_pivot.m_y = key["pivot"]["y"];
                slice.m_keys.push_back(sk);
            }

            outData.m_slices.push_back(slice);
        }
    }*/
    return clips;
}
