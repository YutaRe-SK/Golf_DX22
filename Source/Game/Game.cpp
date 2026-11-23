#include "Game.h"
#include <Windows.h>
#include <cmath>

using namespace DirectX;

namespace
{
    const XMFLOAT3 START_POSITION = { 0.0f, Ball::RADIUS, -150.0f };

    XMFLOAT4 LerpColor(const XMFLOAT4& a, const XMFLOAT4& b, float t)
    {
        return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t,
                 a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t };
    }
}

bool Game::Initialize(GraphicsDevice* graphics)
{
    m_graphics = graphics;
    ID3D11Device* device = graphics->GetDevice();

    if (!m_standardShader.Load(device, L"Shaders/StandardVS.hlsl", L"Shaders/StandardPS.hlsl")) return false;
    if (!m_uiShader.Load(device, L"Shaders/UIVS.hlsl", L"Shaders/UIPS.hlsl")) return false;
    if (!CreateConstantBuffers()) return false;
    if (!CreateRenderStates()) return false;

    // 地面：400m四方、4mマス（ドライバーの飛距離に合わせて拡大）
    if (!m_ground.CreateCheckerPlane(device, 400.0f, 100,
        { 0.20f, 0.55f, 0.20f, 1.0f }, { 0.25f, 0.62f, 0.25f, 1.0f })) return false;

    if (!m_ballMesh.CreateSphere(device, Ball::RADIUS, 32, 16, { 1.0f, 1.0f, 1.0f, 1.0f })) return false;
    if (!m_uiQuad.CreateQuad(device)) return false;

    m_camera.SetPerspective(XM_PIDIV4,
        static_cast<float>(graphics->GetWidth()) / graphics->GetHeight(), 0.05f, 1000.0f);

    ResetHole();
    return true;
}

bool Game::CreateConstantBuffers()
{
    ID3D11Device* device = m_graphics->GetDevice();

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    bd.ByteWidth = sizeof(CBFrame);
    if (FAILED(device->CreateBuffer(&bd, nullptr, &m_cbFrame))) return false;

    bd.ByteWidth = sizeof(CBObject);
    if (FAILED(device->CreateBuffer(&bd, nullptr, &m_cbObject))) return false;

    return true;
}

bool Game::CreateRenderStates()
{
    ID3D11Device* device = m_graphics->GetDevice();

    // UI用：深度テストなし（UI同士が重なっても後に描いたものが上に出る）
    D3D11_DEPTH_STENCIL_DESC dsd = {};
    dsd.DepthEnable = FALSE;
    dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsd.DepthFunc = D3D11_COMPARISON_ALWAYS;
    if (FAILED(device->CreateDepthStencilState(&dsd, &m_dsNoDepth))) return false;

    // UI用：裏面カリングなし（画面座標への変換で頂点の巡回順が反転するため）
    D3D11_RASTERIZER_DESC rd = {};
    rd.FillMode = D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_NONE;
    rd.DepthClipEnable = TRUE;
    if (FAILED(device->CreateRasterizerState(&rd, &m_rsNoCull))) return false;

    return true;
}

void Game::OnResize(int width, int height)
{
    if (height > 0)
    {
        m_camera.SetAspect(static_cast<float>(width) / height);
    }
}

void Game::ResetHole()
{
    m_ball.Reset(START_POSITION);
    m_gauge.Stop();
    m_state = State::Aiming;
    m_aimYaw = 0.0f;
    m_strokes = 0;

    // カメラを即座に定位置へ
    m_cameraEye = ComputeDesiredEye();
    m_camera.LookAt(m_cameraEye, m_ball.GetPosition());
}

void Game::Update(float deltaTime, const Input& input)
{
    if (input.IsTriggered('R'))
    {
        ResetHole();
        return;
    }

    switch (m_state)
    {
    case State::Aiming:
    {
        const float aimSpeed = 1.0f;  // ラジアン/秒
        if (input.IsDown(VK_LEFT))  m_aimYaw -= aimSpeed * deltaTime;
        if (input.IsDown(VK_RIGHT)) m_aimYaw += aimSpeed * deltaTime;

        if (input.IsTriggered(VK_UP))   m_club.Prev();
        if (input.IsTriggered(VK_DOWN)) m_club.Next();

        if (input.IsTriggered(VK_SPACE))
        {
            m_gauge.Start();
            m_state = State::Charging;
        }
        break;
    }

    case State::Charging:
    {
        m_gauge.Update(deltaTime);

        if (input.IsTriggered('C'))
        {
            m_gauge.Stop();
            m_state = State::Aiming;
        }
        else if (input.IsTriggered(VK_SPACE))
        {
            float power = m_gauge.Stop();
            if (power < 0.02f) power = 0.02f;

            const ClubData& club = m_club.GetCurrent();
            m_ball.Hit(m_aimYaw, XMConvertToRadians(club.launchAngleDeg), club.maxSpeed * power);
            ++m_strokes;
            m_state = State::Flying;
        }
        break;
    }

    case State::Flying:
        m_ball.Update(deltaTime);
        if (!m_ball.IsMoving())
        {
            m_state = State::Aiming;
        }
        break;
    }

    UpdateCamera(deltaTime);
}

XMFLOAT3 Game::ComputeDesiredEye() const
{
    // 飛行中は少し引いて高めから追う
    bool flying = (m_state == State::Flying);
    float distance = flying ? 6.0f : 3.0f;
    float height = flying ? 2.5f : 1.2f;

    const XMFLOAT3& p = m_ball.GetPosition();
    XMFLOAT3 eye =
    {
        p.x - sinf(m_aimYaw) * distance,
        p.y + height,
        p.z - cosf(m_aimYaw) * distance
    };
    if (eye.y < 0.3f) eye.y = 0.3f;
    return eye;
}

void Game::UpdateCamera(float deltaTime)
{
    XMFLOAT3 desired = ComputeDesiredEye();

    // なめらかに追従（フレームレートに依存しない補間）
    float t = 1.0f - expf(-6.0f * deltaTime);
    m_cameraEye.x += (desired.x - m_cameraEye.x) * t;
    m_cameraEye.y += (desired.y - m_cameraEye.y) * t;
    m_cameraEye.z += (desired.z - m_cameraEye.z) * t;

    m_camera.LookAt(m_cameraEye, m_ball.GetPosition());
}

void Game::DrawMesh(const Mesh& mesh, FXMMATRIX world, const XMFLOAT4& color)
{
    CBObject cb;
    XMStoreFloat4x4(&cb.world, XMMatrixTranspose(world));
    cb.color = color;
    m_graphics->GetContext()->UpdateSubresource(m_cbObject.Get(), 0, nullptr, &cb, 0, 0);
    mesh.Draw(m_graphics->GetContext());
}

void Game::DrawRect(float x, float y, float w, float h, const XMFLOAT4& color)
{
    // ピクセル座標（左上原点）→ 画面座標
    float W = static_cast<float>(m_graphics->GetWidth());
    float H = static_cast<float>(m_graphics->GetHeight());

    XMMATRIX m = XMMatrixScaling(w, h, 1.0f)
        * XMMatrixTranslation(x, y, 0.0f)
        * XMMatrixOrthographicOffCenterLH(0.0f, W, H, 0.0f, 0.0f, 1.0f);
    DrawMesh(m_uiQuad, m, color);
}

void Game::Render()
{
    ID3D11DeviceContext* context = m_graphics->GetContext();

    CBFrame frame;
    XMStoreFloat4x4(&frame.view, XMMatrixTranspose(m_camera.GetView()));
    XMStoreFloat4x4(&frame.projection, XMMatrixTranspose(m_camera.GetProjection()));
    XMStoreFloat4(&frame.lightDir, XMVector3Normalize(XMVectorSet(0.5f, -1.0f, 0.3f, 0.0f)));
    const XMFLOAT3& cam = m_camera.GetPosition();
    frame.cameraPos = { cam.x, cam.y, cam.z, 1.0f };
    context->UpdateSubresource(m_cbFrame.Get(), 0, nullptr, &frame, 0, 0);

    ID3D11Buffer* cbs[] = { m_cbFrame.Get(), m_cbObject.Get() };
    context->VSSetConstantBuffers(0, 2, cbs);
    context->PSSetConstantBuffers(0, 2, cbs);

    RenderWorld();
    RenderUI();
}

void Game::RenderWorld()
{
    ID3D11DeviceContext* context = m_graphics->GetContext();
    context->OMSetDepthStencilState(nullptr, 0);
    context->RSSetState(nullptr);
    m_standardShader.Bind(context);

    DrawMesh(m_ground, XMMatrixIdentity(), { 1.0f, 1.0f, 1.0f, 1.0f });

    const XMFLOAT3& p = m_ball.GetPosition();
    DrawMesh(m_ballMesh, XMMatrixTranslation(p.x, p.y, p.z), { 1.0f, 1.0f, 1.0f, 1.0f });

    // 方向ガイド（1m間隔の黄色い点）
    if (m_state != State::Flying)
    {
        float dx = sinf(m_aimYaw);
        float dz = cosf(m_aimYaw);
        for (int i = 1; i <= 10; ++i)
        {
            float d = static_cast<float>(i);
            XMMATRIX world = XMMatrixScaling(0.3f, 0.3f, 0.3f)
                * XMMatrixTranslation(p.x + dx * d, Ball::RADIUS * 0.3f, p.z + dz * d);
            DrawMesh(m_ballMesh, world, { 1.0f, 0.9f, 0.2f, 1.0f });
        }
    }
}

void Game::RenderUI()
{
    ID3D11DeviceContext* context = m_graphics->GetContext();
    context->OMSetDepthStencilState(m_dsNoDepth.Get(), 0);
    context->RSSetState(m_rsNoCull.Get());
    m_uiShader.Bind(context);

    float H = static_cast<float>(m_graphics->GetHeight());
    float W = static_cast<float>(m_graphics->GetWidth());

    // クラブ表示（左下、選択中は白枠付き）
    for (int i = 0; i < Club::GetCount(); ++i)
    {
        float x = 20.0f + i * 70.0f;
        float y = H - 60.0f;
        if (i == m_club.GetIndex())
        {
            DrawRect(x - 3.0f, y - 3.0f, 66.0f, 46.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
            DrawRect(x, y, 60.0f, 40.0f, Club::GetData(i).color);
        }
        else
        {
            DrawRect(x, y, 60.0f, 40.0f, { 0.25f, 0.25f, 0.25f, 1.0f });
        }
    }

    // パワーゲージ（下中央、飛行中は非表示）
    if (m_state != State::Flying)
    {
        const float gw = 400.0f;
        const float gh = 28.0f;
        float gx = (W - gw) * 0.5f;
        float gy = H - 60.0f;

        DrawRect(gx - 3.0f, gy - 3.0f, gw + 6.0f, gh + 6.0f, { 0.1f, 0.1f, 0.1f, 1.0f });
        DrawRect(gx, gy, gw, gh, { 0.3f, 0.3f, 0.3f, 1.0f });

        float v = m_gauge.GetValue();
        if (v > 0.0f)
        {
            XMFLOAT4 color = LerpColor({ 0.2f, 0.9f, 0.2f, 1.0f }, { 0.95f, 0.2f, 0.1f, 1.0f }, v);
            DrawRect(gx, gy, gw * v, gh, color);
        }
    }

    // 次フレームの3D描画のために元に戻す
    context->OMSetDepthStencilState(nullptr, 0);
    context->RSSetState(nullptr);
}