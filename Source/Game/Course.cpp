#include "Course.h"
#include <Windows.h>
#include <fstream>
#include <cmath>
#include <algorithm>

using namespace DirectX;

bool Course::Load(ID3D11Device* device, const std::string& mapPath, const Model& cube)
{
    std::ifstream file(mapPath);
    if (!file)
    {
        MessageBoxA(nullptr, mapPath.c_str(), "Course file not found", MB_OK);
        return false;
    }

    std::vector<std::string> rows;
    std::string line;
    while (std::getline(file, line))
    {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#') continue;
        rows.push_back(line);
    }

    m_depth = static_cast<int>(rows.size());
    m_width = 0;
    for (const std::string& r : rows) m_width = (std::max)(m_width, static_cast<int>(r.size()));
    m_heights.assign(static_cast<size_t>(m_width) * m_depth, 0);
    m_trees.clear();

    bool hasStart = false;
    bool hasCup = false;

    for (int r = 0; r < m_depth; ++r)
    {
        for (int c = 0; c < m_width; ++c)
        {
            char ch = (c < static_cast<int>(rows[r].size())) ? rows[r][c] : '.';
            int h = 0;
            if (ch >= '1' && ch <= '9') h = ch - '0';
            else if (ch == 'T' || ch == 'S' || ch == 'G') h = 1;
            m_heights[static_cast<size_t>(r) * m_width + c] = h;

            if (ch == 'T')
            {
                float yaw = XMConvertToRadians(static_cast<float>((c * 73 + r * 151) % 360));
                m_trees.push_back({ CellCenter(c, r, h), yaw });
            }
            else if (ch == 'S') { m_start = CellCenter(c, r, h); hasStart = true; }
            else if (ch == 'G') { m_cup = CellCenter(c, r, h); hasCup = true; }
        }
    }

    if (!hasStart || !hasCup)
    {
        MessageBoxA(nullptr, "Map needs one 'S' and one 'G'", "Course error", MB_OK);
        return false;
    }
    return BuildBlockMesh(device, cube);
}

int Course::GetHeightAt(int col, int row) const
{
    if (col < 0 || col >= m_width || row < 0 || row >= m_depth) return 0;
    return m_heights[static_cast<size_t>(row) * m_width + col];
}

XMFLOAT3 Course::CellCenter(int col, int row, int height) const
{
    // 上の行ほど奥（+Z）
    float x = (col - (m_width - 1) * 0.5f) * BLOCK_SIZE;
    float z = ((m_depth - 1) * 0.5f - row) * BLOCK_SIZE;
    float y = (height > 0) ? (height - 1) * BLOCK_SIZE : VOID_HEIGHT;
    return { x, y, z };
}

float Course::GetGroundHeight(float x, float z) const
{
    int col = static_cast<int>(floorf(x / BLOCK_SIZE + m_width * 0.5f));
    int row = static_cast<int>(floorf(m_depth * 0.5f - z / BLOCK_SIZE));
    int h = GetHeightAt(col, row);
    return (h > 0) ? (h - 1) * BLOCK_SIZE : VOID_HEIGHT;
}

void Course::ResolveTreeCollision(XMFLOAT3& pos, XMFLOAT3& vel, float radius) const
{
    const float minDist = TRUNK_RADIUS + radius;
    for (const Tree& t : m_trees)
    {
        if (pos.y - radius > t.position.y + TRUNK_HEIGHT) continue;   // 幹より上を通過

        float dx = pos.x - t.position.x;
        float dz = pos.z - t.position.z;
        float d2 = dx * dx + dz * dz;
        if (d2 >= minDist * minDist || d2 < 1e-8f) continue;

        float d = sqrtf(d2);
        float nx = dx / d;
        float nz = dz / d;
        pos.x = t.position.x + nx * minDist;
        pos.z = t.position.z + nz * minDist;

        float vn = vel.x * nx + vel.z * nz;
        if (vn < 0.0f)
        {
            // 反発係数0.5で跳ね返す
            vel.x -= 1.5f * vn * nx;
            vel.z -= 1.5f * vn * nz;
        }
    }
}

bool Course::BuildBlockMesh(ID3D11Device* device, const Model& cube)
{
    const XMFLOAT3& bmin = cube.GetBoundsMin();
    const XMFLOAT3& bmax = cube.GetBoundsMax();
    XMFLOAT3 size = { (std::max)(bmax.x - bmin.x, 1e-4f),
                      (std::max)(bmax.y - bmin.y, 1e-4f),
                      (std::max)(bmax.z - bmin.z, 1e-4f) };

    std::vector<Mesh::Vertex> vertices;
    std::vector<uint32_t>     indices;

    for (int r = 0; r < m_depth; ++r)
    {
        for (int c = 0; c < m_width; ++c)
        {
            int h = GetHeightAt(c, r);
            if (h == 0) continue;

            XMFLOAT3 center = CellCenter(c, r, h);
            float columnTop = center.y;

            for (int layer = 0; layer < h; ++layer)   // layer 0 = 最上段
            {
                float blockTop = columnTop - layer * BLOCK_SIZE;
                XMFLOAT3 origin = { center.x - BLOCK_SIZE * 0.5f, blockTop - BLOCK_SIZE, center.z - BLOCK_SIZE * 0.5f };

                for (const Model::SubMesh& sm : cube.GetSubMeshes())
                {
                    for (size_t i = 0; i + 2 < sm.indices.size(); i += 3)
                    {
                        XMFLOAT3 lp[3];   // cube内の正規化座標 0～1
                        XMFLOAT3 wp[3];   // ワールド座標
                        for (int k = 0; k < 3; ++k)
                        {
                            const XMFLOAT3& p = sm.vertices[sm.indices[i + k]].position;
                            lp[k] = { std::clamp((p.x - bmin.x) / size.x, 0.0f, 1.0f),
                                      std::clamp((p.y - bmin.y) / size.y, 0.0f, 1.0f),
                                      std::clamp((p.z - bmin.z) / size.z, 0.0f, 1.0f) };
                            wp[k] = { origin.x + lp[k].x * BLOCK_SIZE,
                                      origin.y + lp[k].y * BLOCK_SIZE,
                                      origin.z + lp[k].z * BLOCK_SIZE };
                        }

                        // 面の向きは頂点法線ではなく三角形から求める（スムーズ法線のcube対策）
                        XMVECTOR e1 = XMVectorSubtract(XMLoadFloat3(&wp[1]), XMLoadFloat3(&wp[0]));
                        XMVECTOR e2 = XMVectorSubtract(XMLoadFloat3(&wp[2]), XMLoadFloat3(&wp[0]));
                        XMVECTOR nv = XMVector3Cross(e1, e2);
                        if (XMVectorGetX(XMVector3LengthSq(nv)) < 1e-10f) continue;
                        XMFLOAT3 n;
                        XMStoreFloat3(&n, XMVector3Normalize(nv));

                        bool isTop = n.y > 0.5f;
                        bool isSide = fabsf(n.y) <= 0.5f;

                        // 見えない面は作らない
                        if (n.y < -0.5f) continue;                 // 底面
                        if (isTop && layer > 0) continue;          // 上に積まれている
                        if (isSide)
                        {
                            int nc = c, nr = r;
                            if (fabsf(n.x) > fabsf(n.z)) nc += (n.x > 0.0f) ? 1 : -1;
                            else                         nr += (n.z > 0.0f) ? -1 : 1;   // +Zは上の行
                            int nh = GetHeightAt(nc, nr);
                            float neighborTop = (nh > 0) ? (nh - 1) * BLOCK_SIZE : VOID_HEIGHT;
                            if (neighborTop >= blockTop - 0.001f) continue;   // 隣の列で隠れる
                        }

                        uint32_t base = static_cast<uint32_t>(vertices.size());
                        for (int k = 0; k < 3; ++k)
                        {
                            Mesh::Vertex v;
                            v.position = wp[k];
                            v.normal = n;
                            v.color = { 1.0f, 1.0f, 1.0f, 1.0f };

                            // block_field.png：上部＝芝、中央＝芝の垂れ、下部＝土
                            if (isTop)
                            {
                                v.uv = { lp[k].x, 0.02f + lp[k].z * 0.23f };
                            }
                            else
                            {
                                float s = (fabsf(n.x) > fabsf(n.z)) ? lp[k].z : lp[k].x;
                                float t = 1.0f - lp[k].y;   // 0 = ブロックの上端
                                float vv = (layer == 0) ? 0.27f + t * 0.72f   // 芝の縁＋土
                                    : 0.62f + t * 0.37f;  // 土のみ
                                v.uv = { s, vv };
                            }
                            vertices.push_back(v);
                        }
                        indices.push_back(base);
                        indices.push_back(base + 1);
                        indices.push_back(base + 2);
                    }
                }
            }
        }
    }

    if (indices.empty())
    {
        MessageBoxA(nullptr, "No blocks in course", "Course error", MB_OK);
        return false;
    }
    return m_blockMesh.Create(device, vertices, indices);
}