using System;
using System.Collections.Generic;
using System.Dynamic;
using System.Linq;
using System.Text;

namespace InfinityScript
{
    public class ScriptDynamics : DynamicObject
    {
        private int _entRef;

        public ScriptDynamics(int entRef)
        {
            _entRef = entRef;
        }

        public override bool TryInvokeMember(InvokeMemberBinder binder, object[] args, out object result)
        {
            var functionName = binder.Name;

            try
            {
                // convert arguments to Parameter
                var parameters = new Parameter[args.Length];

                for (int i = 0; i < args.Length; i++)
                {
                    parameters[i] = new Parameter(args[i]);
                }

                Function.SetEntRef(_entRef);
                result = Function.Call(functionName, typeof(object), parameters);

                return true;
            }
            catch (Exception ex)
            {
                Log.Error(ex);

                result = null;

                return false;
            }
        }
    }
}
