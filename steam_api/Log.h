#pragma once

#if USE_MANAGED_CODE
using namespace System;
using namespace System::Collections::Generic;
using namespace System::IO;
using namespace System::Globalization;

namespace IW4 {
	public enum struct LogLevel {
		None = 0,
		Error = 1,
		Warning = 2,
		Info = 4,
		Debug = 8
	};

	public ref class Log {
	private:
		Log() { }

		static StreamWriter^ _logWriter;
		static LogLevel _logLevel;
		static String^ _filename;

		static void Write(String^ message, LogLevel level);
	public:
		static void Initialize(String^ filename, LogLevel logLevel, bool free);

		static bool MayWriteType(LogLevel type);

		static void Debug(String^ message);
		static void Info(String^ message);
		static void Warn(String^ message);
		static void Error(String^ message);
		static void Error(Exception^ exception) { Error(exception->ToString()); }

		static property LogLevel CurrentLogLevel {
			void set(LogLevel logLevel) {
				_logLevel = logLevel;
			}
		}
	};
}
#endif