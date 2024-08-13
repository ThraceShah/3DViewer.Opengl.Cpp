#include "GLRender.h"
#include "Viewer.Geometry.hpp"
#include "glad/glad.h"
#include <cstdint>
#include <fstream>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/matrix.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>

namespace vgo
{

static std::filesystem::path ROOT_DIR;

void init_root_dir(const char* rootDir)
{
    ROOT_DIR = rootDir;
}
KeyCode operator|(KeyCode a, KeyCode b)
{
    return static_cast<KeyCode>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
}

KeyCode operator&(KeyCode a, KeyCode b)
{
    return static_cast<KeyCode>(static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b));
}

KeyCode operator~(KeyCode a)
{
    return static_cast<KeyCode>(~static_cast<std::uint32_t>(a));
}

class Shader
{
  public:
    Shader(const std::filesystem::path &vertexShaderPath, const std::filesystem::path &fragmentShaderPath,
           const std::filesystem::path &geometryShaderPath = "")
    {
        _program = glCreateProgram();
        GLuint vertexShader;
        GLuint fragmentShader;
        try
        {
            vertexShader = LoadShader(vertexShaderPath, GL_VERTEX_SHADER);
            fragmentShader = LoadShader(fragmentShaderPath, GL_FRAGMENT_SHADER);
        }
        catch (const std::runtime_error &)
        {
            glDeleteProgram(_program);
            throw; // rethrow the exception
        }
        glAttachShader(_program, vertexShader);
        if (geometryShaderPath != "")
        {
            GLuint geometryShader;
            try
            {
                geometryShader = LoadShader(geometryShaderPath, GL_GEOMETRY_SHADER);
            }
            catch (const std::runtime_error &)
            {
                glDeleteProgram(_program);
                throw; // rethrow the exception
            }
            glAttachShader(_program, geometryShader);
        }
        glAttachShader(_program, fragmentShader);
        glLinkProgram(_program);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    void Use()
    {
        glUseProgram(_program);
    }

    void SetUniform(const std::string &name, const glm::mat4 &value)
    {
        GLint location = glGetUniformLocation(_program, name.c_str());
        if (location == -1)
        {
            throw std::runtime_error("Uniform not found: " + name);
        }
        glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
    }

    void SetUniform(const std::string &name, const glm::vec4 &value)
    {
        GLint location = glGetUniformLocation(_program, name.c_str());
        if (location == -1)
        {
            throw std::runtime_error("Uniform not found: " + name);
        }
        glUniform4fv(location, 1, &value[0]);
    }

    void SetUniform(const std::string &name, const glm::mat3 &value)
    {
        GLint location = glGetUniformLocation(_program, name.c_str());
        if (location == -1)
        {
            throw std::runtime_error("Uniform not found: " + name);
        }
        glUniformMatrix3fv(location, 1, GL_FALSE, &value[0][0]);
    }
    ~Shader()
    {
        glDeleteProgram(_program);
    }

  private:
    GLuint _program;

    GLuint LoadShader(const std::filesystem::path &path, GLenum type)
    {
        std::string src = ReadAllText(path);
        const char *c_src = src.c_str();
        GLuint handle = glCreateShader(type);
        glShaderSource(handle, 1, &c_src, NULL);
        glCompileShader(handle);
        GLint compileStatus;
        glGetShaderiv(handle, GL_COMPILE_STATUS, &compileStatus);
        if (compileStatus == GL_FALSE)
        {
            GLint infoLogLength;
            glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &infoLogLength);
            std::vector<GLchar> infoLog(infoLogLength);
            glGetShaderInfoLog(handle, infoLogLength, NULL, infoLog.data());
            std::string infoLogStr(infoLog.begin(), infoLog.end());
            throw std::runtime_error("Shader compilation failed: " + infoLogStr);
        }
        return handle;
    }

    std::string ReadAllText(const std::filesystem::path &path)
    {
        std::ifstream file(path);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
};

class PartBuffers
{
  public:
    PartBuffers() : length(0), vaos(nullptr), vbos(nullptr), ebos(nullptr)
    {
    }

    PartBuffers(const AsmGeometry &asmGeo)
        : length(asmGeo.Parts.size()), vaos(new GLuint[length]), vbos(new GLuint[length]), ebos(new GLuint[length])
    {
        glGenVertexArrays(length, vaos);
        glGenBuffers(length, vbos);
        glGenBuffers(length, ebos);
        for (int32_t i = 0; i < length; i++)
        {
            auto &part = asmGeo.Parts[i];
            glBindVertexArray(vaos[i]);
            glBindBuffer(GL_ARRAY_BUFFER, vbos[i]);
            glBufferData(GL_ARRAY_BUFFER, part.Vertices.size() * sizeof(glm::vec4), part.Vertices.begin(),
                         GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebos[i]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, part.Indices.size() * sizeof(int32_t), part.Indices.begin(),
                         GL_STATIC_DRAW);
            glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void *)0);
            glEnableVertexAttribArray(0);
        }
    }

    int32_t GetLength()
    {
        return this->length;
    }

    bool TryGetPartBuffer(int32_t partIndex, GLuint &vao, GLuint &ebo)
    {
        if (partIndex >= this->length)
        {
            return false;
        }
        vao = this->vaos[partIndex];
        ebo = this->ebos[partIndex];
        return true;
    }
    ~PartBuffers()
    {
        if (length == 0)
        {
            return;
        }
        glDeleteVertexArrays(length, vaos);
        glDeleteBuffers(length, vbos);
        glDeleteBuffers(length, ebos);
        delete[] vaos;
        delete[] vbos;
        delete[] ebos;
    }

  private:
    int32_t length;
    GLuint *vaos;
    GLuint *vbos;
    GLuint *ebos;
};

struct VSConstantBuffer
{
    glm::mat4 world;
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 translation;
    glm::mat4 origin;

    VSConstantBuffer()
        : world(Mat4Identity), view(glm::lookAt(glm::vec3(0, 0, -16.f), Vec3Zero, Vec3Unity)),
          projection(glm::perspectiveFov(1.570796327f, 800.0f, 600.0f, 0.1f, 100.0f)),
          translation(Mat4Identity),
          origin(Mat4Identity)
    {
    }

};

struct PSConstantBuffer
{
    glm::vec4 objColor = glm::vec4(0.5882353f, 0.5882353f, 0.5882353f, 1.0f);
};


class GlRender
{
  public:

    GlRender()
        : world(Mat4Identity), vsConstantBuffer(), psConstantBuffer(),
          faceShader(ROOT_DIR / "GLSL/faceShader.vert", ROOT_DIR / "GLSL/faceShader.frag",
                     ROOT_DIR / "GLSL/faceShader.geom"),
          lineShader(ROOT_DIR / "GLSL/lineShader.vert", ROOT_DIR / "GLSL/lineShader.frag"),
          pickShader(ROOT_DIR / "GLSL/pickShader.vert", ROOT_DIR / "GLSL/pickShader.frag"), geometry(), width(800),
          height(600)
    {
        glm::vec3 eye(0.0f, 0.0f, -20.0f);
        vsConstantBuffer.view = glm::lookAt(eye, Vec3Zero, Vec3Unity);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LINE_SMOOTH); // 启用线条平滑
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_POLYGON_OFFSET_FILL); // 开启深度偏移
        // 设置深度偏移,offset = m * factor + r * units,其中，m是多边形的最大深度斜率，r是能产生显著深度变化的最小值
        glPolygonOffset(1.0f, 0.5f);
    }

    void UpdateGeometry(const AsmGeometry &asmGeometry)
    {
        keyCode = KeyCode::None;
        zoom = ZOOM;
        mouseXOffset = 0;
        mouseYOffset = 0;

        vsConstantBuffer = VSConstantBuffer();
        UpdateProjMatrix();
        asmGeometry.CreateAsmWorldRH(16, 12, world);
        partBuffers = std::make_unique<PartBuffers>(asmGeometry);
        geometry = asmGeometry;
    }

    void GLControlResize(GLuint width, GLuint height)
    {
        this->width = width;
        this->height = height;
        glViewport(0, 0, width, height);
        UpdateProjMatrix();
    }

    void Render()
    {
        if (geometry.Parts.size() == 0 && first)
        {
            first = false;
            return;
        }
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto xRadians = glm::radians(mouseXOffset);
        auto yRadians = glm::radians(mouseYOffset);
        glm::mat4 W =
            glm::rotate(Mat4Identity, yRadians, Vec3Unitx) * glm::rotate(Mat4Identity, xRadians, Vec3Unity) * world;

        vsConstantBuffer.world = W;
        psConstantBuffer.objColor = glm::vec4(0.5882353f, 0.5882353f, 0.5882353f, 1.0f);

        glEnable(GL_POLYGON_OFFSET_FILL);
        faceShader.Use();
        glm::mat4 WI = glm::inverse(W);
        auto WIT = glm::transpose(WI);
        auto WIT3x3 = glm::mat3(WIT);
        faceShader.SetUniform("g_WIT", WIT3x3);
        faceShader.SetUniform("g_World", vsConstantBuffer.world);
        faceShader.SetUniform("g_View", vsConstantBuffer.view);
        faceShader.SetUniform("g_Proj", vsConstantBuffer.projection);
        faceShader.SetUniform("g_Translation", vsConstantBuffer.translation);
        faceShader.SetUniform("objectColor", psConstantBuffer.objColor);
        auto compsCount = geometry.Components.size();
        for (int32_t i = 0; i < compsCount; i++)
        {
            auto &comp = geometry.Components[i];
            auto &part = geometry.Parts[comp.PartIndex];
            faceShader.SetUniform("g_Origin", comp.CompMatrix);
            GLuint vao, ebo;
            if (partBuffers->TryGetPartBuffer(comp.PartIndex, vao, ebo))
            {
                glBindVertexArray(vao);
                glDrawElements(GL_TRIANGLES, part.FaceCount, GL_UNSIGNED_INT,
                               (void *)(part.FaceStartIndex * sizeof(int32_t)));
            }
        }

        //绘制边线
        //glDisable(GL_POLYGON_OFFSET_FILL);
        //lineShader.Use();
        //psConstantBuffer.objColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        //lineShader.SetUniform("g_World", vsConstantBuffer.world);
        //lineShader.SetUniform("g_View", vsConstantBuffer.view);
        //lineShader.SetUniform("g_Proj", vsConstantBuffer.projection);
        //lineShader.SetUniform("g_Translation", vsConstantBuffer.translation);
        //lineShader.SetUniform("objectColor", psConstantBuffer.objColor);
        //for (int32_t i = 0; i < compsCount; i++)
        //{
        //    auto &comp = geometry.Components[i];
        //    auto &part = geometry.Parts[comp.PartIndex];
        //    lineShader.SetUniform("g_Origin", comp.CompMatrix);
        //    GLuint vao, ebo;
        //    if (partBuffers->TryGetPartBuffer(comp.PartIndex, vao, ebo))
        //    {
        //        glBindVertexArray(vao);
        //        glDrawElements(GL_LINES, part.EdgeCount, GL_UNSIGNED_INT,
        //                       (void *)(part.EdgeStartIndex * sizeof(int32_t)));
        //    }
        //}
    }

    void MouseDown(KeyCode code, int32_t x, int32_t y)
    {
        lastX = static_cast<float>(x);
        lastY = static_cast<float>(y);
        keyCode = keyCode | code;
    }

    void MouseUp(KeyCode code, int32_t x, int32_t y)
    {
        if (code == KeyCode::Left && keyCode == KeyCode::Left)
        {
            // highlight
        }
        keyCode = keyCode & (~code);
    }

    void MouseMove(int32_t x, int32_t y)
    {
        if (keyCode != KeyCode::Middle && keyCode != KeyCode::ControlLeft)
        {
            return;
        }
        auto xPosIn = x;
        auto yPosIn = y;

        float xPos = (float)xPosIn;
        float yPos = (float)yPosIn;

        float xOffset = xPos - lastX;
        float yOffset = lastY - yPos; // reversed since y-coordinates go from bottom to top

        lastX = xPos;
        lastY = yPos;
        switch (keyCode)
        {
        case KeyCode::Middle:
            ProcessMouseMovement(xOffset, yOffset);
            break;
        case KeyCode::ControlLeft:
            vsConstantBuffer.translation[0][3] +=xOffset * 0.002f;
            vsConstantBuffer.translation[1][3] += yOffset * 0.002f;
            break;
        default:
            break;
        }
    }

    void MouseWheel(int32_t delta)
    {
        ProcessMouseScroll(delta * 0.01f);
        UpdateProjMatrix();
    }

    void KeyDown(KeyCode code)
    {
        keyCode = keyCode | code;
    }

    void KeyUp(KeyCode code)
    {
        keyCode = keyCode & (~code);
    }

    ~GlRender()
    {
    }

  private:
    const float ZOOM = 60.f;
    KeyCode keyCode = KeyCode::None;
    float zoom = ZOOM;
    float mouseXOffset = 0;
    float mouseYOffset = 0;

    glm::mat4 world;

    VSConstantBuffer vsConstantBuffer;
    PSConstantBuffer psConstantBuffer;

    Shader faceShader;

    Shader lineShader;

    Shader pickShader;

    std::unique_ptr<PartBuffers> partBuffers;

    AsmGeometry geometry;

    GLuint width;

    GLuint height;

    bool first = true;

    float lastX = 0.0f;

    float lastY = 0.0f;

    void UpdateProjMatrix()
    {
        auto radians = glm::radians(zoom);
        auto aspectRatio = glm::max(width, 1u) / glm::max(height, 1u);
        vsConstantBuffer.projection = glm::perspective(radians, (float)aspectRatio, 0.1f, 100.0f);
    }

    void ProcessMouseScroll(float yoffset)
    {
        // opengl这里的取值范围和dx是不一样的,opengl不能小于0,但是dx可以,这和二者的投影算法不同有关
        zoom -= yoffset;
        if (zoom <= -0.0f)
            zoom = 0.001f;
        if (zoom >= 180.0f)
            zoom = 179.990f;
    }
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
    {

        mouseXOffset += xoffset * 0.8f;
        mouseYOffset += yoffset * 0.8f;
    }
};
} // namespace vgo

static vgo::GlRender *glRender = nullptr;

int32_t init_gl_render(void *getProcAddress,char *rootDir)
{
    std::cout << "rootDir: " << rootDir << "\n";
    vgo::init_root_dir(rootDir);
    if (!gladLoadGLLoader((GLADloadproc)getProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    if (glRender == nullptr)
    {
        glRender = new vgo::GlRender();
    }
    return 0;
}

void realease_gl_render()
{
    if (glRender != nullptr)
    {
        delete glRender;
        glRender = nullptr;
    }
}

void gl_control_resize(uint32_t width, uint32_t height)
{
    glRender->GLControlResize(width, height);
}

void gl_control_render()
{
    glRender->Render();
}

void gl_control_update_geometry(AsmGeometry *asmgeo)
{
    auto asmGeometry = reinterpret_cast<vgo::AsmGeometry *>(asmgeo);
    glRender->UpdateGeometry(*asmGeometry);
}

void gl_control_mouse_down(KeyCode_t keycode, int32_t x, int32_t y)
{
    glRender->MouseDown((vgo::KeyCode)keycode, x, y);
}

void gl_control_mouse_up(KeyCode_t keycode, int32_t x, int32_t y)
{
    glRender->MouseUp((vgo::KeyCode)keycode, x, y);
}

void gl_control_mouse_move(int32_t x, int32_t y)
{
    glRender->MouseMove(x, y);
}

void gl_control_mouse_wheel(int32_t delta)
{
    glRender->MouseWheel(delta);
}

void gl_control_key_down(KeyCode_t keycode)
{
    glRender->KeyDown((vgo::KeyCode)keycode);
}

void gl_control_key_up(KeyCode_t keycode)
{
    glRender->KeyUp((vgo::KeyCode)keycode);
}
