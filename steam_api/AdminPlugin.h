#pragma once

#if USE_MANAGED_CODE
#include "Log.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::IO;
using namespace System::Reflection;

ref class MatchEntry;

namespace IW4
{
	public enum struct EventEat
	{
		EatNone,
		EatGame,
		EatPlugins,
		EatAll
	};

	public ref class AdminClient
	{
	private:
		Int64 _guid;
		String^ _name;
		Int16 _clientID;
		Int16 _ping;
		int _score;
	internal:
		AdminClient() {}

		void SetData(Int64 guid, String^ name, Int16 clientNum)
		{
			_guid = guid;
			_name = name;
			_clientID = clientNum;
		}

		void SetGameData(Int16 ping, int score)
		{
			_ping = ping;
			_score = score;
		}

		void SetName(String^ name)
		{
			_name = name;
		}
	public:
		property Int64 GUID
		{
			Int64 get() { return _guid; }
		}

		property String^ Name
		{
			String^ get() { return _name; }
		}

		property Int16 ClientNum
		{
			Int16 get() { return _clientID; }
		}

		property Int16 Ping
		{
			Int16 get() { return _ping; }
		}

		property int Score
		{
			int get() { return _score; }
		}

		static AdminClient^ Get(void* entity);
		static AdminClient^ Get(int clientNum);
	};

	public delegate void CommandRedirectDelegate(String^ data);

	public ref class AdminPluginBase abstract
	{
	private:
		static CommandRedirectDelegate^ _redirectTarget;
	internal:
		static void RedirectThis(char* value);
	public:
		virtual EventEat OnSay(AdminClient^ client, String^ message) { return EventEat::EatNone; }
		virtual void OnFrame() {}

		List<AdminClient^>^ GetClients();

		void SayAll(String^ name, String^ message);
		void SayAll(String^ message);

		void SayTo(AdminClient^ client, String^ message);
		void SayTo(AdminClient^ client, String^ name, String^ message);

		void SayTo(int clientNum, String^ message);
		void SayTo(int clientNum, String^ name, String^ message);

		void ExecuteCommand(String^ command);
		void ExecuteCommand(String^ command, CommandRedirectDelegate^ redirect);

		String^ GetDvar(String^ name);
		void SetDvar(String^ name, String^ value);
	};

	ref class AdminPluginCode
	{
	private:
		static List<AdminPluginBase^>^ _plugins;
	internal:
		static void LoadScripts();
		static void LoadAssemblies(String^ folder, String^ filter);
	public:
		static void Initialize();

		static Assembly^ ResolveAssembly(Object^ sender, ResolveEventArgs^ args);

		static bool TriggerSay(void* entity, char* name, char* message);
		static bool TriggerSay(int clientNum, char* name, char* message);
		static bool TriggerSay(AdminClient^ client, char* name, char* message);

		static void TriggerFrame();

		static void LoadAssembly(Assembly^ assembly);
		static void LoadScript(AdminPluginBase^ script);
	};
};

bool APC_TriggerSay(void* entity, char* name, char* message);
void APC_TriggerFrame();
#endif