using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Net;

namespace InfinityScript
{
    public class Entity : Notifiable
    {
        #region static field stuff
        private static Dictionary<string, int> _fieldNames = new Dictionary<string, int>();

        private static void AddFieldMapping(string name, int id)
        {
            _fieldNames[name.ToLowerInvariant()] = id;
        }

        internal static void InitializeMappings()
        {
            AddFieldMapping("code_classname", 0);
            AddFieldMapping("classname", 1);
            AddFieldMapping("origin", 2);
            AddFieldMapping("model", 3);
            AddFieldMapping("spawnflags", 4);
            AddFieldMapping("target", 5);
            AddFieldMapping("targetname", 6);
            AddFieldMapping("count", 7);
            AddFieldMapping("health", 8);
            AddFieldMapping("dmg", 9);
            AddFieldMapping("angles", 10);
            AddFieldMapping("birthtime", 11);
            AddFieldMapping("script_linkname", 12);
            AddFieldMapping("slidevelocity", 13);
            AddFieldMapping("name", 24576);
            AddFieldMapping("sessionteam", 24577);
            AddFieldMapping("sessionstate", 24578);
            AddFieldMapping("maxhealth", 24579);
            AddFieldMapping("score", 24580);
            AddFieldMapping("deaths", 24581);
            AddFieldMapping("statusicon", 24582);
            AddFieldMapping("headicon", 24583);
            AddFieldMapping("headiconteam", 24584);
            AddFieldMapping("kills", 24585);
            AddFieldMapping("assists", 24586);
            AddFieldMapping("hasradar", 24587);
            AddFieldMapping("isradarblocked", 24588);
            AddFieldMapping("radarstrength", 24589);
            AddFieldMapping("radarshowenemydirection", 24590);
            AddFieldMapping("radarmode", 24591);
            AddFieldMapping("forcespectatorclient", 24592);
            AddFieldMapping("killcamentity", 24593);
            AddFieldMapping("killcamentitylookat", 24594);
            AddFieldMapping("archivetime", 24595);
            AddFieldMapping("psoffsettime", 24596);
            AddFieldMapping("pers", 24597);
            AddFieldMapping("veh_speed", 32768);
            AddFieldMapping("veh_pathspeed", 32769);
            AddFieldMapping("veh_transmission", 32770);
            AddFieldMapping("veh_pathdir", 32771);
            AddFieldMapping("veh_pathtype", 32772);
            AddFieldMapping("veh_topspeed", 32773);
            AddFieldMapping("veh_brake", 32774);
            AddFieldMapping("veh_throttle", 32775);
            AddFieldMapping("x", 0);
            AddFieldMapping("y", 1);
            AddFieldMapping("z", 2);
            AddFieldMapping("fontscale", 3);
            AddFieldMapping("font", 4);
            AddFieldMapping("alignx", 5);
            AddFieldMapping("aligny", 6);
            AddFieldMapping("horzalign", 7);
            AddFieldMapping("vertalign", 8);
            AddFieldMapping("color", 9);
            AddFieldMapping("alpha", 10);
            AddFieldMapping("label", 11);
            AddFieldMapping("sort", 12);
            AddFieldMapping("foreground", 13);
            AddFieldMapping("lowresbackground", 14);
            AddFieldMapping("hidewhendead", 15);
            AddFieldMapping("hidewheninmenu", 16);
            AddFieldMapping("glowcolor", 17);
            AddFieldMapping("glowalpha", 18);
            AddFieldMapping("archived", 19);
            AddFieldMapping("hidein3rdperson", 20);
            //AddFieldMapping("targetname", 0);
            //AddFieldMapping("target", 1);
            AddFieldMapping("script_linkname", 2);
            AddFieldMapping("script_noteworthy", 3);
            //AddFieldMapping("origin", 4);
            //AddFieldMapping("angles", 5);
            AddFieldMapping("speed", 6);
            AddFieldMapping("lookahead", 7);
        }
        #endregion

        private int _entRef;

        internal Entity(int entRef)
        {
            _entRef = entRef;

            OnNotify("spawned_player", thisEntity =>
            {
                if (thisEntity.SpawnedPlayer != null)
                {
                    thisEntity.SpawnedPlayer();
                }
            });
        }

        public int EntRef
        {
            get
            {
                return _entRef;
            }
        }

        #region onnotify
        public void OnNotify(string type, Action<Entity> handler)
        {
            OnNotifyInternal(type, handler);
        }

        public void OnNotify(string type, Action<Entity, Parameter> handler)
        {
            OnNotifyInternal(type, handler);
        }

        public void OnNotify(string type, Action<Entity, Parameter, Parameter> handler)
        {
            OnNotifyInternal(type, handler);
        }

        public void OnNotify(string type, Action<Entity, Parameter, Parameter, Parameter> handler)
        {
            OnNotifyInternal(type, handler);
        }

        public void OnNotify(string type, Action<Entity, Parameter, Parameter, Parameter, Parameter> handler)
        {
            OnNotifyInternal(type, handler);
        }

        public void OnNotify(string type, Action<Entity, Parameter, Parameter, Parameter, Parameter, Parameter> handler)
        {
            OnNotifyInternal(type, handler);
        }

        public void OnNotify(string type, Action<Entity, Parameter, Parameter, Parameter, Parameter, Parameter, Parameter> handler)
        {
            OnNotifyInternal(type, handler);
        }

        public void OnNotify(string type, Action<Entity, Parameter, Parameter, Parameter, Parameter, Parameter, Parameter, Parameter> handler)
        {
            OnNotifyInternal(type, handler);
        }

        public void OnNotify(string type, Action<Entity, Parameter, Parameter, Parameter, Parameter, Parameter, Parameter, Parameter, Parameter> handler)
        {
            OnNotifyInternal(type, handler);
        }

        public void OnNotify(string type, Action<Entity, Parameter, Parameter, Parameter, Parameter, Parameter, Parameter, Parameter, Parameter, Parameter> handler)
        {
            OnNotifyInternal(type, handler);
        }

        public void OnNotify(string type, Action<Entity, Parameter, Parameter, Parameter, Parameter, Parameter, Parameter, Parameter, Parameter, Parameter, Parameter> handler)
        {
            OnNotifyInternal(type, handler);
        }
        #endregion

        #region events
        public event Action SpawnedPlayer;
        #endregion

        #region predefined calls
        public bool HasWeapon(string name)
        {
            return (Call<int>("hasweapon", name) != 0);
        }

        public void TakeAllWeapons()
        {
            Call("takeallweapons");
        }

        public void GiveWeapon(string name)
        {
            Call("giveweapon", name);
        }

        public void TakeWeapon(string name)
        {
            Call("takeweapon", name);
        }

        public void SwitchToWeapon(string name)
        {
            Call("switchtoweapon", name);
        }

        public void SwitchToWeaponImmediate(string name)
        {
            Call("switchtoweaponimmediate", name);
        }

        public void SetPerk(string name, bool codePerk, bool useSlot)
        {
            Call("setperk", name, codePerk, useSlot);
        }

        public void SetClientDvar(string name, string value)
        {
            Call("setclientdvar", name, value);
        }

        public int GetWeaponAmmoClip(string weaponName)
        {
            return Call<int>("getWeaponAmmoClip", weaponName);
        }

        public int GetWeaponAmmoStock(string weaponName)
        {
            return Call<int>("getWeaponAmmoStock", weaponName);
        }
        #endregion

        #region calls
        public void Call(string func, params Parameter[] parameters)
        {
            Function.SetEntRef(_entRef);
            Function.Call(func, parameters);
        }

        /*public void Call(int identifier, params Parameter[] parameters)
        {
            Function.SetEntRef(_entRef);
            Function.Call(identifier, parameters);
        }*/

        public TReturn Call<TReturn>(string func, params Parameter[] parameters)
        {
            Function.SetEntRef(_entRef);
            return Function.Call<TReturn>(func, parameters);
        }

        /*public TReturn Call<TReturn>(int identifier, params Parameter[] parameters)
        {
            Function.SetEntRef(_entRef);
            return Function.Call<TReturn>(identifier, parameters);
        }*/
        #endregion

        #region call wrappers
        public bool IsAlive
        {
            get
            {
                Function.SetEntRef(-1);
                return (Function.Call<int>("isalive", this) != 0); 
            }
        }

        public bool IsPlayer
        {
            get
            {
                Function.SetEntRef(-1);
                return (Function.Call<int>("isplayer", this) != 0);
            }
        }

        public string CurrentWeapon
        {
            get
            {
                return Call<string>("getcurrentweapon");
            }
        }

        public long GUID
        {
            get
            {
                return long.Parse(Call<string>("getguid"), System.Globalization.NumberStyles.HexNumber);
            }
        }

        #endregion

        #region field wrappers
        public Vector3 Origin
        {
            get
            {
                return GetField<Vector3>("origin");
            }
            set
            {
                SetField("origin", value);
            }
        }

        public int Health
        {
            get
            {
                return GetField<int>("health");
            }
            set
            {
                SetField("health", value);
            }
        }

        public string Name
        {
            get
            {
                return GetField<string>("name");
            }
            set
            {
                SetField("name", value);
            }
        }
        #endregion

        #region fields
        private Dictionary<string, object> _privateFields = new Dictionary<string, object>();

        public bool HasField(string name)
        {
            name = name.ToLowerInvariant();

            return (_fieldNames.ContainsKey(name) || _privateFields.ContainsKey(name));
        }

        public T GetField<T>(string name)
        {
            name = name.ToLowerInvariant();

            if (!_fieldNames.ContainsKey(name))
            {
                return (T)_privateFields[name];
            }

            object returnValue = null;
            int fieldID = _fieldNames[name];

            GameInterface.Script_GetField(_entRef, fieldID);

            switch (GameInterface.Script_GetType(0))
            {
                case VariableType.Integer:
                    returnValue = GameInterface.Script_GetInt(0);
                    break;
                case VariableType.Float:
                    returnValue = GameInterface.Script_GetFloat(0);
                    break;
                case VariableType.String:
                    returnValue = GameInterface.Script_GetString(0);
                    break;
                case VariableType.Vector:
                    Vector3 outValue;
                    GameInterface.Script_GetVector(0, out outValue);
                    returnValue = outValue;
                    break;
            }

            GameInterface.Script_CleanReturnStack();

            return (T)Convert.ChangeType(returnValue, typeof(T));
        }

        public void SetField(string name, Parameter value)
        {
            name = name.ToLowerInvariant();

            if (!_fieldNames.ContainsKey(name))
            {
                _privateFields[name] = value.InternalValue;
                return;
            }

            int fieldID = _fieldNames[name];

            value.PushValue();
            GameInterface.Script_SetField(_entRef, fieldID);
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
            GameInterface.Script_NotifyNum(_entRef, type, parameters.Length);
        }
        #endregion

        #region pool stuff
        private static Dictionary<int, Entity> _entities = new Dictionary<int, Entity>();

        public static Entity GetEntity(int entRef)
        {
            if (_entities.ContainsKey(entRef))
            {
                return _entities[entRef];
            }

            var entity = new Entity(entRef);
            _entities[entRef] = entity;
            if (entRef < 18)
            {
                entity.OnNotify("disconnect", ent =>
                {
                    _entities.Remove(ent.EntRef);
                });
            }

            return entity;
        }

        internal static void RunAll(Action<Entity> cb)
        {
            var entities = _entities.Values.ToArray();

            foreach (var entity in entities)
            {
                try
                {
                    cb(entity);
                }
                catch (Exception ex)
                {
                    Log.Write(LogLevel.Error, "Exception during RunAll call: {0}", ex.ToString());
                }
            }
        }
        #endregion

        #region ontimer
        public void OnInterval(int interval, Func<Entity, bool> function)
        {
            _timers.Add(new ScriptTimer()
            {
                func = function,
                triggerTime = 0,
                interval = interval
            });
        }

        public void AfterDelay(int delay, Action<Entity> function)
        {
            _timers.Add(new ScriptTimer()
            {
                func = function,
                triggerTime = (_currentTime + delay),
                interval = -1
            });
        }
        #endregion

        public int UserID
        {
            get
            {
                return (int)(GUID & 0xFFFFFFFF);
            }
        }

        public int Ping
        {
            get
            {
                return GameInterface.GetPing(_entRef);
            }
        }

        public IPEndPoint IP
        {
            get
            {
                var l = GameInterface.GetClientAddress(_entRef);
                var ip = (l >> 32);
                var port = (int)(l & 0xFFFFFFFF);
                return new IPEndPoint(new IPAddress(ip), port);
            }
        }

        public override string ToString()
        {
            return string.Format("[Entity:{0}:{1}]", _entRef >> 16, _entRef & 0xFFFF);
        }

        public override int GetHashCode()
        {
            return _entRef;
        }

        public override bool Equals(object obj)
        {
            var other = obj as Entity;

            return other != null && other.EntRef == EntRef;
        }
    }
}
