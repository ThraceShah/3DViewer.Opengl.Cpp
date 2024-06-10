using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Threading;
using System.Timers;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Media;
using Avalonia.OpenGL;
using Avalonia.OpenGL.Controls;
using Avalonia.Platform.Storage;
using Avalonia.Rendering;
using Avalonia.Rendering.Composition;
using Avalonia.Threading;
using Avalonia.VisualTree;
using Viewer.Avalonia.Entry.ViewModels;
using Viewer.IContract;
using Viewer.Native;
using static System.Collections.Specialized.BitVector32;

namespace Viewer.Avalonia.Entry.Views;

public partial class OpenglView : UserControl
{
    public OpenglView()
    {
        InitializeComponent();
        //必须要设置背景色才能触发点击测试
        this.Background = Brushes.Transparent;
        this.RegKeyAction();
    }

    double scale = 1;

    public void UpdateScale(double newScale)
    {
        this.scale = newScale;
        this.GLControl.UpdateScale(newScale);
    }


    public void RegKeyAction()
    {

        this.PointerPressed += (sender, args) =>
        {
            if (args.GetCurrentPoint(this).Properties.IsMiddleButtonPressed)
            {
                // 记录当前坐标
                var p = args.GetPosition(this);
                args.Handled = true;
                Vgo.gl_control_mouse_down((int)KeyCode.Middle, (int)p.X, (int)p.Y);
            }
            if (args.GetCurrentPoint(this).Properties.IsLeftButtonPressed)
            {
                // 记录当前坐标
                var p = args.GetPosition(this);
                args.Handled = true;
                Vgo.gl_control_mouse_down((int)KeyCode.Left, (int)p.X, (int)p.Y);
            }
        };
        this.PointerReleased += (sender, args) =>
        {
            if (args.GetCurrentPoint(this).Properties.PointerUpdateKind==PointerUpdateKind.MiddleButtonReleased)
            {
                // 记录当前坐标
                var p = args.GetPosition(this);
                args.Handled = true;
                Vgo.gl_control_mouse_up((int)KeyCode.Middle, (int)p.X, (int)p.Y);
                GLControl.ResetWatch();
            }
            if (args.GetCurrentPoint(this).Properties.PointerUpdateKind == PointerUpdateKind.LeftButtonReleased)
            {
                // 记录当前坐标
                var p = args.GetPosition(this);
                args.Handled = true;
                GLControl.LeftReleased((int)(p.X * scale), (int)(p.Y * scale));
                GLControl.ResetWatch();
            }
        };
        this.PointerMoved += (sender, args) =>
        {
            // 记录当前坐标
            var p = args.GetPosition(this);
            args.Handled = true;
            // GLControl.GlRender.MouseMove((int)p.X, (int)p.Y);
            Vgo.gl_control_mouse_move((int)p.X, (int)p.Y);
        };
        this.PointerWheelChanged += (sender, args) =>
        {
            var p = args.GetCurrentPoint(this);
            Vgo.gl_control_mouse_wheel((int)args.Delta.Y * 100);
            GLControl.ResetWatch();
        };

    }

}

public class OpenGlPageControl : OpenGlControlBase
{

    bool inited = false;

    AsmGeometry asmGeometry;

    AsmGeometry? lastGeometry;
    bool updateAsm = false;

    uint width = 0;

    uint height = 0;

    bool updateSize = false;

    double scale = 1;

    Stopwatch sw = new Stopwatch();

    public void ResetWatch()
    {
        sw.Reset();
    }

    [UnsafeAccessor(UnsafeAccessorKind.Field, Name = "_getProcAddress")]
    static extern ref Func<string, IntPtr> GetValue(GlInterface gl);


    protected override unsafe void OnOpenGlInit(GlInterface gl)
    {
        if(inited is false)
        {
            procApi = GetValue(gl);
            unsafe
            {
                delegate* unmanaged<sbyte*, nint> procPtr = &GetProcAddressUnmanage;
                int r = Vgo.init_gl_render((nint)procPtr,AppContext.BaseDirectory);
                if (r != 0)
                {
                    throw new Exception("init_gl_render failed");
                }
                inited = true;
            }
            sw.Start();
        }
    }

    static Func<string, IntPtr> procApi;

    [UnmanagedCallersOnly]
    private static unsafe nint GetProcAddressUnmanage(sbyte* str)
    {
        return procApi(new(str));
    }

    protected override unsafe void OnOpenGlRender(GlInterface gl, int fb)
    {
        //执行opengl相关操作的函数，必须在OnOpenGlRender或OnOpenGlInit内执行
        if(sw.ElapsedMilliseconds>1000)
        {
            this.RequestNextFrameRendering();
            return;
        }
        if(leftReleased)
        {
            leftReleased = false;
            Vgo.gl_control_mouse_up((int)KeyCode.Left, nx, ny);
        }
        if(updateAsm)
        {
            updateAsm = false;
            Vgo.gl_control_update_geometry(ref asmGeometry);
            if(lastGeometry.HasValue)
            {
                lastGeometry?.Dispose();
                lastGeometry = null;
            }
        }
        if(updateSize)
        {
            updateSize = false;
            Vgo.gl_control_resize((int)width, (int)height);
        }
        Vgo.gl_control_render();
        this.RequestNextFrameRendering();
    }

    public unsafe void UpdateGeometry(ref AsmGeometry geometry)
    {
        this.lastGeometry=this.asmGeometry;
        this.asmGeometry = geometry;
        this.updateAsm = true;
        ResetWatch();
    }



    protected override void OnSizeChanged(SizeChangedEventArgs e)
    {
        base.OnSizeChanged(e);
        this.width = (uint)(e.NewSize.Width*this.scale);
        this.height = (uint)(e.NewSize.Height*this.scale);
        updateSize = true;
        ResetWatch();
    }

    public void UpdateScale(double newScale)
    {
        this.scale = newScale;
        this.width = (uint)(this.width * this.scale);
        this.height = (uint)(this.height * this.scale);
        updateSize = true;
        ResetWatch();
    }

    int nx = 0;
    int ny = 0;
    bool leftReleased = false;
    public void LeftReleased(int nx,int ny)
    {
        this.nx = nx;
        this.ny = ny;
        leftReleased = true;
        ResetWatch();
    }

}