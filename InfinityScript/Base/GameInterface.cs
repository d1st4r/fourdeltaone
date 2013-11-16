using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;

namespace InfinityScript
{
    public enum VariableType
    {
        Entity = 1,
        String = 2,
        Vector = 4,
        Float = 5,
        Integer = 6
    }

    static class GameInterface
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern void Script_PushString(string str);

        [DllImport("iw4m.dll", EntryPoint = "GI_PushInt")]
        public static extern void Script_PushInt(int value);

        [DllImport("iw4m.dll", EntryPoint = "GI_PushFloat")]
        public static extern void Script_PushFloat(float value);

        [DllImport("iw4m.dll", EntryPoint = "GI_PushEntRef")]
        public static extern void Script_PushEntRef(int value);

        [DllImport("iw4m.dll", EntryPoint = "GI_PushVector")]
        public static extern void Script_PushVector(float x, float y, float z);

        [DllImport("iw4m.dll", EntryPoint = "GI_Call")]
        public static extern bool Script_Call(string functionName, int entref, int numParams);

        [DllImport("iw4m.dll", EntryPoint = "GI_GetType")]
        public static extern VariableType Script_GetType(int index);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern string Script_GetString(int index);

        [DllImport("iw4m.dll", EntryPoint = "GI_GetInt")]
        public static extern int Script_GetInt(int index);

        [DllImport("iw4m.dll", EntryPoint = "GI_GetFloat")]
        public static extern float Script_GetFloat(int index);

        [DllImport("iw4m.dll", EntryPoint = "GI_GetEntRef")]
        public static extern int Script_GetEntRef(int index);

        [DllImport("iw4m.dll", EntryPoint = "GI_GetVector")]
        public static extern void Script_GetVector(int index, out Vector3 vector);

        [DllImport("iw4m.dll", EntryPoint = "GI_NotifyNumArgs")]
        public static extern int Notify_NumArgs();

        [DllImport("iw4m.dll", EntryPoint = "GI_CleanReturnStack")]
        public static extern void Script_CleanReturnStack();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern string Notify_Type();

        [DllImport("iw4m.dll", EntryPoint = "GI_GetObjectType")]
        public static extern int Script_GetObjectType(int obj);

        [DllImport("iw4m.dll", EntryPoint = "GI_ToEntRef")]
        public static extern int Script_ToEntRef(int obj);

        [DllImport("iw4m.dll", EntryPoint = "GI_GetField")]
        public static extern int Script_GetField(int entref, int field);

        [DllImport("iw4m.dll", EntryPoint = "GI_SetField")]
        public static extern int Script_SetField(int entref, int field);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern void Script_NotifyNum(int entref, string notify, int numArgs);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern void Print(string s);

        [DllImport("iw4m.dll", EntryPoint = "GI_SetDropItemEnabled")]
        public static extern bool SetDropItemEnabled(bool enabled);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern string Dvar_InfoString_Big(int flag);

        [DllImport("iw4m.dll", EntryPoint = "GI_AddTestClient")]
        public static extern int AddTestClient();

        [DllImport("iw4m.dll", EntryPoint = "GI_Cmd_Argc")]
        public static extern int Cmd_Argc();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern string Cmd_Argv(int arg);

        [DllImport("iw4m.dll", EntryPoint = "GI_Cmd_Argc_sv")]
        public static extern int Cmd_Argc_sv();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern string Cmd_Argv_sv(int arg);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern void Cbuf_AddText(string command);

        [DllImport("iw4m.dll", EntryPoint = "GI_GetPing")]
        public static extern int GetPing(int entref);

        [DllImport("iw4m.dll", EntryPoint = "GI_GetClientAddress")]
        public static extern long GetClientAddress(int entref);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern void Cmd_TokenizeString(string tokens);

        [DllImport("iw4m.dll", EntryPoint = "GI_Cmd_EndTokenizedString")]
        public static extern void Cmd_EndTokenizedString();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern void Script_NotifyLevel(string notify, int numArgs);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern void SV_GameSendServerCommand(int entid, int id, string message);

        [DllImport("iw4m.dll", EntryPoint = "GI_GetTempEntRef")]
        public static extern int Script_GetTempEntRef();

        // reads a file from the Quake FS
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern byte[] ReadFile(string filename);

        // stuff for admin
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern string GetHTTPHeader(int num);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern byte[] ReadHTTPBody(int length);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern string GetDvar(string dvarName, string defaultValue);

        // do not use
        [DllImport("iw4m.dll", EntryPoint = "GI_TempFunc")]
        public static extern void TempFunc();
    }
}
