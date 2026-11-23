#pragma once
#include <DirectXMath.h>

class Camera
{
public:
    void SetPerspective(float fovY, float aspect, float nearZ, float farZ);
    void SetAspect(float aspect);
    void LookAt(const DirectX::XMFLOAT3& eye, const DirectX::XMFLOAT3& target);

    DirectX::XMMATRIX GetView() const;
    DirectX::XMMATRIX GetProjection() const;
    const DirectX::XMFLOAT3& GetPosition() const { return m_eye; }

private:
    DirectX::XMFLOAT3 m_eye = { 0.0f, 2.0f, -5.0f };
    DirectX::XMFLOAT3 m_target = { 0.0f, 0.0f, 0.0f };
    float m_fovY = DirectX::XM_PIDIV4;
    float m_aspect = 16.0f / 9.0f;
    float m_nearZ = 0.05f;
    float m_farZ = 1000.0f;
};