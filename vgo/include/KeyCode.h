#pragma once

#define KeyCode_t uint32_t
#define KeyCode_None 0b00
#define KeyCode_Control 0b01
#define KeyCode_Left 0b10
#define KeyCode_ControlLeft 0b11
#define KeyCode_Middle 0b100

#ifdef __cplusplus
#include <cstdint>
namespace vgo
{
enum KeyCode : std::uint32_t
{
    None = KeyCode_None,
    Control = KeyCode_Control,
    Left = KeyCode_Left,
    ControlLeft = KeyCode_ControlLeft,
    Middle = KeyCode_Middle,
};
}
#endif