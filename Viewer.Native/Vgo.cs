using System.Runtime.InteropServices;
using Viewer.IContract;

namespace Viewer.Native;


public unsafe sealed class Vgo
{
    [DllImport("vgo.dll", CallingConvention = CallingConvention.Cdecl,CharSet =CharSet.Ansi,EntryPoint = "init_gl_render")]
    public static extern int init_gl_render(nint getProcAddress, string rootDir);

    [DllImport("vgo.dll", CallingConvention = CallingConvention.Cdecl,EntryPoint = "realease_gl_render")]
    public static extern void realease_gl_render();

    [DllImport("vgo.dll", CallingConvention = CallingConvention.Cdecl,EntryPoint = "gl_control_resize")]
    public static extern void gl_control_resize(int width, int height);

    [DllImport("vgo.dll", CallingConvention = CallingConvention.Cdecl,EntryPoint = "gl_control_render")]
    public static extern void gl_control_render();

    [DllImport("vgo.dll", CallingConvention = CallingConvention.Cdecl,EntryPoint = "gl_control_update_geometry")]
    public static extern void gl_control_update_geometry(ref AsmGeometry asmGeometry);

    [DllImport("vgo.dll", CallingConvention = CallingConvention.Cdecl,EntryPoint = "gl_control_mouse_down")]
    public static extern void gl_control_mouse_down(int keycode, int x, int y);

    [DllImport("vgo.dll", CallingConvention = CallingConvention.Cdecl,EntryPoint = "gl_control_mouse_up")]
    public static extern void gl_control_mouse_up(int keycode, int x, int y);

    [DllImport("vgo.dll", CallingConvention = CallingConvention.Cdecl,EntryPoint = "gl_control_mouse_move")]
    public static extern void gl_control_mouse_move(int x, int y);

    [DllImport("vgo.dll", CallingConvention = CallingConvention.Cdecl,EntryPoint = "gl_control_mouse_wheel")]
    public static extern void gl_control_mouse_wheel(int delta);

    [DllImport("vgo.dll", CallingConvention = CallingConvention.Cdecl,EntryPoint = "gl_control_key_down")]
    public static extern void gl_control_key_down(int keycode);

    [DllImport("vgo.dll", CallingConvention = CallingConvention.Cdecl,EntryPoint = "gl_control_key_up")]
    public static extern void gl_control_key_up(int keycode);

}