#pragma once

#include "KeyCode.h"
#ifdef VGO_EXPORT
#define DLL_EXPORT extern "C" __declspec(dllexport)
#else
#define DLL_EXPORT extern "C" __declspec(dllimport)
#endif // VGO_EXPORT


typedef struct UnSafeArray
{
    void *ptr;
    int32_t len;
} UnSafeArray_t;

typedef struct Box
{
    float min[3];
    float max[3];
} Box_t;

typedef UnSafeArray_t UnSafeArray_vec4_t;
typedef UnSafeArray_t UnSafeArray_int32_t;

struct PartGeometry
{
    UnSafeArray_vec4_t Vertices;
    UnSafeArray_int32_t Indices;
    UnSafeArray_int32_t FaceIndices;
    UnSafeArray_int32_t ProtoFaceIndices;
    UnSafeArray_int32_t EdgeIndices;
    UnSafeArray_int32_t ProtoEdgeIndices;
    int32_t FaceStartIndex;
    int32_t FaceCount;
    int32_t EdgeStartIndex;
    int32_t EdgeCount;
    Box_t Box;
};

struct CompGeometry
{
    int32_t PartIndex;
    float CompMatrix[16];
};

typedef UnSafeArray_t UnSafeArray_PartGeometry_t;
typedef UnSafeArray_t UnSafeArray_CompGeometry_t;

struct AsmGeometry
{
    UnSafeArray_PartGeometry_t Parts;
    UnSafeArray_CompGeometry_t Components;
};


DLL_EXPORT int32_t init_gl_render(void *getProcAddress,char *rootDir);

DLL_EXPORT void realease_gl_render();

DLL_EXPORT void gl_control_resize(uint32_t width, uint32_t height);

DLL_EXPORT void gl_control_render();

DLL_EXPORT void gl_control_update_geometry(AsmGeometry *asmGeometry);

DLL_EXPORT void gl_control_mouse_down(KeyCode_t keycode, int32_t x, int32_t y);

DLL_EXPORT void gl_control_mouse_up(KeyCode_t keycode, int32_t x, int32_t y);

DLL_EXPORT void gl_control_mouse_move(int32_t x, int32_t y);

DLL_EXPORT void gl_control_mouse_wheel(int32_t delta);

DLL_EXPORT void gl_control_key_down(KeyCode_t keycode);

DLL_EXPORT void gl_control_key_up(KeyCode_t keycode);

