using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Nancy;

namespace InfinityScript
{
    public class WebHome : WebFrontend
    {
        public WebHome()
            : base()
        {
            Get["/"] = parameters =>
            {
                // fetch stat points
                var oneDayAgo = DateTime.UtcNow - new TimeSpan(24, 0, 0);
                var statPoints = WebBaseScript.Database.Table<PlayerCountStatisticPoint>().Where(p => p.Time >= oneDayAgo);

                return View["Index", new
                {
                    StatPoints = statPoints,
                    MapName = GameInterface.GetDvar("mapname", ""),
                    GameType = GameInterface.GetDvar("g_gametype", "dm"),
                    PlayerLimit = GameInterface.GetDvar("sv_maxclients", "0"),
                    PlayerCount = WebBaseScript.NumPlayers
                }];
            };
        }
    }
}
