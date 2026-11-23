#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include "Core/Input.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/Shader.h"
#include "Graphics/Mesh.h"
#include "Graphics/Camera.h"
#include "Ball.h"
#include "Club.h"
#include "ShotGauge.h"

class Game
{
public:
    bool Initialize(GraphicsDevice* graphics);
    void Update(float deltaTime, const Input& input);
    void Render();
    void OnResize(int width, int height);

    int         GetStrokes()  const { return m_strokes; }
    const char* GetClubName() const { return m_club.GetCurrent().name; }

private:
    enum class State
    {
        Aiming,    // 方向・クラブ選択中
        Charging,  // ゲージ動作中
        Flying     // ボールが動いている
    };

    // HLSLのcbufferと並びを一致させる（16バイト境界）
    struct CBFrame
    {
        DirectX::XMFLOAT4X4 view;
        DirectX::XMFLOAT4X4 projection;
        DirectX::XMFLOAT4   lightDir;
        DirectX::XMFLOAT4   cameraPos;
    };
    struct CBObject
    {
        DirectX::XMFLOAT4X4 world;
        DirectX::XMFLOAT4   color;
    };

    bool CreateConstantBuffers();
    bool CreateRenderStates();

    void ResetHole();
    void UpdateCamera(float deltaTime);
    DirectX::XMFLOAT3 ComputeDesiredEye() const;

    void RenderWorld();
    void RenderUI();
    void DrawMesh(const Mesh& mesh, DirectX::FXMMATRIX world, const DirectX::XMFLOAT4& color);
    void DrawRect(float x, float y, float w, float h, const DirectX::XMFLOAT4& color);

    GraphicsDevice* m_graphics = nullptr;

    Shader m_standardShader;
    Shader m_uiShader;
    Mesh   m_ground;
    Mesh   m_ballMesh;
    Mesh   m_uiQuad;
    Camera m_camera;

    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_cbFrame;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_cbObject;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_dsNoDepth;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState>   m_rsNoCull;

    Ball      m_ball;
    Club      m_club;
    ShotGauge m_gauge;

    State m_state = State::Aiming;
    float m_aimYaw = 0.0f;
    int   m_strokes = 0;
    DirectX::XMFLOAT3 m_cameraEye = { 0.0f, 0.0f, 0.0f };
};