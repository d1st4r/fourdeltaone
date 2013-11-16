using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace InfinityScript
{
    public class GameLogListener : ILogListener
    {
        public void LogMessage(string source, string message, LogLevel level)
        {
            GameInterface.Print("[" + source + "] " + message + "\n");
        }

        public bool WantsFilteredMessages { get { return true; } }
    }
}
