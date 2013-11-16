using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using SQLite;

namespace InfinityScript
{
    public class WebBaseScript : BaseScript
    {
        public static string HostName { get; set; }
        public static string RconPassword { get; set; }
        public static int OwnerID { get; set; }
        public static int NewOwnerID { get; set; }

        public static int NumPlayers { get; set; }

        public static SQLiteConnection Database { get; set; }

        static WebBaseScript()
        {
            Database = new SQLiteConnection("zone/" + GameInterface.GetDvar("sv_baseConfigName", "dummy") + ".iw4db", true);
            Database.CreateTable<PlayerCountStatisticPoint>();
        }

        public WebBaseScript() : base()
        {
            Action updateVars = delegate()
            {
                HostName = Call<string>("getdvar", "sv_hostname", "");
                RconPassword = Call<string>("getdvar", "rcon_password", "");

                if (NewOwnerID != 0)
                {
                    Call("setdvar", "sv_ownerUserID", NewOwnerID.ToString());
                    NewOwnerID = 0;
                }

                OwnerID = Call<int>("getdvarint", "sv_ownerUserID", 0);

                NumPlayers = Players.Count;
            };

            updateVars();

            OnInterval(2000, () =>
            {
                updateVars();

                return true;
            });

            Tick += PlayerCountOnTick;
        }

        private static uint GameTime { get; set; }
        private static uint LastStatTime { get; set; }
        private uint LastFrameTime { get; set; }

        private void PlayerCountOnTick()
        {
            var currentTime = (uint)Function.Call<int>("getTime");
            GameTime += (currentTime - LastFrameTime);
            LastFrameTime = currentTime;

            if (GameTime >= (LastStatTime + 240000) || LastStatTime == 0)
            {
                Database.Insert(new PlayerCountStatisticPoint() { Time = DateTime.UtcNow, PlayerCount = Players.Count });

                LastStatTime = GameTime;
            }
        }
    }
}
