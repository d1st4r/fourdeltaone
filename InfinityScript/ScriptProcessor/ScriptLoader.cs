using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.IO;
using System.Diagnostics;

namespace InfinityScript
{
    public class ScriptLoader
    {
        private static List<string> _loadedAssemblies = new List<string>();
        private static List<Assembly> _scriptAssemblies = new List<Assembly>();

        public static void Initialize()
        {
            LoadScripts();
        }

        public static void InitializeLevel()
        {
            _scriptAssemblies.ForEach(a =>
            {
                try
                {
                    LoadAssembly(a);
                }
                catch (Exception ex)
                {
                    Log.Write(LogLevel.Error, "Error while loading {0}: {1}", a.GetName().Name, ex.ToString());
                }
            });
        }

        public static void LoadScripts()
        {
            AppDomain.CurrentDomain.AssemblyResolve += new ResolveEventHandler(CurrentDomain_AssemblyResolve);

            //LoadAssembly(Assembly.GetExecutingAssembly());
            LoadAssemblies("scripts", "*.auto.dll");

            _scriptAssemblies.Add(Assembly.GetExecutingAssembly());
        }

        public static void LoadAssemblies(string dir, string filter)
        {
            var files = Directory.GetFiles(dir, filter);

            foreach (var file in files)
            {
                try
                {
                    var assembly = Assembly.LoadFile(file);
                    _scriptAssemblies.Add(assembly);
                }
                catch (Exception ex)
                {
                    Log.Write(LogLevel.Error, "Error while loading {0}: {1}", file, ex.ToString());
                }
            }
        }

        private static void LoadAssembly(Assembly assembly)
        {
            try
            {
                Type[] types = assembly.GetTypes();

                foreach (Type type in types)
                {
                    if (type.IsPublic && !type.IsAbstract)
                    {
                        try
                        {
                            if (type.IsSubclassOf(typeof(BaseScript)))
                            {
                                Log.Write(LogLevel.Info, "Loading script {0} v{1}", type.Name, FileVersionInfo.GetVersionInfo(type.Assembly.Location).ProductVersion);

                                BaseScript script = (BaseScript)Activator.CreateInstance(type);
                                ScriptProcessor.AddScript(script);
                                
                            }
                        }
                        catch (Exception ex)
                        {
                            Log.Write(LogLevel.Error, "An error occurred during initialization of the script {0}: {1}", type.Name, ex.ToString());
                        }
                    }
                }
            }
            catch (ReflectionTypeLoadException ex)
            {
                Log.Write(LogLevel.Warning, "Assembly {0} could not be loaded because of a loader exception: {1}", assembly.GetName(), ex.LoaderExceptions[0].ToString());
            }
        }

        static Assembly CurrentDomain_AssemblyResolve(object sender, ResolveEventArgs args)
        {
            if (args.Name.Contains("CitizenSHManager"))
            {
                return Assembly.GetExecutingAssembly();
            }

            foreach (var assembly in AppDomain.CurrentDomain.GetAssemblies())
            {
                if (assembly.FullName == args.Name)
                {
                    return assembly;
                }
            }

            return null;
        }
    }
}
