#pragma once
#include <DirectXMath.h>

struct ClubData
{
    const char* name;            // 半角英数字のみ（タイトルバー表示用）
    float             maxSpeed;        // ゲージ最大時の初速 (m/s)
    float             launchAngleDeg;  // 打ち出し角 (度)
    DirectX::XMFLOAT4 color;           // UI表示色
};

class Club
{
public:
    void Next();
    void Prev();

    const ClubData& GetCurrent() const { return CLUBS[m_index]; }
    int GetIndex() const { return m_index; }

    static int GetCount();
    static const ClubData& GetData(int index) { return CLUBS[index]; }

private:
    int m_index = 0;
    static const ClubData CLUBS[];
};