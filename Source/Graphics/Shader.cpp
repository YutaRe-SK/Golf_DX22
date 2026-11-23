#include "Shader.h"
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

bool Shader::Compile(const wchar_t* path, const char* entry, const char* target, ID3DBlob** outBlob)
{
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    Microsoft::WRL::ComPtr<ID3DBlob> error;
    HRESULT hr = D3DCompileFromFile(path, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entry, target, flags, 0, outBlob, &error);
    if (FAILED(hr))
    {
        if (error)
        {
            // HLSLÇÃÉGÉâÅ[ï∂ÇÕâpåÍÇ»ÇÃÇ≈MessageBoxAÇ≈ï\é¶Ç∑ÇÈ
            MessageBoxA(nullptr, static_cast<const char*>(error->GetBufferPointer()), "Shader Compile Error", MB_OK);
        }
        else
        {
            MessageBoxW(nullptr, path, L"Shader file not found", MB_OK);
        }
        return false;
    }
    return true;
}

bool Shader::Load(ID3D11Device* device, const wchar_t* vsPath, const wchar_t* psPath)
{
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> psBlob;

    if (!Compile(vsPath, "VSMain", "vs_5_0", &vsBlob)) return false;
    if (!Compile(psPath, "PSMain", "ps_5_0", &psBlob)) return false;

    if (FAILED(device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vs))) return false;
    if (FAILED(device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_ps))) return false;

    // Mesh::Vertex Ç∆èáî‘ÅEå^ÇàÍívÇ≥ÇπÇÈ
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    if (FAILED(device->CreateInputLayout(layout, ARRAYSIZE(layout),
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_layout))) return false;

    return true;
}

void Shader::Bind(ID3D11DeviceContext* context) const
{
    context->IASetInputLayout(m_layout.Get());
    context->VSSetShader(m_vs.Get(), nullptr, 0);
    context->PSSetShader(m_ps.Get(), nullptr, 0);
}