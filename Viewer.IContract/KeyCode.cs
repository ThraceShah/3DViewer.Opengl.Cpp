using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;

namespace Viewer.IContract
{
    public enum KeyCode : int
    {
        None = 0b00,
        Control = 0b01,
        Left = 0b10,
        ControlLeft = 0b11,
        Middle = 0b100,
    }
}
