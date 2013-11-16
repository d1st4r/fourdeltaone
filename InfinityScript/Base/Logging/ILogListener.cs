using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace InfinityScript
{
    public interface ILogListener
    {
        void LogMessage(string source, string message, LogLevel level);
        bool WantsFilteredMessages { get; }
    }
}
