using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;

using Newtonsoft.Json.Linq;

namespace NUI
{
    // manages server 'recommendations' on the home page
    class ServerInfo
    {
        public IPEndPoint EndPoint { get; set; }

        public Dictionary<string, string> Info { get; set; }

        public bool Queried { get; set; }
        public int QueryTime { get; set; }
        public int Ping { get; set; }

        // volatile score: player count; team score divergence
        public int GetVolatileScore()
        {
            int score = -100;

            if (!Info.ContainsKey("clients") || !Info.ContainsKey("sv_maxclients"))
            {
                return -100000;
            }

            int clients;
            int maxClients;
            if (!int.TryParse(Info["sv_maxclients"], out maxClients) || !int.TryParse(Info["clients"], out clients))
            {
                return -100000;
            }

            if (clients >= maxClients || clients == 0)
            {
                return -100000;
            }

            if (maxClients >= 12)
            {
                if (clients >= (maxClients - (maxClients * 0.2)))
                {
                    score += 10;
                }
                else if (clients >= (maxClients - (maxClients * 0.5)))
                {
                    score += 20; // these games need topping off
                }
                else if (clients >= (maxClients - (maxClients * 0.75)))
                {
                    score += 15; // empty games we would love to see filled
                }
            }

            return score;
        }

        public int GetScore()
        {
            // every server starts at -100 points
            int score = -100;

            // and we score them based on fun stuff
            if (Info.ContainsKey("gametype"))
            {
                var gameType = Info["gametype"];

                // FFA is interesting, perhaps.
                if (gameType == "dm")
                {
                    score += 10;
                }

                // domination is cool, but a bit common
                // also, sabotage is cool too, but annoying!
                if (gameType == "dom" || gameType == "sab")
                {
                    score += 10;
                }

                // these gametypes are much better and less 'common'
                if (gameType == "dd" || gameType == "koth" || gameType == "ctf")
                {
                    score += 20;
                }

                if (gameType == "ss" || gameType == "gtnw" || gameType == "arena" || gameType == "gg")
                {
                    score += 25; // these gametypes are custom, therefore deserve pointing out
                }
            }

            if (Info.ContainsKey("hc"))
            {
                if (Info["hc"] == "1")
                {
                    score -= 10; // hardcore servers get reduced, this might be an option?
                }
            }

            // TODO: more purity checks?
            if (Info.ContainsKey("sv_maxclients"))
            {
                int maxClients;

                if (int.TryParse(Info["sv_maxclients"], out maxClients))
                {
                    if (maxClients > 18) // maxclients patch!
                    {
                        score += (3 * (maxClients - 18));
                    }
                    else if (maxClients < 18) // smaller servers may be more fun
                    {
                        score += (1 * (18 - Math.Max(maxClients, 12)));
                    }
                }
            }

            return score;
        }
    }

    public class ServerPage
    {
        private const int PingPercentile = 30; // todo: make this a dvar

        private Socket socket;
        private byte[] obtainedData = new byte[4096];
        private EndPoint obtainedIP = null;

        private bool gotServerList = false;

        private List<IPEndPoint> serversToQuery = new List<IPEndPoint>();
        private Dictionary<IPEndPoint, ServerInfo> servers = new Dictionary<IPEndPoint, ServerInfo>();

        public void Initialize()
        {
            socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
            socket.Blocking = false;
            socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReceiveTimeout, 500);
            socket.Bind(new IPEndPoint(IPAddress.Any, 0));

            obtainedIP = new IPEndPoint(IPAddress.Any, 0);

            socket.BeginReceiveFrom(obtainedData, 0, obtainedData.Length, SocketFlags.None, ref obtainedIP, OnReceive, null);

            new Thread(delegate()
            {
                int lastQueryStep = Environment.TickCount;

                while (true)
                {
                    try
                    {
                        Thread.Sleep(50);

                        int count = 0;

                        for (int i = 0; i < serversToQuery.Count && count < 20; i++)
                        {
                            var serverEP = serversToQuery[i];

                            if (!servers[serverEP].Queried)
                            {
                                QueryServer(serverEP);

                                count++;
                            }
                        }

                        if (count == 0 && serversToQuery.Count > 0)
                        {
                            if (!gotServerList)
                            {
                                new Thread(DelayedCompleteThread).Start();

                                gotServerList = true;
                            }
                        }
                    }
                    catch (Exception ex)
                    {
                        Log.Error(ex);
                    }
                }
            }).Start();

            ThreadPool.QueueUserWorkItem((s) => StartListing());
        }

        private void DelayedCompleteThread()
        {
            try
            {
                // wait for the last servers to respond
                Thread.Sleep(2500);

                // go find the servers with lowest ping
                var responded = from server in servers.Values
                                where server.Info != null
                                select server;

                // get the percentile
                var pings = from server in responded
                            select server.Ping;

                var percentileTreshold = Percentile(pings, PingPercentile / 100.0);

                Log.Info("Ping treshold: {0}", percentileTreshold);

                var lowPingServers = from server in responded
                                     where server.Ping < percentileTreshold
                                     select server;

                Log.Info("found {0} low-ping servers", lowPingServers.Count());

                // filter them out based on our lovely criteria
                var filteredServers = from server in lowPingServers
                                      where (!server.Info.ContainsKey("fs_game") || server.Info["fs_game"] == "") // mhm
                                      select server;

                // and score them
                var scoredServers = from server in filteredServers
                                    orderby server.GetScore() descending
                                    select server;

                var scoredFinal = scoredServers.ToArray();

                // keep the list up-to-date
                while (true)
                {
                    // refresh player counts every 10 seconds
                    Thread.Sleep(10000);

                    foreach (var server in scoredFinal)
                    {
                        server.Queried = false;
                    }

                    Thread.Sleep(2500);

                    var j = new JObject();
                    var ss = new JArray();

                    foreach (var server in scoredFinal)
                    {
                        var s = new JObject();

                        foreach (var info in server.Info)
                        {
                            s[info.Key] = info.Value;
                        }

                        s["score"] = server.GetScore();
                        s["volatileScore"] = server.GetVolatileScore();
                        s["ping"] = server.Ping;
                        s["addr"] = server.EndPoint.ToString();

                        ss.Add(s);
                    }

                    j["servers"] = ss;

                    //Log.Debug("jason is " + j.ToString());

                    GameInterface.ExecuteJS(string.Format("CodeCallback_PageBroadcast(\"{0}\", {1});", "homeServersUpdated", j.ToString()));
                }
            }
            catch (Exception ex)
            {
                Log.Error(ex);
            }
        }

        private void StartListing()
        {
            var packet = Encoding.ASCII.GetBytes("????getservers IW4 61586 full empty");
            packet[0] = 0xFF; packet[1] = 0xFF; packet[2] = 0xFF; packet[3] = 0xFF;

            socket.SendTo(packet, new IPEndPoint(Dns.GetHostEntry("iw4.prod.fourdeltaone.net").AddressList.First(), 20810));
        }

        private void QueryServer(IPEndPoint ep)
        {
            var serverInfo = servers[ep];
            serverInfo.Queried = true;
            serverInfo.Ping = -1;
            serverInfo.QueryTime = Environment.TickCount;

            var packet = Encoding.ASCII.GetBytes("????getinfo xxx");
            packet[0] = 0xFF; packet[1] = 0xFF; packet[2] = 0xFF; packet[3] = 0xFF;

            socket.SendTo(packet, ep);
        }

        private void OnReceive(IAsyncResult result)
        {
            try
            {
                int bytes = socket.EndReceiveFrom(result, ref obtainedIP);

                //Log.Debug("Received {0} bytes from {1}...", bytes, obtainedIP);

                string packetData = Encoding.Default.GetString(obtainedData, 0, bytes);
                packetData = packetData.Substring(4);

                string[] lines = packetData.Split('\n');
                string message = lines[0];

                string messageType = message.Split(' ', '\\')[0].ToLower();

                //Log.Debug("message type: {0}", messageType);

                if (messageType == "inforesponse")
                {
                    var info = GetParams(lines[1].Split('\\'));
                    ServerInfo thisInfo;

                    if (servers.TryGetValue((IPEndPoint)obtainedIP, out thisInfo))
                    {
                        thisInfo.Info = info;
                        thisInfo.Ping = Environment.TickCount - thisInfo.QueryTime;

                        
                    }
                }
                else if (messageType == "getserversresponse")
                {
                    var buffptr = 0;

                    while ((buffptr + 1) < obtainedData.Length)
                    {
                        do 
                        {
                            if (obtainedData[buffptr++] == (byte)'\\')
                            {
                                break;
                            }
                        } while (buffptr < obtainedData.Length);

                        if (buffptr >= (obtainedData.Length - 6))
                        {
                            break;
                        }

                        var ip = new byte[4];
                        ip[0] = obtainedData[buffptr++];
                        ip[1] = obtainedData[buffptr++];
                        ip[2] = obtainedData[buffptr++];
                        ip[3] = obtainedData[buffptr++];

                        ushort port = (ushort)((obtainedData[buffptr++]) << 8);
                        port += (ushort)((obtainedData[buffptr++]) & 0xFF);

                        IPEndPoint ep = new IPEndPoint(new IPAddress(ip), port);

                        //Log.Debug("got server {0}", ep);

                        if (obtainedData[buffptr] != (byte)'\\')
                        {
                            break;
                        }

                        servers.Add(ep, new ServerInfo() { EndPoint = ep });
                        serversToQuery.Add(ep);

                        if (obtainedData[buffptr + 1] == (byte)'E' && obtainedData[buffptr + 2] == (byte)'O' && obtainedData[buffptr + 3] == (byte)'T')
                        {
                            break;
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                Log.Error(ex);
            }

            socket.BeginReceiveFrom(obtainedData, 0, obtainedData.Length, SocketFlags.None, ref obtainedIP, OnReceive, null);
        }

        private Dictionary<string, string> GetParams(string[] parts)
        {
            string key, val;
            var paras = new Dictionary<string, string>();

            for (int i = 0; i < parts.Length; i++)
            {
                if (parts[i].Length == 0)
                    continue;

                key = parts[i++];
                val = parts[i];
                paras[key] = val;
            }

            return paras;
        }

        public static double Percentile(IEnumerable<int> seq, double percentile)
        {
            var elements = seq.ToArray();
            Array.Sort(elements);
            double realIndex = percentile * (elements.Length - 1);
            int index = (int)realIndex;
            double frac = realIndex - index;
            if (index + 1 < elements.Length)
                return elements[index] * (1 - frac) + elements[index + 1] * frac;
            else
                return elements[index];
        }
    }
}
