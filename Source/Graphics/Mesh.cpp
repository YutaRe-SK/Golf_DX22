#include "Mesh.h"
#include <cmath>

using namespace DirectX;

bool Mesh::Create(ID3D11Device* device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
    D3D11_BUFFER_DESC vbd = {};
    vbd.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertices.size());
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vdata = { vertices.data() };
    if (FAILED(device->CreateBuffer(&vbd, &vdata, &m_vb))) return false;

    D3D11_BUFFER_DESC ibd = {};
    ibd.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * indices.size());
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA idata = { indices.data() };
    if (FAILED(device->CreateBuffer(&ibd, &idata, &m_ib))) return false;

    m_indexCount = static_cast<UINT>(indices.size());
    return true;
}

void Mesh::Draw(ID3D11DeviceContext* context) const
{
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_vb.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(m_ib.Get(), DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->DrawIndexed(m_indexCount, 0, 0);
}

bool Mesh::CreateCheckerPlane(ID3D11Device* device, float size, int divisions,
    const XMFLOAT4& colorA, const XMFLOAT4& colorB)
{
    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;

    float cell = size / divisions;
    float half = size * 0.5f;
    XMFLOAT3 up = { 0.0f, 1.0f, 0.0f };

    for (int z = 0; z < divisions; ++z)
    {
        for (int x = 0; x < divisions; ++x)
        {
            float x0 = -half + x * cell, x1 = x0 + cell;
            float z0 = -half + z * cell, z1 = z0 + cell;
            const XMFLOAT4& c = ((x + z) % 2 == 0) ? colorA : colorB;

            uint32_t base = static_cast<uint32_t>(vertices.size());
            vertices.push_back({ { x0, 0.0f, z0 }, up, c });
            vertices.push_back({ { x0, 0.0f, z1 }, up, c });
            vertices.push_back({ { x1, 0.0f, z1 }, up, c });
            vertices.push_back({ { x1, 0.0f, z0 }, up, c });

            // 上から見て時計回り（D3D11の表面）
            indices.insert(indices.end(), { base, base + 1, base + 2, base, base + 2, base + 3 });
        }
    }
    return Create(device, vertices, indices);
}

bool Mesh::CreateSphere(ID3D11Device* device, float radius, int slices, int stacks, const XMFLOAT4& color)
{
    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;

    for (int i = 0; i <= stacks; ++i)
    {
        float phi = XM_PI * i / stacks;            // 0(上) ～ PI(下)
        for (int j = 0; j <= slices; ++j)
        {
            float theta = XM_2PI * j / slices;
            XMFLOAT3 n = { sinf(phi) * cosf(theta), cosf(phi), sinf(phi) * sinf(theta) };
            XMFLOAT3 p = { n.x * radius, n.y * radius, n.z * radius };
            vertices.push_back({ p, n, color });
        }
    }

    for (int i = 0; i < stacks; ++i)
    {
        for (int j = 0; j < slices; ++j)
        {
            uint32_t a = i * (slices + 1) + j;
            uint32_t b = (i + 1) * (slices + 1) + j;
            indices.insert(indices.end(), { a, a + 1, b, a + 1, b + 1, b });
        }
    }
    return Create(device, vertices, indices);
}

bool Mesh::CreateQuad(ID3D11Device* device)
{
    XMFLOAT3 n = { 0.0f, 0.0f, -1.0f };
    XMFLOAT4 c = { 1.0f, 1.0f, 1.0f, 1.0f };

    std::vector<Vertex> vertices =
    {
        { { 0.0f, 0.0f, 0.0f }, n, c },
        { { 1.0f, 0.0f, 0.0f }, n, c },
        { { 1.0f, 1.0f, 0.0f }, n, c },
        { { 0.0f, 1.0f, 0.0f }, n, c },
    };
    std::vector<uint32_t> indices = { 0, 1, 2, 0, 2, 3 };
    return Create(device, vertices, indices);
}