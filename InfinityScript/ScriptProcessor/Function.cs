using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace InfinityScript
{
    public static class Function
    {
        private static Dictionary<string, int> _functionMappings = new Dictionary<string, int>();
        private static Dictionary<string, int> _globalFunctionMappings = new Dictionary<string, int>();

        public static void AddMapping(string name, int value)
        {
            _functionMappings[name.ToLowerInvariant()] = value;
        }

        public static void AddGlobalMapping(string name, int value)
        {
            _globalFunctionMappings[name.ToLowerInvariant()] = value;
        }

        public static bool IsFunction(string name)
        {
            return (_functionMappings.ContainsKey(name.ToLowerInvariant()) || _globalFunctionMappings.ContainsKey(name.ToLowerInvariant()));
        }

        private static int _entRef;

        private static object _returnValue;

        public static void SetEntRef(int entRef)
        {
            _entRef = entRef;
        }

        /*public static void Call(string func, params Parameter[] parameters)
        {
            func = func.ToLowerInvariant();

            var table = _globalFunctionMappings;

            if (_entRef != -1)
            {
                table = _functionMappings;
            }

            if (!table.ContainsKey(func))
            {
                Log.Write(LogLevel.Warning, "no such function: {0}", func);
                return;
            }

            Call(table[func], parameters);
        }*/

        public static void Call(string identifier, params Parameter[] parameters)
        {
            CallRaw(identifier, parameters);
        }

        /*public static TReturn Call<TReturn>(string func, params Parameter[] parameters)
        {
            func = func.ToLowerInvariant();

            var table = _globalFunctionMappings;

            if (_entRef != -1)
            {
                table = _functionMappings;
            }

            if (!table.ContainsKey(func))
            {
                Log.Write(LogLevel.Warning, "no such function: {0}", func);
                return default(TReturn);
            }

            return Call<TReturn>(table[func], parameters);
        }
        */

        public static TReturn Call<TReturn>(string identifier, params Parameter[] parameters)
        {
            object retval = Call(identifier, typeof(TReturn), parameters);

            return (TReturn)retval;
        }

        internal static object Call(string identifier, Type returnType, params Parameter[] parameters)
        {
            CallRaw(identifier, parameters);

            return _returnValue;
        }

        private static void CallRaw(string identifier, params Parameter[] parameters)
        {
            // push arguments
            foreach (var parameter in parameters.Reverse())
            {
                parameter.PushValue();
            }

            // call the function
            if (!GameInterface.Script_Call(identifier.ToLowerInvariant(), _entRef, parameters.Length))
            {
                SetEntRef(-1);
                throw new ScriptException("Could not find function " + identifier);
            }

            // reset the entref to 0
            SetEntRef(-1);

            // check for return values
            _returnValue = null;

            if (GameInterface.Notify_NumArgs() == 1)
            {
                switch (GameInterface.Script_GetType(0))
                {
                    case VariableType.Entity:
                        _returnValue = Entity.GetEntity(GameInterface.Script_GetEntRef(0));
                        break;
                    case VariableType.Integer:
                        _returnValue = GameInterface.Script_GetInt(0);
                        break;
                    case VariableType.Float:
                        _returnValue = GameInterface.Script_GetFloat(0);
                        break;
                    case VariableType.String:
                        _returnValue = GameInterface.Script_GetString(0);
                        break;
                    case VariableType.Vector:
                        Vector3 outValue;
                        GameInterface.Script_GetVector(0, out outValue);
                        _returnValue = outValue;
                        break;
                }
            }

            GameInterface.Script_CleanReturnStack();
        }
    }

    public class ScriptException : Exception
    {
        public ScriptException()
            : base()
        {

        }

        public ScriptException(string a)
            : base(a)
        {

        }
    }
}
