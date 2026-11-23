#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <string>
#include <vector>
#include "Graphics/Mesh.h"
#include "Model.h"

class Course
{
public:
    struct Tree
    {
        DirectX::XMFLOAT3 position;   // 根元
        float yaw;
    };

    // mapPath: e.g. "Assets/Stages/course01.txt"
    bool Load(ID3D11Device* device, const std::string& mapPath, const Model& cube);

    // その位置の地面の高さ。ブロックが無ければ VOID_HEIGHT
    float GetGroundHeight(float x, float z) const;
    // 木の幹との衝突を解決する
    void ResolveTreeCollision(DirectX::XMFLOAT3& pos, DirectX::XMFLOAT3& vel, float radius) const;

    const Mesh& GetBlockMesh() const { return m_blockMesh; }
    const std::vector<Tree>& GetTrees() const { return m_trees; }
    const DirectX::XMFLOAT3& GetStartPosition() const { return m_start; }   // 地面上の点
    const DirectX::XMFLOAT3& GetCupPosition()   const { return m_cup; }     // 地面上の点

    static constexpr float BLOCK_SIZE = 2.0f;      // 1マス＝2m立方
    static constexpr float VOID_HEIGHT = -1000.0f;
    static constexpr float TRUNK_RADIUS = 0.4f;      // 木の当たり判定（幹）
    static constexpr float TRUNK_HEIGHT = 4.0f;

private:
    int  GetHeightAt(int col, int row) const;
    DirectX::XMFLOAT3 CellCenter(int col, int row, int height) const;
    bool BuildBlockMesh(ID3D11Device* device, const Model& cube);

    int m_width = 0;
    int m_depth = 0;
    std::vector<int>  m_heights;
    std::vector<Tree> m_trees;
    Mesh m_blockMesh;
    DirectX::XMFLOAT3 m_start = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 m_cup = { 0.0f, 0.0f, 0.0f };
};