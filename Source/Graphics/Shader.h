#pragma once
#include <d3d11.h>
#include <wrl/client.h>

class Shader
{
public:
    // vsPath/psPath: e.g. L"Shaders/StandardVS.hlsl"
    bool Load(ID3D11Device* device, const wchar_t* vsPath, const wchar_t* psPath);
    void Bind(ID3D11DeviceContext* context) const;

private:
    bool Compile(const wchar_t* path, const char* entry, const char* target, ID3DBlob** outBlob);

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vs;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_ps;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_layout;
};