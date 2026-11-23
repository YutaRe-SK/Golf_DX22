#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <vector>
#include <cstdint>

class Mesh
{
public:
    struct Vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT4 color;
    };

    bool Create(ID3D11Device* device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
    void Draw(ID3D11DeviceContext* context) const;

    // 市松模様の地面（XZ平面、中心が原点）
    bool CreateCheckerPlane(ID3D11Device* device, float size, int divisions,
        const DirectX::XMFLOAT4& colorA, const DirectX::XMFLOAT4& colorB);
    // 球（中心が原点）
    bool CreateSphere(ID3D11Device* device, float radius, int slices, int stacks,
        const DirectX::XMFLOAT4& color);
    // UI用の四角形（0～1の範囲、左上が原点）
    bool CreateQuad(ID3D11Device* device);

private:
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vb;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_ib;
    UINT m_indexCount = 0;
};