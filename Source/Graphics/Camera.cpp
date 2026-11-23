#include "Camera.h"

using namespace DirectX;

void Camera::SetPerspective(float fovY, float aspect, float nearZ, float farZ)
{
    m_fovY = fovY;
    m_aspect = aspect;
    m_nearZ = nearZ;
    m_farZ = farZ;
}

void Camera::SetAspect(float aspect)
{
    m_aspect = aspect;
}

void Camera::LookAt(const XMFLOAT3& eye, const XMFLOAT3& target)
{
    m_eye = eye;
    m_target = target;
}

XMMATRIX Camera::GetView() const
{
    return XMMatrixLookAtLH(XMLoadFloat3(&m_eye), XMLoadFloat3(&m_target), XMVectorSet(0, 1, 0, 0));
}

XMMATRIX Camera::GetProjection() const
{
    return XMMatrixPerspectiveFovLH(m_fovY, m_aspect, m_nearZ, m_farZ);
}