using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace InfinityScript
{
    public class Parameter
    {
        private VariableType _type;
        private object _value;

        internal object InternalValue
        {
            get
            {
                return _value;
            }
        }

        public VariableType Type
        {
            get
            {
                return _type;
            }
        }

        internal Parameter(VariableType type, object value)
        {
            _type = type;
            _value = value;
        }

        public static explicit operator int(Parameter p)
        {
            return Convert.ToInt32(p._value);
        }

        public static explicit operator uint(Parameter p)
        {
            return Convert.ToUInt32(p._value);
        }

        public static explicit operator float(Parameter p)
        {
            return Convert.ToSingle(p._value);
        }

        public static explicit operator string(Parameter p)
        {
            return Convert.ToString(p._value);
        }

        public static explicit operator Entity(Parameter p)
        {
            return (Entity)p._value;
        }

        public T As<T>()
        {
            return (T)Convert.ChangeType(_value, typeof(T));
        }

        public Parameter(string v)
        {
            _type = VariableType.String;
            _value = v;
        }

        public static implicit operator Parameter(string v)
        {
            return new Parameter(v);
        }

        public Parameter(int v)
        {
            _type = VariableType.Integer;
            _value = v;
        }

        public static implicit operator Parameter(int v)
        {
            return new Parameter(v);
        }

        public Parameter(float v)
        {
            _type = VariableType.Float;
            _value = v;
        }

        public static implicit operator Parameter(float v)
        {
            return new Parameter(v);
        }

        public Parameter(Vector3 v)
        {
            _type = VariableType.Vector;
            _value = v;
        }

        public static implicit operator Parameter(Vector3 v)
        {
            return new Parameter(v);
        }

        public Parameter(Entity v)
        {
            _type = VariableType.Entity;
            _value = v;
        }

        public static implicit operator Parameter(Entity v)
        {
            return new Parameter(v);
        }

        public static implicit operator Parameter(bool v)
        {
            return new Parameter((v) ? 1 : 0);
        }

        public Parameter(object v)
        {
            _value = v;

            var typeName = v.GetType().Name;

            if (typeName == typeof(float).Name)
            {
                _type = VariableType.Float;
            }
            else if (typeName == typeof(int).Name || typeName == typeof(uint).Name || typeName == typeof(ushort).Name || typeName == typeof(short).Name || typeName == typeof(bool).Name)
            {
                _type = VariableType.Integer;
            }
            else if (typeName == typeof(string).Name)
            {
                _type = VariableType.String;
            }
            else if (typeName == typeof(Entity).Name)
            {
                _type = VariableType.Entity;
            }
            else if (typeName == typeof(Vector3).Name)
            {
                _type = VariableType.Vector;
            }
            else
            {
                throw new InvalidCastException("Type " + typeName + " can not be converted to Parameter");
            }
        }

        internal void PushValue()
        {
            switch (_type)
            {
                case VariableType.Float:
                    GameInterface.Script_PushFloat(Convert.ToSingle(_value));
                    break;
                case VariableType.Integer:
                    GameInterface.Script_PushInt(Convert.ToInt32(_value));
                    break;
                case VariableType.String:
                    GameInterface.Script_PushString(Convert.ToString(_value));
                    break;
                case VariableType.Entity:
                    GameInterface.Script_PushEntRef(((Entity)_value).EntRef);
                    break;
                case VariableType.Vector:
                    var v = (Vector3)_value;
                    GameInterface.Script_PushVector(v.X, v.Y, v.Z);
                    break;
            }
        }

        public bool IsNull
        {
            get
            {
                return _value == null;
            }
        }

        public override string ToString()
        {
            return _value.ToString();
        }
    }
}
