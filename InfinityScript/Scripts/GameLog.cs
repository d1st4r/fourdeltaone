using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace InfinityScript
{
    /*public class GameLog : BaseScript
    {
        private string _fileName;

        private Stream _file;
        private static StreamWriter _writer;

        private static DateTime _startTime;

        public GameLog()
        {
            _fileName = "scripts/" + Call<string>("getDvar", "g_log", "games_mp.log").Replace("/", "").Replace("\\", "");

            _file = File.Open(_fileName, FileMode.Append, FileAccess.Write, FileShare.ReadWrite);
            _writer = new StreamWriter(_file);

            _startTime = DateTime.Now;

            Write("------------------------------------------------------------");
            Write("InitGame: {0}", GameInterface.Dvar_InfoString_Big(1024));

            PlayerConnected += new Action<Entity>(GameLog_PlayerConnected);
        }

        void GameLog_PlayerConnected(Entity obj)
        {
            Write("J;{0};{1};{2}", obj.Call<string>("getGuid"), obj.EntRef, obj.GetField<string>("name"));
        }

        public override void OnPlayerDisconnect(Entity obj)
        {
            Write("Q;{0};{1};{2}", obj.Call<string>("getGuid"), obj.EntRef, obj.GetField<string>("name"));
        }

        public override void OnPlayerDamage(Entity player, Entity inflictor, Entity attacker, int damage, int dFlags, string mod, string weapon, Vector3 point, Vector3 dir, string hitLoc)
        {
            Write("D;{0};{1};{2};{3};{4};{5}", GetDamageDetails(player), GetDamageDetails(attacker), weapon, damage, mod, hitLoc);
        }

        public override void OnPlayerKilled(Entity player, Entity inflictor, Entity attacker, int damage, string mod, string weapon, Vector3 dir, string hitLoc)
        {
            Write("K;{0};{1};{2};{3};{4};{5}", GetDamageDetails(player), GetDamageDetails(attacker), weapon, damage, mod, hitLoc);
        }

        public override void OnSay(Entity player, string name, string message)
        {
            if (message.StartsWith("/"))
            {
                message = message.Substring(1);
            }

            Write("say;{0};{1};{2};{3}", player.Call<string>("getGuid"), player.EntRef, name, message);
        }

        public override void OnExitLevel()
        {
            Write("ExitLevel: executed");
        }

        private string GetDamageDetails(Entity player)
        {
            if (player == null || !player.IsPlayer)
            {
                return ";-1;world;world";
            }

            return string.Format("{0};{1};{2};{3}", player.Call<string>("getGuid"), player.EntRef, player.GetField<string>("sessionteam"), player.GetField<string>("name"));
        }

        public static void Write(string format, params object[] args)
        {
            _writer.Write(FormatTime(DateTime.Now - _startTime));
            _writer.Write(" ");
            _writer.WriteLine(string.Format(format, args));
            _writer.Flush();
        }

        private static string FormatTime(TimeSpan duration)
        {
            var secs = (int)duration.TotalSeconds;
            var time = string.Format("{0}:{1}", secs / 60, (secs % 60).ToString().PadLeft(2, '0'));
            return time.PadLeft(6, ' ');
        }
    }*/
}
