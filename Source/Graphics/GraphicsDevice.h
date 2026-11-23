#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

class GraphicsDevice
{
public:
    bool Initialize(HWND hWnd, int width, int height);
    void Shutdown();
    bool Resize(int width, int height);

    void BeginFrame(float r, float g, float b, float a);
    void EndFrame();

    ID3D11Device* GetDevice()  const { return m_device.Get(); }
    ID3D11DeviceContext* GetContext() const { return m_context.Get(); }
    int GetWidth()  const { return m_width; }
    int GetHeight() const { return m_height; }

private:
    bool CreateRenderTargets();
    void ReleaseRenderTargets();

    Microsoft::WRL::ComPtr<ID3D11Device>           m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext>    m_context;
    Microsoft::WRL::ComPtr<IDXGISwapChain>         m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;
    Microsoft::WRL::ComPtr<ID3D11Texture2D>        m_depthBuffer;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsv;

    D3D11_VIEWPORT m_viewport = {};
    int m_width = 0;
    int m_height = 0;
};