using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace NUI
{
    public static class SHManager
    {
        private static ServerPage serverPage;

        public static void InitializeOnce()
        {
            Log.Initialize(LogLevel.All);
            Log.AddListener(new FileLogListener("scripts\\NUI.log", false));
            //Log.AddListener(new TraceLogListener());
            Log.AddListener(new GameLogListener());

            Log.Info("Initializing NUIm\n");

            try
            {
                serverPage = new ServerPage();
                serverPage.Initialize();
            }
            catch (Exception ex)
            {
                Log.Error(ex);
            }

            Log.Info("Initializing NUI done, continuing boot...");
        }
    }
}
