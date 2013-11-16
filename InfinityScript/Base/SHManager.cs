using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Net;

namespace InfinityScript
{
    internal static class SHManager
    {
        public static void InitializeOnce()
        {
            // initialize logging
            Log.Initialize(LogLevel.All);
            Log.AddListener(new FileLogListener("InfinityScript.log", false));
            Log.AddListener(new TraceLogListener());
            Log.AddListener(new GameLogListener());

            try
            {
                Entity.InitializeMappings();
                ScriptNames.Initialize();
                ScriptLoader.Initialize();
                WebManager.Initialize();
            }
            catch (Exception ex)
            {
                if (ex.InnerException != null)
                {
                    Log.Write(LogLevel.Critical, ex.InnerException.ToString());
                }

                Log.Write(LogLevel.Critical, ex.ToString());
                Environment.Exit(0);
            }
        }

        public static void InitializeLevel()
        {
            // load scripts
            try
            {
                ScriptProcessor.ClearScripts();
                ScriptLoader.InitializeLevel();
            }
            catch (Exception ex)
            {
                Log.Write(LogLevel.Critical, ex.ToString());
                Environment.Exit(0);
            }

            //GameInterface.TempFunc();
            //Environment.Exit(0);
        }

        public static void RunFrame()
        {
            try
            {
                Entity.RunAll(entity => entity.ProcessNotifications());
                Entity.RunAll(entity => entity.ProcessTimers());
                ScriptProcessor.RunAll(script => script.RunFrame());
            }
            catch (Exception ex)
            {
                Log.Write(LogLevel.Critical, ex.ToString());
                Environment.Exit(0);
            }
            //GameInterface.Script_PushString("Hello!");
            //GameInterface.Script_PushInt(1337);
            //GameInterface.Script_Call(362, 0, 1);
        }

        public static void Shutdown()
        {
            ScriptProcessor.RunAll(script => script.OnExitLevel());
        }

        public static void LoadScript(string scriptName)
        {
            ScriptLoader.LoadAssemblies("scripts", scriptName);
        }

        public static bool HandleSay(int clientNum, string clientName, ref string message, int team)
        {
            var entity = Entity.GetEntity(clientNum);
            var eatgame = false;
            var eatscript = false;
            var messageTemp = message;
            ScriptProcessor.RunAll(script =>
            {
                
                // Run Script.OnSay3 (by reference, with team and eat)
                if (eatscript) return;

                var handled = script.OnSay3(entity, team == 0 ? BaseScript.ChatType.All : BaseScript.ChatType.Team, clientName, ref messageTemp);

                eatgame = eatgame || handled.HasFlag(BaseScript.EventEat.EatGame);
                eatscript = handled.HasFlag(BaseScript.EventEat.EatScript);

                // Run Script.OnSay2 (normal, with eat)
                if (eatscript) return;

                handled = script.OnSay2(entity, clientName, messageTemp);
                eatgame = eatgame || handled.HasFlag(BaseScript.EventEat.EatGame);
                eatscript = handled.HasFlag(BaseScript.EventEat.EatScript);
                
                // Run Script.OnSay (normal, without eat)
                if (eatscript) return;
                script.OnSay(entity, clientName, messageTemp);
            });

            message = messageTemp;

            return eatgame;
        }

        public static void HandleCall(int entityRef, CallType funcID)
        {
            var entity = Entity.GetEntity(entityRef);
            int numArgs = GameInterface.Notify_NumArgs();
            var paras = CollectParameters(numArgs);

            switch (funcID)
            {
                case CallType.StartGameType:
                    ScriptProcessor.RunAll(script => script.OnStartGameType());
                    break;
                case CallType.PlayerConnect:
                    //ScriptProcessor.RunAll(script => script.OnPlayerConnect(entity));
                    break;
                case CallType.PlayerDisconnect:
                    ScriptProcessor.RunAll(script => script.OnPlayerDisconnect(entity));
                    break;
                case CallType.PlayerDamage:
                    if (paras[6].IsNull)
                    {
                        paras[6] = new Vector3(0, 0, 0);
                    }

                    if (paras[7].IsNull)
                    {
                        paras[7] = new Vector3(0, 0, 0);
                    }

                    ScriptProcessor.RunAll(script => script.OnPlayerDamage(entity, paras[0].As<Entity>(), paras[1].As<Entity>(), paras[2].As<int>(), paras[3].As<int>(), paras[4].As<string>(), paras[5].As<string>(), paras[6].As<Vector3>(), paras[7].As<Vector3>(), paras[8].As<string>()));
                    break;
                case CallType.PlayerKilled:
                    if (paras[5].IsNull)
                    {
                        paras[5] = new Vector3(0, 0, 0);
                    }

                    ScriptProcessor.RunAll(script => script.OnPlayerKilled(entity, paras[0].As<Entity>(), paras[1].As<Entity>(), paras[2].As<int>(), paras[3].As<string>(), paras[4].As<string>(), paras[5].As<Vector3>(), paras[6].As<string>()));
                    break;
            }
        }

        public static void HandleNotify(int entity)
        {
            string type = GameInterface.Notify_Type();
            int numArgs = GameInterface.Notify_NumArgs();

            var paras = CollectParameters(numArgs);

            if (type != "touch")
            {
                // dispatch the thingy
                // IW4M CHANGE
                //if (GameInterface.Script_GetObjectType(entity) == 21) // actual entity
                if (GameInterface.Script_GetObjectType(entity) == 22) // actual entity
                {
                    var entRef = GameInterface.Script_ToEntRef(entity);
                    var entObj = Entity.GetEntity(entRef);

                    entObj.HandleNotify(entity, type, paras);
                }
                //else if(GameInterface.Script_GetObjectType(entity) == 24) // not an actual entity
                else if (GameInterface.Script_GetObjectType(entity) == 21) // not an actual entity
                {
                    var entRef = GameInterface.Script_GetTempEntRef();
                    var entObj = Entity.GetEntity(entRef);

                    entObj.HandleNotify(entity, type, paras);
                }

                ScriptProcessor.RunAll(script => script.HandleNotify(entity, type, paras));
            }
        }

        public static bool HandleServerCommand(string commandName)
        {
            var args = new string[GameInterface.Cmd_Argc()];

            for (var i = 0; i < args.Length; i++)
            {
                args[i] = GameInterface.Cmd_Argv(i);
            }

            var eat = false;
            ScriptProcessor.RunAll(script =>
            {
                var success = script.ProcessServerCommand(commandName.ToLowerInvariant(), args);
                if (success)
                {
                    eat = true;
                }
            });
            return eat;
        }

        public static bool HandleClientCommand(string commandName, int entity)
        {
            var args = new string[GameInterface.Cmd_Argc_sv()];

            for (var i = 0; i < args.Length; i++)
            {
                args[i] = GameInterface.Cmd_Argv_sv(i);
            }

            var entObj = Entity.GetEntity(entity);
            var handled = false;

            ScriptProcessor.RunAll(script =>
            {
                var success = script.ProcessClientCommand(commandName.ToLowerInvariant(), entObj, args);
                if (success)
                {
                    handled = true;
                }
            });

            return handled;
        }

        public static byte[] HandleWebRequest(string method, string uri, string query, int numHeaders, int remoteIP)
        {
            var headers = new Dictionary<string, string>();

            for (int i = 0; i < numHeaders; i++)
            {
                var header = GameInterface.GetHTTPHeader(i);
                var headerParts = header.Split(new[] { ": " }, 2, StringSplitOptions.None);

                if (headerParts.Length < 2)
                {
                    continue;
                }

                headers[headerParts[0]] = headerParts[1];
            }

            try
            {
                return WebManager.HandleRequest(method, uri.Split('?')[0], query, headers, new IPAddress(BitConverter.GetBytes(IPAddress.NetworkToHostOrder(remoteIP))));
            }
            catch (Exception ex)
            {
                if (ex.InnerException != null)
                {
                    Log.Error(ex.InnerException);
                }

                Log.Error(ex);
                return Encoding.UTF8.GetBytes("HTTP/1.0 500 Internal Server Error\r\n\r\n" + ex.Message);
            }

            /*var requestData = new Dictionary<string, object>();
            requestData["owin.RequestBody"] = Stream.Null;
            requestData["owin.RequestHeaders"] = headers;
            requestData["owin.RequestMethod"] = method;
            requestData["owin.RequestPath"] = uri.Substring(1).Split('?')[0];
            requestData["owin.RequestPathBase"] = "/";
            requestData["owin.RequestProtocol"] = "HTTP/1.0";
            requestData["owin.RequestQueryString"] = query;
            requestData["owin.RequestScheme"] = "http";

            requestData["owin.Version"] = "1.0";
            requestData["owin.CallCancelled"] = new System.Threading.CancellationToken();

            WebManager.HandleRequest(requestData);

            try
            {
                foreach (var data in requestData)
                {
                    Log.Debug(data.Key + ": " + data.Value.ToString());
                }

                var responseStream = requestData["owin.ResponseBody"] as Stream;
                var responseHeaders = requestData["owin.ResponseHeaders"] as IDictionary<string, string[]>;
                var responseStatusCode = requestData.ContainsKey("owin.ResponseStatusCode") ? (int)requestData["owin.ResponseStatusCode"] : 200;

                var response = new StringBuilder();
                response.Append("HTTP/1.0 ");
                response.Append(responseStatusCode.ToString());
                response.Append(" ");
                response.Append(_statusCodes[responseStatusCode]);
                response.Append("\r\n");

                foreach (var header in responseHeaders)
                {
                    response.Append(header.Key);
                    response.Append(": ");
                    response.Append(string.Join(", ", header.Value));
                    response.Append("\r\n");
                }

                response.Append("\r\n");

                var headerBytes = Encoding.UTF8.GetBytes(response.ToString());
                var dataBytes = new byte[responseStream.Length + headerBytes.Length];
                Array.Copy(headerBytes, dataBytes, headerBytes.Length);

                responseStream.Read(dataBytes, headerBytes.Length, (int)responseStream.Length);

                return dataBytes;
            }
            catch (Exception ex)
            {
                Log.Error(ex);
                return Encoding.UTF8.GetBytes("HTTP/1.0 500 Internal Server Error\r\n\r\n" + ex.Message);
            }*/
        }

        private static Parameter[] CollectParameters(int numArgs)
        {
            var paras = new Parameter[numArgs];

            for (int i = 0; i < numArgs; i++)
            {
                var ptype = GameInterface.Script_GetType(i);
                object value = null;

                switch (ptype)
                {
                    case VariableType.Integer:
                        value = GameInterface.Script_GetInt(i);
                        break;
                    case VariableType.String:
                        value = GameInterface.Script_GetString(i);
                        break;
                    case VariableType.Float:
                        value = GameInterface.Script_GetFloat(i);
                        break;
                    case VariableType.Entity:
                        value = Entity.GetEntity(GameInterface.Script_GetEntRef(i));
                        break;
                    case VariableType.Vector:
                        Vector3 v;
                        GameInterface.Script_GetVector(i, out v);
                        value = v;
                        break;
                    default:
                        break;
                }

                paras[i] = new Parameter(ptype, value);
            }

            return paras;
        }
    }

    internal enum CallType
    {
        StartGameType = 0,
        PlayerConnect = 1,
        PlayerDisconnect = 2,
        PlayerDamage = 3,
        PlayerKilled = 4,
        VehicleDamage = 5
    }
}
