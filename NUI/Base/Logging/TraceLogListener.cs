using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Diagnostics;

namespace NUI
{
    public class TraceLogListener : ILogListener
    {
        public void LogMessage(string source, string message, LogLevel level)
        {
            Trace.WriteLine("[" + source + "] " + message);
        }

        public bool WantsFilteredMessages { get { return false; } }
    }
}
