using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace InfinityScript
{
    public abstract class BaseScript : Notifiable
    {
        #region events
        protected event Action Tick;
        protected event Action<Entity> PlayerConnecting;
        protected event Action<Entity> PlayerConnected;
        protected event Action<Entity> PlayerDisconnected;
        #endregion

        #region player list
        public List<Entity> Players { get; private set; }
        #endregion

        public BaseScript()
        {
            Players = new List<Entity>();

            OnNotify("connecting", entity =>
            {
                Players.Add(entity.As<Entity>());

                if (PlayerConnecting != null)
                {
                    PlayerConnecting(entity.As<Entity>());
                }
            });

            OnNotify("connected", entity =>
            {
                if (PlayerConnected != null)
                {
                    PlayerConnected(entity.As<Entity>());
                }
            });

            _ = new ScriptDynamics(-1);
        }

        public dynamic _ { get; private set; }

        #region virtual call functions
        public virtual void OnStartGameType() { }
        public virtual void OnPlayerDisconnect(Entity player)
        {
            Players.Remove(player);

            if (PlayerDisconnected != null)
            {
                PlayerDisconnected(player);
            }
        }

        public virtual void OnPlayerDamage(Entity player, Entity inflictor, Entity attacker, int damage, int dFlags, string mod, string weapon, Vector3 point, Vector3 dir, string hitLoc) { }
        public virtual void OnPlayerKilled(Entity player, Entity inflictor, Entity attacker, int damage, string mod, string weapon, Vector3 dir, string hitLoc) { }

        public virtual void OnSay(Entity player, string name, string message) { }
        public virtual EventEat OnSay2(Entity player, string name, string message) { return EventEat.EatNone; }
        public virtual EventEat OnSay3(Entity player, ChatType type, string name, ref string message) { return EventEat.EatNone; }
        public enum EventEat { EatNone = 0, EatScript = 1, EatGame = 2 };
        public enum ChatType { All = 0, Team = 1 };

        public virtual void OnExitLevel() { }
        #endregion

        #region onnotify
        public void OnNotify(string type, Action handler)
        {
            OnNotifyInternal(type, handler);
        }

        public void OnNotify(string type, Action<Parameter> handler)
        {
            OnNotifyInternal(type, handler);
        }

        public void OnNotify(string type, Action<Parameter, Parameter> handler)
        {
            OnNotifyInternal(type, handler);
        }

        public void OnNotify(string type, Action<Parameter, Parameter, Parameter> handler)
        {
            OnNotifyInternal(type, handler);
        }

        public void OnNotify(string type, Action<Parameter, Parameter, Parameter, Parameter> handler)
        {
            OnNotifyInternal(type, handler);
        }

        public void OnNotify(string type, Action<Parameter, Parameter, Parameter, Parameter, Parameter> handler)
        {
            OnNotifyInternal(type, handler);
        }

        public void OnNotify(string type, Action<Parameter, Parameter, Parameter, Parameter, Parameter, Parameter> handler)
        {
            OnNotifyInternal(type, handler);
        }

        public void OnNotify(string type, Action<Parameter, Parameter, Parameter, Parameter, Parameter, Parameter, Parameter> handler)
        {
            OnNotifyInternal(type, handler);
        }

        public void OnNotify(string type, Action<Parameter, Parameter, Parameter, Parameter, Parameter, Parameter, Parameter, Parameter> handler)
        {
            OnNotifyInternal(type, handler);
        }

        public void OnNotify(string type, Action<Parameter, Parameter, Parameter, Parameter, Parameter, Parameter, Parameter, Parameter, Parameter> handler)
        {
            OnNotifyInternal(type, handler);
        }

        public void OnNotify(string type, Action<Parameter, Parameter, Parameter, Parameter, Parameter, Parameter, Parameter, Parameter, Parameter, Parameter> handler)
        {
            OnNotifyInternal(type, handler);
        }
        #endregion

        #region timer adders
        public void OnInterval(int interval, Func<bool> function)
        {
            _timers.Add(new ScriptTimer()
            {
                func = function,
                triggerTime = 0,
                interval = interval
            });
        }

        public void AfterDelay(int delay, Action function)
        {
            _timers.Add(new ScriptTimer()
            {
                func = function,
                triggerTime = (_currentTime + delay),
                interval = -1
            });
        }
        #endregion

        #region runframe
        internal void RunFrame()
        {
            // handle tick
            if (Tick != null)
            {
                try
                {
                    Tick();
                }
                catch (Exception ex)
                {
                    Log.Write(LogLevel.Error, "Exception during Tick on script {0}: {1}", GetType().Name, ex.ToString());
                }
            }

            ProcessTimers();
            ProcessNotifications();
        }
        #endregion

        #region notify
        public void Notify(string type, params Parameter[] parameters)
        {
            // push arguments
            foreach (var parameter in parameters.Reverse())
            {
                parameter.PushValue();
            }

            // call game function
            GameInterface.Script_NotifyLevel(type, parameters.Length);
        }
        #endregion

        #region calls
        public void Call(string func, params Parameter[] parameters)
        {
            Function.SetEntRef(-1);
            Function.Call(func, parameters);
        }

        /*public void Call(int identifier, params Parameter[] parameters)
        {
            Function.SetEntRef(-1);
            Function.Call(identifier, parameters);
        }*/

        public TReturn Call<TReturn>(string func, params Parameter[] parameters)
        {
            Function.SetEntRef(-1);
            return Function.Call<TReturn>(func, parameters);
        }

        /*public TReturn Call<TReturn>(int identifier, params Parameter[] parameters)
        {
            Function.SetEntRef(-1);
            return Function.Call<TReturn>(identifier, parameters);
        }*/
        #endregion

        #region commands
        internal Dictionary<string, List<Func<string[], bool>>> _serverCommandHandlers = new Dictionary<string, List<Func<string[], bool>>>();
        internal Dictionary<string, List<Action<Entity, string[]>>> _clientCommandHandlers = new Dictionary<string, List<Action<Entity, string[]>>>();

        internal bool ProcessServerCommand(string command, string[] args)
        {
            bool eat = false;
            if (_serverCommandHandlers.ContainsKey(command))
            {
                var handles = _serverCommandHandlers[command];
                foreach (var handle in handles)
                {
                    if (handle(args))
                    {
                        eat = true;
                    }
                }
            }
            return eat;
        }
        internal bool ProcessClientCommand(string command, Entity entity, string[] args)
        {
            bool eat = false;
            if (_clientCommandHandlers.ContainsKey(command))
            {
                var handles = _clientCommandHandlers[command];
                foreach (var handle in handles)
                {
                    handle(entity, args);
                    eat = true;
                }
            }
            return eat;
        }

        public void OnServerCommand(string command, Func<string[], bool> func)
        {
            if (!_serverCommandHandlers.ContainsKey(command))
            {
                _serverCommandHandlers[command] = new List<Func<string[], bool>>();
            }
            _serverCommandHandlers[command].Add(func);
        }
        public void OnServerCommand(string command, Action<string[]> func)
        {
            OnServerCommand(command, args =>
            {
                func(args);
                return true;
            });
        }
        public void OnServerCommand(string command, Action func)
        {
            OnServerCommand(command, args =>
            {
                func();
                return true;
            });
        }

        public void OnClientCommand(string command, Action<Entity, string[]> func)
        {
            if (!_clientCommandHandlers.ContainsKey(command))
            {
                _clientCommandHandlers[command] = new List<Action<Entity, string[]>>();
            }
            _clientCommandHandlers[command].Add(func);
        }
        public void OnClientCommand(string command, Action<Entity> func)
        {
            OnClientCommand(command, (entity, args) => func(entity));
        }
        public void OnClientCommand(string command, Action func)
        {
            OnClientCommand(command, (entity, args) => func());
        }
        #endregion
    }
}
