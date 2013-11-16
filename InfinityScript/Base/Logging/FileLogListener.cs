using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;

namespace InfinityScript
{
    public class FileLogListener : ILogListener
    {
        private StreamWriter _writer;

        public FileLogListener(string filename, bool append)
        {
            try
            {
                _writer = new StreamWriter(filename, append);
            }
            catch (IOException)
            {
                _writer = null;
            }
        }

        public void LogMessage(string source, string message, LogLevel level)
        {
            if (_writer == null)
            {
                return;
            }

            var date = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss", CultureInfo.InvariantCulture);
            var levelS = level.ToString().ToUpper();
            var sourceS = (source == string.Empty) ? string.Empty : ("[" + source + "]");

            var text = string.Format("{0} - {1} - {2}: {3}", date, sourceS, levelS, message);
            _writer.WriteLine(text);
            _writer.Flush();
        }

        public bool WantsFilteredMessages { get { return true; } }
    }
}
