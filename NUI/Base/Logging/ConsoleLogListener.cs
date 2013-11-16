using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;

namespace NUI
{
    public class ConsoleLogListener : ILogListener
    {
        public void LogMessage(string source, string message, LogLevel level)
        {
            var levelS = level.ToString().ToUpper();
            var sourceS = (source == string.Empty) ? string.Empty : ("[" + source + "]");

            var text = string.Format("{0} - {2}", sourceS, levelS, message);
            Console.WriteLine(text);
        }

        public bool WantsFilteredMessages { get { return true; } }
    }
}
