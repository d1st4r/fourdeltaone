using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;

namespace NUI
{
    public static class GameInterface
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Print(string s);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void ExecuteJS(string s);
    }
}
