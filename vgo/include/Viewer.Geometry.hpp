#pragma once
#include <cstdint>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/matrix.hpp>
#include <glm/trigonometric.hpp>
#include <stdexcept>

namespace vgo
{
constexpr glm::vec3 Vec3Unitx = glm::vec3(1.0f, 0.0f, 0.0f);
constexpr glm::vec3 Vec3Unity = glm::vec3(0.0f, 1.0f, 0.0f);
constexpr glm::vec3 Vec3Unitz = glm::vec3(0.0f, 0.0f, 1.0f);
constexpr glm::vec3 Vec3Zero = glm::vec3(0.0f, 0.0f, 0.0f);
constexpr glm::vec3 Vec3One = glm::vec3(1.0f, 1.0f, 1.0f);
constexpr glm::vec4 Vec4Zero = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
constexpr glm::vec4 Vec4One = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
constexpr glm::vec4 Vec4Unitx = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
constexpr glm::vec4 Vec4Unity = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
constexpr glm::vec4 Vec4Unitz = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
constexpr glm::mat4 Mat4Identity = glm::mat4(1.0f);
constexpr glm::mat4 Mat4Zero = glm::mat4(0.0f);

void init_root_dir(const char *rootDir);

template <typename T> struct UnSafeArray
{
  public:
    const T *data() const
    {
        return ptr;
    }

    T &operator[](int32_t index)
    {
        return ptr[index];
    }

    const T &operator[](int32_t index) const
    {
        return ptr[index];
    }

    int32_t size() const
    {
        return len;
    }

    T *begin()
    {
        return ptr;
    }

    const T *begin() const
    {
        return ptr;
    }

    T *end()
    {
        return ptr + len;
    }

    const T *end() const
    {
        return ptr + len;
    }

  private:
    T *ptr;
    int32_t len;
};

struct PartGeometry
{

  public:
    UnSafeArray<glm::vec4> Vertices;
    UnSafeArray<int32_t> Indices;
    UnSafeArray<int32_t> FaceIndices;
    UnSafeArray<int32_t> ProtoFaceIndices;
    UnSafeArray<int32_t> EdgeIndices;
    UnSafeArray<int32_t> ProtoEdgeIndices;
    int32_t FaceStartIndex;
    int32_t FaceCount;
    int32_t EdgeStartIndex;
    int32_t EdgeCount;
    glm::vec3 Box[2]; // [min, max]
};

struct CompGeometry
{
  public:
    int32_t PartIndex;
    glm::mat4 CompMatrix;
};

struct AsmGeometry
{
  public:
    UnSafeArray<PartGeometry> Parts;
    UnSafeArray<CompGeometry> Components;

    int32_t GetCompFirstIdByIndex(int32_t compIndex) const
    {
        if (compIndex > this->Components.size())
        {
            throw std::runtime_error("Index out of range");
        }
        int32_t id = 0;
        for (int32_t i = 0; i < compIndex; i++)
        {
            // FaceIndices和EdgeIndices里面最后一个元素并不代表一个面或者一条线
            const auto &comp = this->Components[i];
            const auto &part = this->Parts[comp.PartIndex];
            auto totalSize = part.FaceIndices.size() + part.EdgeIndices.size();
            id += totalSize - 2;
        }
        return id;
    }

    void CreateAsmWorldRH(float xSize, float ySize, glm::mat4 &world) const
    {
        glm::vec3 min(FLT_MAX, FLT_MAX, FLT_MAX);
        glm::vec3 max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
        for (const auto &comp : this->Components)
        {
            const auto &part = this->Parts[comp.PartIndex];
            const auto &trans = comp.CompMatrix;
            glm::vec3 transMin = trans * glm::vec4(part.Box[0], 1.0f);
            glm::vec3 transMax = trans * glm::vec4(part.Box[1], 1.0f);
            min = glm::min(min, transMin);
            max = glm::max(max, transMax);
        }
        glm::vec3 center = (min + max) * 0.5f;
        glm::vec3 size = max - min;
        float t = glm::min(size.z, glm::min(size.x, size.y));
        if (t == size.z)
        {
            CreateWorld(center, Vec3Unitz, Vec3Unity, world);
        }
        else if (t == size.y)
        {
            CreateWorld(center, Vec3Unity, Vec3Unitx, world);
        }
        else
        {
            CreateWorld(center, Vec3Unitx, Vec3Unitz, world);
        }

        auto tMin = world * glm::vec4(min, 1.0f);
        auto tMax = world * glm::vec4(max, 1.0f);
        glm::vec3 tSize = glm::abs(glm::vec3(tMax - tMin));
        float xScale = xSize / tSize.x;
        float yScale = ySize / tSize.y;
        float scale = glm::min(xScale, yScale);
        world = glm::scale(world,glm::vec3(scale,scale,scale));
        world[3] *= scale;
        world[3][3] = 1;
        // world = world * glm::scale(Mat4Identity, glm::vec3(scale));
    }

  private:
    void CreateWorld(const glm::vec3 &position, const glm::vec3 &forward, const glm::vec3 &up, glm::mat4 &world) const
    {
        glm::vec3 z = glm::normalize(forward);
        glm::vec3 x = glm::normalize(glm::cross(up, z));
        glm::vec3 y = glm::cross(z, x);
        glm::vec4 translate = glm::vec4(-glm::dot(x, position), -glm::dot(y, position), -glm::dot(z, position), 1.0f);
        //world = glm::mat4(glm::vec4(x, 0.0f), glm::vec4(y, 0.0f), glm::vec4(z, 0.0f), translate);
        world = glm::mat4(glm::vec4(x, -glm::dot(x, position)), glm::vec4(y, -glm::dot(y, position)),
                          glm::vec4(z, -glm::dot(z, position)), glm::vec4(0,0,0,1.0f));
        world = glm::transpose(world);
    }
};
} // namespace vgo
