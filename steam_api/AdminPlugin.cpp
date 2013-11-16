#include "StdInc.h"
#include "AdminPlugin.h"

// wat
#if USE_MANAGED_CODE
typedef struct gentity_s {
	unsigned char pad[628];
} gentity_t;

extern gentity_t* entities;
extern int* maxclients;

typedef DWORD (__cdecl* SV_GameClientNum_Score_t)(int clientID);
extern SV_GameClientNum_Score_t SV_GameClientNum_Score;

typedef void (__cdecl * Cbuf_AddText_t)(int a1, char* cmd);
extern Cbuf_AddText_t Cbuf_AddText;

typedef void (__cdecl * Cmd_ExecuteSingleCommand_t)(int a1, int a2, char* cmd);
extern Cmd_ExecuteSingleCommand_t Cmd_ExecuteSingleCommand;

//typedef char* (__cdecl * dvar_toString_t)(dvar_t* cvar);
//dvar_toString_t dvar_toString_f = (dvar_toString_t)0x484DD0;

typedef dvar_t* (__cdecl * Dvar_FindVar_t)(char*);
extern Dvar_FindVar_t Dvar_FindVar;

#define AP_OUTPUTBUF_LENGTH ( 4096 - 16 )
char ap_outputbuf[4096];

void AP_FlushRedirect(char* outputbuf);

char* dvar_toString(dvar_t* cvar)
{
	switch (cvar->type)
	{
		case 0:
			return (cvar->current.boolean) ? "1" : "0";
		case 1:
			return va("%g", cvar->current.value);
		case 5:
		case 6: // enum
			return va("%i", cvar->current.integer);
		case 7:
			return va("%s", cvar->current.string);
		default:
			return "";
	}
}

namespace IW4
{
	void AdminPluginCode::Initialize()
	{
		_plugins = gcnew List<AdminPluginBase^>();

		Log::Initialize("IW4Ext.log", LogLevel::Debug | LogLevel::Info | LogLevel::Warning | LogLevel::Error, true);
		Log::Debug("AdminPluginCode::Initialize");

		AppDomain::CurrentDomain->AssemblyResolve += gcnew ResolveEventHandler(&AdminPluginCode::ResolveAssembly);

		LoadScripts();
	}

	bool AdminPluginCode::TriggerSay(void* entity, char* cname, char* cmessage)
	{
		AdminClient^ client = AdminClient::Get(entity);

		return TriggerSay(client, cname, cmessage);
	}

	bool AdminPluginCode::TriggerSay(int clientNum, char* cname, char* cmessage)
	{
		AdminClient^ client = AdminClient::Get(clientNum);

		return TriggerSay(client, cname, cmessage);
	}

	bool AdminPluginCode::TriggerSay(AdminClient^ client, char* cname, char* cmessage)
	{
			String^ message = gcnew String(cmessage);
			client->SetName(gcnew String(cname));

			bool globalEat = false;

			for each (AdminPluginBase^ plugin in _plugins)
			{
				EventEat eat = EventEat::EatNone;

				try
				{
					eat = plugin->OnSay(client, message);
				}
				catch (Exception^ e)
				{
					Log::Error(e);
				}

				if (eat == EventEat::EatAll || eat == EventEat::EatGame)
				{
					globalEat = true;
				}

				if (eat == EventEat::EatAll || eat == EventEat::EatPlugins)
				{
					break;
				}
			}

			return globalEat;
	}

	void AdminPluginCode::TriggerFrame()
	{
		for each (AdminPluginBase^ plugin in _plugins)
		{
			try
			{
				plugin->OnFrame();
			}
			catch (Exception^ e)
			{
				Log::Error(e);
			}
		}
	}

	Assembly^ AdminPluginCode::ResolveAssembly(Object^ sender, ResolveEventArgs^ args) {
		if (args->Name->Contains("steam_api")) {
			Log::Debug("oh, somebody asked for us. let's return 'us'.");
			return Assembly::GetExecutingAssembly();
		}

		for each (Assembly^ assembly in AppDomain::CurrentDomain->GetAssemblies()) {
			if (assembly->FullName == args->Name) {
				return assembly;
			}
		}

		for each (String^ file in Directory::GetFiles("plugins")) {
			try {
				String^ name = FileInfo(file).FullName;

				if (AssemblyName::GetAssemblyName(name)->FullName == args->Name) {
					return Assembly::LoadFile(name);
				}
			} catch (Exception^) {
				// nothing
			}
		}

		return nullptr;
	}

	void AdminPluginCode::LoadScripts() {
		String^ baseDir = Path::GetDirectoryName(Assembly::GetExecutingAssembly()->Location);

		Log::Debug("AdminPluginCode::LoadScripts");

		LoadAssemblies(baseDir + "\\plugins", "*.dll");
	}

	void AdminPluginCode::LoadAssembly(Assembly^ assembly) {
		try {
			cli::array<Type^>^ types = assembly->GetTypes();

			for each (Type^ type in types) {
				if (type->IsPublic && !type->IsAbstract) {
					try {
						if (type->IsSubclassOf(AdminPluginBase::typeid)) {
							AdminPluginBase^ script = (AdminPluginBase^)Activator::CreateInstance(type);

							LoadScript(script);
						}
					} catch (Exception^ e) {
						Log::Error("An exception occurred during initialization of the script " + type->Name + ".");
						Log::Error(e);
					}
				}
			}
		} catch (ReflectionTypeLoadException^ e) {
			// likely a non-v2hook script
			Log::Warn("Assembly " + assembly->GetName() + " could not be loaded because of a loader exception.");
			Log::Warn("Exception: " + e->LoaderExceptions[0]->ToString()->Split('\n')[0]->Trim());
		}
	}

	void AdminPluginCode::LoadAssemblies(String^ folder, String^ filter) {
		try
		{
			cli::array<String^>^ files = Directory::GetFiles(folder, filter);

			for each (String^ file in files) {
				String^ fileName = FileInfo(file).FullName;

				Assembly^ assembly = Assembly::LoadFile(fileName);

				LoadAssembly(assembly);
			}
		}
		catch (Exception^ e)
		{
			Log::Error(e);
		}
	}

	void AdminPluginCode::LoadScript(AdminPluginBase^ script) {
		Log::Info("Loading plugin " + script->GetType()->Name);

		//ScriptProcessor::Instance->AddScript(script);
		_plugins->Add(script);
	}

	// SAY CODE
	void AdminPluginBase::SayAll(String^ message)
	{
		SayAll(gcnew String(sv_sayName->current.string), message);
	}

	void AdminPluginBase::SayAll(String^ name, String^ message)
	{
		IntPtr nameP = Runtime::InteropServices::Marshal::StringToHGlobalAnsi(name);
		IntPtr messageP = Runtime::InteropServices::Marshal::StringToHGlobalAnsi(message);

		G_SayToAll(0xFFFFFFFF, (char*)nameP.ToPointer(), (char*)messageP.ToPointer());

		Runtime::InteropServices::Marshal::FreeHGlobal(nameP);
		Runtime::InteropServices::Marshal::FreeHGlobal(messageP);
	}

	void AdminPluginBase::SayTo(AdminClient^ client, String^ message)
	{
		SayTo(client->ClientNum, message);
	}

	void AdminPluginBase::SayTo(AdminClient^ client, String^ name, String^ message)
	{
		SayTo(client->ClientNum, name, message);
	}

	void AdminPluginBase::SayTo(int clientNum, String^ message)
	{
		SayTo(clientNum, gcnew String(sv_sayName->current.string), message);
	}

	void AdminPluginBase::SayTo(int clientNum, String^ name, String^ message)
	{
		IntPtr nameP = Runtime::InteropServices::Marshal::StringToHGlobalAnsi(name);
		IntPtr messageP = Runtime::InteropServices::Marshal::StringToHGlobalAnsi("^2(pm): ^7" + message);

		G_SayToClient(clientNum, 0xFFFFFFFF, (char*)nameP.ToPointer(), (char*)messageP.ToPointer());

		Runtime::InteropServices::Marshal::FreeHGlobal(nameP);
		Runtime::InteropServices::Marshal::FreeHGlobal(messageP);
	}

	// MW2-SPECIFIC CODE
	AdminClient^ AdminClient::Get(void* entity)
	{
		for (int i = 0; i < *maxclients; i++)
		{
			gentity_t* gentity = &entities[i];

			if ((DWORD)gentity == (DWORD)entity)
			{
				return Get(i);
			}
		}

		return nullptr;
	}

	AdminClient^ AdminClient::Get(int clientNum)
	{
		BYTE* clientAddress = (BYTE*)0x31D9390;

		for (int i = 0; i < *(int*)0x31D938C; i++) {
			if (*clientAddress >= 3) { // connected
				if (i == clientNum)
				{
					char* guid = (char*)(clientAddress + 269020);
					char myGUID[33];
					memset(myGUID, 0, sizeof(myGUID));
					memcpy(myGUID, guid, 32);

					Int64 nguid = Int64::Parse((gcnew String(myGUID))->Trim(), NumberStyles::AllowHexSpecifier);

					char* cName = (char*)(clientAddress + 135844);
					String^ name = gcnew String(cName);

					int score = SV_GameClientNum_Score(i);
					short ping = *(short*)(clientAddress + 135880);

					AdminClient^ client = gcnew AdminClient();
					client->SetData(nguid, name, i);
					client->SetGameData(ping, score);

					return client;
				}
			}

			clientAddress += 681872;
		}

		return nullptr;
	}

	List<AdminClient^>^ AdminPluginBase::GetClients()
	{
		List<AdminClient^>^ retval = gcnew List<AdminClient^>();
		
		BYTE* clientAddress = (BYTE*)0x31D9390;

		for (int i = 0; i < *(int*)0x31D938C; i++) {
			if (*clientAddress >= 3) { // connected
				char* guid = (char*)(clientAddress + 269020);
				char myGUID[33];
				memset(myGUID, 0, sizeof(myGUID));
				memcpy(myGUID, guid, 32);

				Int64 nguid = Int64::Parse((gcnew String(myGUID))->Trim(), NumberStyles::AllowHexSpecifier);

				char* cName = (char*)(clientAddress + 135844);
				String^ name = gcnew String(cName);

				int score = SV_GameClientNum_Score(i);
				short ping = *(short*)(clientAddress + 135880);

				AdminClient^ client = gcnew AdminClient();
				client->SetData(nguid, name, i);
				client->SetGameData(ping, score);

				retval->Add(client);
			}

			clientAddress += 681872;
		}

		return retval;
	}

	void AdminPluginBase::ExecuteCommand(String^ command)
	{
		IntPtr commandP = Runtime::InteropServices::Marshal::StringToHGlobalAnsi(command);

		Cbuf_AddText(0, (char*)commandP.ToPointer());

		Runtime::InteropServices::Marshal::FreeHGlobal(commandP);
	}

	void AdminPluginBase::ExecuteCommand(String^ command, CommandRedirectDelegate^ redirect)
	{
		_redirectTarget = redirect;

		IntPtr commandP = Runtime::InteropServices::Marshal::StringToHGlobalAnsi(command);

		Com_BeginRedirect( ap_outputbuf, AP_OUTPUTBUF_LENGTH, AP_FlushRedirect );
		Cmd_ExecuteSingleCommand(0, 0, (char*)commandP.ToPointer());
		Com_EndRedirect();

		Runtime::InteropServices::Marshal::FreeHGlobal(commandP);
	}

	void AdminPluginBase::RedirectThis(char* value)
	{
		_redirectTarget(gcnew String(value));
	}
	
	String^ AdminPluginBase::GetDvar(String^ name)
	{
		IntPtr nameP = Runtime::InteropServices::Marshal::StringToHGlobalAnsi(name);
		dvar_t* cvar = Dvar_FindVar((char*)nameP.ToPointer());
		Runtime::InteropServices::Marshal::FreeHGlobal(nameP);

		if (!cvar)
		{
			return "";
		}

		// shouldn't call this func directly from managed code, va()'s TLS usage messes up
		return gcnew String(dvar_toString(cvar));
	}

	void AdminPluginBase::SetDvar(String^ name, String^ value)
	{
		ExecuteCommand(String::Format("set {0} \"{1}\"", name, value));
	}
}

bool APC_TriggerSay(void* entity, char* name, char* message)
{
	return IW4::AdminPluginCode::TriggerSay(entity, name, message);
}

void APC_TriggerFrame()
{
	return IW4::AdminPluginCode::TriggerFrame();
}

void AP_FlushRedirect(char* outputbuf)
{
	IW4::AdminPluginBase::RedirectThis(outputbuf);
}
#endif