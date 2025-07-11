#include "pch.h"
#include "AnimationClip.h"
#include "D2DRender.h"
#include <fstream>
#include "json.hpp"
#include "AsepriteParser.h"

AnimationClips AsepriteParser::Load(const std::filesystem::path& jsonPath)
{
    /*
     Aseprite JSON �ļ��� ���� �ִϸ��̼� �����͸� �����ϴ� ����:

     1. JSON�� "frames" �׸��� �Ľ��Ͽ�, ���� ��������Ʈ �����ӵ��� Frame ����ü�� �����ϰ�,
        ������� frames ���Ϳ� �����Ѵ�. (��� ������ �����͸� �ε��ϴ� 1�� ó��)

     2. JSON�� "meta.frameTags" �׸��� ��������, ������ from~to ������ �ش��ϴ� Frame���� ���
        AnimationClip ������ �籸���Ѵ�. (���� �ִϸ��̼� ���� ���� ó��)

     3. �̸����� �з��� AnimationClip���� clips[�±��̸�] ���·� �����Ͽ� ��ȯ�Ѵ�.
        clips�� ���������� AssetManager �Ǵ� GameObject�� ���޵Ǿ� �ִϸ��̼� ����� ���ȴ�.
     */

    // 1) JSON �ε��ϼ���.
    std::ifstream ifs(jsonPath); // �б� ���� ��Ʈ���� ������

    if (!ifs.is_open()) // ������ ������
    {
        std::cerr << "������ �� �� �����ϴ�.\n";
        return {};
    }

    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    auto parsed = nlohmann::json::parse(content, nullptr, false);  // ������ ���� 'false'�� ���� ��Ȱ��ȭ

    if (parsed.is_discarded()) // �Ľ� ���Ͽ� ������ ������
    {
        std::cerr << "JSON �Ľ� ����: ��ȿ���� ���� �����Դϴ�.\n";
        return {};
    }
    const auto& root = parsed; // �Ľ̵� ���� ��Ʈ�� ǥ��

    // 1) Frames: object or array ���� ��� ����

    std::vector<Frame> frames;

    auto& framesNode = root["frames"]; // frames ��� ���� ã�� ����
    if (framesNode.is_object())
    {
        for (auto it = framesNode.begin(); it != framesNode.end(); ++it)
        {
            Frame fr; // ������ ����� ����
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

            fr.m_sourceSize.x = f["sourceSize"]["w"]; // xy�� �ƴ� ����
            fr.m_sourceSize.y = f["sourceSize"]["h"];
            fr.m_duration = f["duration"];
            frames.push_back(fr);
        }
    }
    else if (framesNode.is_array())
    {
        for (const auto& f : framesNode)
        {
            Frame fr; // ������ ����� ����
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

            fr.m_sourceSize.x = f["sourceSize"]["w"]; // xy�� �ƴ� ����
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


    // 2) �±׺���  AnimationClips �����մϴ�. ( frameTags & slices )

    AnimationClips clips; // ������ clips ��ü

    if (root["meta"].contains("frameTags"))
    {
        for (const auto& t : root["meta"]["frameTags"]) {
            int start = t["from"];
            int end = t["to"];
            AnimationClip clip; // clips�� �±׿� ���� frame���� ��Ƽ� ������ clip

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

    // 3)  (�ɼ�)
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
