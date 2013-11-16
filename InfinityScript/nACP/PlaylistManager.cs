using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace InfinityScript
{
    public class PlaylistManager : BaseScript
    {
        private static Playlist Playlist { get; set; }
        private static GameType GameType { get; set; }
        private static List<PlaylistEntry> CurrentPlaylist = new List<PlaylistEntry>();
        private static PlaylistSet PlaylistSet { get; set; }
        private static PlaylistManager Manager;

        private static Dictionary<string, string> Rules = new Dictionary<string, string>();
        private static Dictionary<string, string> Values = new Dictionary<string, string>();

        public PlaylistManager() : base()
        {
            Manager = this;

            //if (Call<int>("getDvarInt", "playlist_enabled", 1) == 0)
            if (_.GetDvarInt("playlist_enabled", 1) == 0)
            {
                return;
            }

            OnNotify("match_ending_very_soon", () =>
            {
                SayNextMap();
            });

            OnInterval(10000, () =>
            {
                ApplyVariables(Rules);

                if (GameType != null)
                {
                    ApplyVariables(GameType.Rules);
                }

                if (Playlist != null)
                {
                    ApplyVariables(Playlist.Rules);
                }

                return true;
            });
        }

        public override BaseScript.EventEat OnSay3(Entity player, BaseScript.ChatType type, string name, ref string message)
        {
            if (_.GetDvarInt("playlist_enabled", 1) == 0)
            {
                return EventEat.EatNone;
            }

            if (message.StartsWith("!nm") || message.StartsWith("!nextmap"))
            {
                SayNextMap();
            }

            return EventEat.EatNone;
        }

        private void SayNextMap()
        {
            if (CurrentPlaylist.Count == 0)
            {
                BuildPlaylist(GameInterface.GetDvar("mapname", ""));
            }

            if (CurrentPlaylist.Count > 0)
            {
                Utilities.SayAll("Next map: " + CurrentPlaylist[0].Map + " (" + CurrentPlaylist[0].GameType.Name + ")");
            }
        }

        public static void RotateMap()
        {
            try
            {
                if (CurrentPlaylist.Count == 0)
                {
                    BuildPlaylist(GameInterface.GetDvar("mapname", ""));

                    if (CurrentPlaylist.Count == 0)
                    {
                        Utilities.ExecuteCommand("map_restart");
                        return;
                    }
                }

                var nextEntry = CurrentPlaylist[0];
                CurrentPlaylist.RemoveAt(0);

                GameType = nextEntry.GameType;

                ApplyVariables(Rules);
                ApplyVariables(Values);
                ApplyVariables(nextEntry.GameType.Rules);
                ApplyVariables(nextEntry.GameType.Values);
                ApplyVariables(Playlist.Rules);
                ApplyVariables(Playlist.Values);

                Utilities.ExecuteCommand("ui_gametype {0}; g_gametype {1}; map {2}; ", nextEntry.GameType.Script, nextEntry.GameType.Script, nextEntry.Map);
            }
            catch (Exception ex)
            {
                Log.Error("Error during map rotation: " + ex.ToString());

                Utilities.ExecuteCommand("map_restart");
            }
        }

        private static void ApplyVariables(IDictionary<string, string> dict)
        {
            foreach (var entry in dict)
            {
                //Manager.Call("setdvar", entry.Key, entry.Value);
                Manager._.SetDvar(entry.Key, entry.Value);
            }
        }

        private static void BuildPlaylist(string currentMap = "")
        {
            // generate a list containing unique maps, with entries the number of times as the weight indicates
            var list = new List<PlaylistEntry>();
            var mapsDone = new HashSet<string>();

            var rnd = new Random();

            var curPlaylistId = int.Parse(GameInterface.GetDvar("playlist", "0"));

            if (curPlaylistId < 0 || curPlaylistId > PlaylistSet.Playlists.Length)
            {
                Log.Debug("curPlaylistId seems invalid.");
                return;
            }

            var curPlaylist = PlaylistSet.Playlists[curPlaylistId];

            if (curPlaylist == null)
            {
                Log.Debug("curPlaylistId seems null.");
                return;
            }

            Playlist = curPlaylist;

            var newEntries = from e in curPlaylist.Entries
                             orderby rnd.Next()
                             select e;

            foreach (var entry in newEntries)
            {
                if (!mapsDone.Contains(entry.Map))
                {
                    list.Add(entry);
                    mapsDone.Add(entry.Map);
                }
            }

            // duplicate the entries with a >1 weight
            var duplicateList = list.GetRange(0, list.Count);

            foreach (var entry in duplicateList)
            {
                if (entry.Weight > 1)
                {
                    for (int i = 1; i < entry.Weight; i++)
                    {
                        list.Add(entry);
                    }
                }
            }

            // shuffle the list, again
            list = (from e in list
                    orderby rnd.Next()
                    select e).ToList();

            // remove current map from start of list, so we won't rotate to the same map
            if (list.Count > 0 && list[0].Map == currentMap)
            {
                list.RemoveAt(0);
            }

            CurrentPlaylist = list;
        }

        public static void ParsePlaylists(string playlistFile)
        {
            var tokenizer = new Tokenizer(playlistFile);
            var token = "";

            var playlistData = new PlaylistSet();

            Playlist curPlaylist = null;
            GameType curGameType = null;

            var line = 0;

            do 
            {
                token = tokenizer.ReadToken();
                token = token.ToLowerInvariant();

                switch (token)
                {
                    case "version":
                        var version = tokenizer.ReadToken(false);

                        if (string.IsNullOrWhiteSpace(version))
                        {
                            Log.Write(LogLevel.Error, "Playlist error: Expected version number after version command", line);
                            break;
                        }

                        int versionNum = 0;
                        int.TryParse(version, out versionNum);

                        if (versionNum <= 0)
                        {
                            Log.Write(LogLevel.Error, "Playlist error: Invalid version number {1}", line, versionNum);
                            break;
                        }

                        playlistData.Version = versionNum;
                        break;

                    case "playlist":
                        {
                            var id = tokenizer.ReadToken(false);

                            if (string.IsNullOrWhiteSpace(id))
                            {
                                Log.Write(LogLevel.Error, "Playlist error: 'playlist' command needs to be followed by a fucking number", line);
                                break;
                            }

                            int pid = -1;
                            int.TryParse(id, out pid);

                            if (pid < 0 || pid > playlistData.Playlists.Length)
                            {
                                Log.Write(LogLevel.Error, "Playlist error: 'playlist' command needs to be followed by a number between {1} and {2}", line, 0, playlistData.Playlists.Length);
                                break;
                            }

                            var playlist = new Playlist();
                            curGameType = null;
                            curPlaylist = playlist;

                            playlistData.Playlists[pid] = playlist;

                            break;
                        }
                    case "gametype":
                        {
                            var id = tokenizer.ReadToken(false);

                            if (string.IsNullOrWhiteSpace(id))
                            {
                                Log.Write(LogLevel.Error, "Playlist error: 'playlist' command needs to be followed by a fucking number", line);
                                break;
                            }

                            var gametype = new GameType() { Id = id };
                            curGameType = gametype;
                            curPlaylist = null;

                            playlistData.GameTypes[id] = curGameType;

                            break;
                        }
                    case "script":
                        {
                            var script = tokenizer.ReadToken(false);

                            if (string.IsNullOrWhiteSpace(script))
                            {
                                Log.Write(LogLevel.Error, "Playlist error: 'script' command needs to be followed by a name", line);
                                break;
                            }

                            if (curGameType != null)
                            {
                                curGameType.Script = script;
                            }
                            else
                            {
                                Log.Write(LogLevel.Error, "Playlist error: not in a gametype during script command", line);
                                break;
                            }

                            break;
                        }
                    case "name":
                        {
                            var name = tokenizer.ReadToken(false);

                            if (string.IsNullOrWhiteSpace(name))
                            {
                                Log.Write(LogLevel.Error, "Playlist error: 'name' command needs to be followed by a name", line);
                                break;
                            }

                            if (curPlaylist != null)
                            {
                                curPlaylist.Name = name;
                            }
                            else if (curGameType != null)
                            {
                                curGameType.Name = name;
                            }
                            else
                            {
                                Log.Write(LogLevel.Error, "Playlist error: not in a gametype/playlist during name command", line);
                                break;
                            }

                            break;
                        }
                    case "description":
                        {
                            var description = tokenizer.ReadToken(false);

                            if (string.IsNullOrWhiteSpace(description))
                            {
                                Log.Write(LogLevel.Error, "Playlist error: 'description' command needs to be followed by a name", line);
                                break;
                            }

                            if (curPlaylist != null)
                            {
                                curPlaylist.Description = description;
                            }
                            else
                            {
                                Log.Write(LogLevel.Error, "Playlist error: not in a playlist during description command", line);
                                break;
                            }

                            break;
                        }
                    case "icon":
                        {
                            var icon = tokenizer.ReadToken(false);

                            if (string.IsNullOrWhiteSpace(icon))
                            {
                                Log.Write(LogLevel.Error, "Playlist error: 'icon' command needs to be followed by a name", line);
                                break;
                            }

                            if (curPlaylist != null)
                            {
                                curPlaylist.Image = icon;
                            }
                            else
                            {
                                Log.Write(LogLevel.Error, "Playlist error: not in a playlist during icon command", line);
                                break;
                            }

                            break;
                        }
                    case "set":
                        {
                            var varName = tokenizer.ReadToken(false);
                            var varValue = tokenizer.ReadToken(false);

                            if (string.IsNullOrWhiteSpace(varName) || string.IsNullOrWhiteSpace(varValue))
                            {
                                Log.Write(LogLevel.Error, "Playlist error: 'set' command needs to be followed by a variable name/value", line);
                                break;
                            }

                            if (curPlaylist != null)
                            {
                                curPlaylist.Values[varName] = varValue;
                            }
                            else if (curGameType != null)
                            {
                                curGameType.Values[varName] = varValue;
                            }
                            else
                            {
                                Values[varName] = varValue;
                            }

                            break;
                        }
                    case "rule":
                        {
                            var varName = tokenizer.ReadToken(false);
                            var varValue = tokenizer.ReadToken(false);

                            if (string.IsNullOrWhiteSpace(varName) || string.IsNullOrWhiteSpace(varValue))
                            {
                                Log.Write(LogLevel.Error, "Playlist error: 'rule' command needs to be followed by a variable name/value", line);
                                break;
                            }

                            if (curPlaylist != null)
                            {
                                curPlaylist.Rules[varName] = varValue;
                            }
                            else if (curGameType != null)
                            {
                                curGameType.Rules[varName] = varValue;
                            }
                            else
                            {
                                Rules[varName] = varValue;
                            }

                            break;
                        }
                    default:
                        {
                            if (curPlaylist != null)
                            {
                                var datas = token.Split(',');

                                if (datas.Length == 3)
                                {
                                    var entry = new PlaylistEntry();
                                    var gameType = datas[1];

                                    GameType gt;
                                    if (!playlistData.GameTypes.TryGetValue(gameType, out gt))
                                    {
                                        Log.Write(LogLevel.Error, "Playlist error: invalid gametype {1}", line, gameType);
                                        break;
                                    }

                                    int weight = 1;
                                    int.TryParse(datas[2], out weight);

                                    entry.GameType = gt;
                                    entry.Map = datas[0];
                                    entry.Weight = weight;

                                    curPlaylist.Entries.Add(entry);

                                    break;
                                }
                            }

                            if (token != "")
                            {
                                Log.Write(LogLevel.Error, "Playlist error: invalid line {1}", line, token);
                            }

                            break;
                        }
                }
            } while (token != "");

            PlaylistSet = playlistData;
        }
    }

    internal class PlaylistSet
    {
        public PlaylistSet()
        {
            GameTypes = new Dictionary<string, GameType>();
            Playlists = new Playlist[48];
        }

        public int Version { get; set; }
        public Dictionary<string, GameType> GameTypes { get; set; }
        public Playlist[] Playlists { get; set; }
    }

    internal class PlaylistEntry
    {
        public GameType GameType { get; set; }
        public string Map { get; set; }
        public int Weight { get; set; }
    }

    internal class Playlist
    {
        public Playlist()
        {
            Values = new Dictionary<string, string>();
            Rules = new Dictionary<string, string>();

            Entries = new List<PlaylistEntry>();
        }

        public string Name { get; set; }
        public string Description { get; set; }
        public string Image { get; set; }

        public Dictionary<string, string> Values { get; set; }
        public Dictionary<string, string> Rules { get; set; }

        public List<PlaylistEntry> Entries { get; set; }
    }

    internal class GameType
    {
        public GameType()
        {
            Values = new Dictionary<string, string>();
            Rules = new Dictionary<string, string>();
        }

        public string Id { get; set; }
        public string Name { get; set; }
        public string Script { get; set; }

        public Dictionary<string, string> Values { get; set; }
        public Dictionary<string, string> Rules { get; set; }
    }
}
