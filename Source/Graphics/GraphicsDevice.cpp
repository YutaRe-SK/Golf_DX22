#include "GraphicsDevice.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

bool GraphicsDevice::Initialize(HWND hWnd, int width, int height)
{
    m_width = width;
    m_height = height;

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount       = 2;
    sd.BufferDesc.Width  = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator   = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage       = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow      = hWnd;
    sd.SampleDesc.Count  = 1;
    sd.Windowed          = TRUE;
    sd.SwapEffect        = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    UINT flags = 0;
#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };
    D3D_FEATURE_LEVEL created;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
        levels, 1, D3D11_SDK_VERSION,
        &sd, &m_swapChain, &m_device, &created, &m_context);

    // デバッグレイヤーが入っていない環境では、フラグを外して再試行する
    if (FAILED(hr) && (flags & D3D11_CREATE_DEVICE_DEBUG))
    {
        flags &= ~D3D11_CREATE_DEVICE_DEBUG;
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
            levels, 1, D3D11_SDK_VERSION,
            &sd, &m_swapChain, &m_device, &created, &m_context);
    }

    if (FAILED(hr))
    {
        MessageBoxW(hWnd, L"D3D11CreateDeviceAndSwapChain failed", L"Error", MB_OK);
        return false;
    }

    return CreateRenderTargets();
}

bool GraphicsDevice::CreateRenderTargets()
{
    // カラーバッファ
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    if (FAILED(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)))) return false;
    if (FAILED(m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_rtv))) return false;

    // 深度バッファ
    D3D11_TEXTURE2D_DESC dd = {};
    dd.Width            = m_width;
    dd.Height           = m_height;
    dd.MipLevels        = 1;
    dd.ArraySize        = 1;
    dd.Format           = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dd.SampleDesc.Count = 1;
    dd.Usage            = D3D11_USAGE_DEFAULT;
    dd.BindFlags        = D3D11_BIND_DEPTH_STENCIL;
    if (FAILED(m_device->CreateTexture2D(&dd, nullptr, &m_depthBuffer))) return false;
    if (FAILED(m_device->CreateDepthStencilView(m_depthBuffer.Get(), nullptr, &m_dsv))) return false;

    // ビューポート
    m_viewport.TopLeftX = 0.0f;
    m_viewport.TopLeftY = 0.0f;
    m_viewport.Width    = static_cast<float>(m_width);
    m_viewport.Height   = static_cast<float>(m_height);
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;

    return true;
}

void GraphicsDevice::ReleaseRenderTargets()
{
    m_context->OMSetRenderTargets(0, nullptr, nullptr);
    m_rtv.Reset();
    m_dsv.Reset();
    m_depthBuffer.Reset();
}

bool GraphicsDevice::Resize(int width, int height)
{
    if (width <= 0 || height <= 0 || !m_swapChain) return false;

    m_width = width;
    m_height = height;

    ReleaseRenderTargets();
    if (FAILED(m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0)))
    {
        return false;
    }
    return CreateRenderTargets();
}

void GraphicsDevice::BeginFrame(float r, float g, float b, float a)
{
    // FLIP系のスワップチェインでは、毎フレームRTVを設定し直す
    m_context->OMSetRenderTargets(1, m_rtv.GetAddressOf(), m_dsv.Get());
    m_context->RSSetViewports(1, &m_viewport);

    const float color[4] = { r, g, b, a };
    m_context->ClearRenderTargetView(m_rtv.Get(), color);
    m_context->ClearDepthStencilView(m_dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void GraphicsDevice::EndFrame()
{
    m_swapChain->Present(1, 0);   // 1 = 垂直同期あり
}

void GraphicsDevice::Shutdown()
{
    if (m_context)
    {
        m_context->ClearState();
    }
    ReleaseRenderTargets();
    m_swapChain.Reset();
    m_context.Reset();
    m_device.Reset();
}