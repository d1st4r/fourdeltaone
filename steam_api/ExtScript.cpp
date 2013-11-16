// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Game extensibility code.
//
// Initial author: NTAuthority
// Started: 2012-04-21
// ==========================================================

#include "StdInc.h"
#include "ExtScript.h"

#if USE_EXTSCRIPT
#define BOOST_PYTHON_STATIC_LIB
#define Py_NO_ENABLE_SHARED
#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>

static struct  
{
	// true if initialized without errors
	bool initialized;

	// application object for web cases
	boost::python::object application;

	// m2admin module
	boost::python::object m2admin;
	
	// python call lock
	CRITICAL_SECTION pyLock;
} g_ext;

class iw4http
{
public:
	static void set_server(boost::python::object application)
	{
		g_ext.application = application;
	}
};

struct iw4client
{
	int num;
	std::string name;
	std::string address;
	int ping;
	int score;
	int uid;
};

typedef DWORD (__cdecl* SV_GameClientNum_Score_t)(int clientID);
extern SV_GameClientNum_Score_t SV_GameClientNum_Score;

class iw4api
{
public:
	static boost::python::object get_dvar(boost::python::str dvarName)
	{
		std::string dvarNameStr = boost::python::extract<std::string>(dvarName)();
		dvar_t* dvar = Dvar_FindVar(const_cast<char*>(dvarNameStr.c_str()));

		if (dvar == NULL)
		{
			return boost::python::object();
		}

		switch (dvar->type)
		{
				case DVAR_TYPE_BOOL:
					return boost::python::object(dvar->current.boolean);
				case DVAR_TYPE_FLOAT:
					return boost::python::object(dvar->current.value);
				case DVAR_TYPE_INT:
				case DVAR_TYPE_ENUM:
					return boost::python::object(dvar->current.integer);
				case DVAR_TYPE_STRING:
					{
						std::string str = dvar->current.string;
						return boost::python::object(str);
					}
				default:
					return boost::python::object();
		}
	}

	static boost::python::list get_clients()
	{
		boost::python::list retval;

		for (int i = 0; i < *svs_numclients; i++)
		{
			if (svs_clients[i].state >= 3)
			{
				retval.append(get_client(i));
			}
		}

		return retval;
	}

	static boost::shared_ptr<iw4client> get_client(int i)
	{
		if (i < 0 || i > *svs_numclients)
		{
			return boost::shared_ptr<iw4client>();
		}

		boost::shared_ptr<iw4client> retval(new iw4client);
		retval->num = i;
		retval->name = Info_ValueForKey(svs_clients[i].connectInfoString, "name");
		retval->ping = svs_clients[i].ping;
		retval->score = SV_GameClientNum_Score(i);
		retval->address = NET_AdrToString(svs_clients[i].adr);
		retval->uid = (svs_clients[i].steamid & 0xFFFFFFFF);
		return retval;
	}

	static void execute(std::string command)
	{
		command += "\n";

		Cbuf_AddText(0, command.c_str());
	}
};

BOOST_PYTHON_MODULE(iw4m)
{
	boost::python::class_<iw4http>("m2http")
		.def("set_server", &iw4http::set_server)
		.staticmethod("set_server");

	boost::python::class_<iw4api>("m2")
		.def("get_dvar", &iw4api::get_dvar)
		.def("get_clients", &iw4api::get_clients)
		.def("get_client", &iw4api::get_client)
		.def("execute", &iw4api::execute)
		.staticmethod("get_dvar")
		.staticmethod("get_clients")
		.staticmethod("get_client")
		.staticmethod("execute");

	boost::python::class_<iw4client, boost::shared_ptr<iw4client>>("m2client", boost::python::no_init)
		.def_readonly("ping", &iw4client::ping)
		.def_readonly("name", &iw4client::name)
		.def_readonly("score", &iw4client::score)
		.def_readonly("address", &iw4client::address)
		.def_readonly("uid", &iw4client::uid)
		.def_readonly("num", &iw4client::num);
}

std::string parse_python_exception();

extern "C" PyObject* PyInit__socket();

struct wsgi_response : boost::noncopyable
{
	std::map<std::string, std::string> headers;
	std::string status;

	boost::python::object start_response(boost::python::object status_, boost::python::list headers_, boost::python::object exc_info_ = boost::python::object())
	{
		std::map<std::string, std::string> response_headers;
		for (boost::python::stl_input_iterator<boost::python::tuple> i(headers_), end; i != end; i++)
		{
			response_headers[boost::python::extract<std::string>((*i)[0])()] = boost::python::extract<std::string>((*i)[1])();
		}

		headers = response_headers;
		status = boost::python::extract<std::string>(status_)();
		
		return boost::python::object(&wsgi_response::write);
	}

	void write(boost::python::str string)
	{
		// oops
	}
};

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(start_response_overloads, start_response, 2, 3)

struct wsgi_errors : boost::noncopyable
{
	void flush()
	{
		// no-op
	}

	void write(boost::python::str string)
	{
		std::string line = boost::python::extract<std::string>(string)();
		Com_Printf(0, line.c_str());
	}
};

struct wsgi_input : boost::noncopyable
{
	mg_connection* connection;

	wsgi_input(mg_connection* conn)
	{
		connection = conn;
	}

	boost::python::handle<> read(int size)
	{
		char* buffer = new char[size];
		mg_read(connection, buffer, size);

		// boost.python doesn't seem to support bytes returning
		PyObject* bytes = PyBytes_FromStringAndSize(buffer, size);
		boost::python::handle<> handle(bytes);

		delete[] buffer;

		return handle;
	}
};

void Ext_ReloadWeb();

void Ext_Initialize()
{
	static cmd_function_t web_reload;

	Com_Printf(0, "Ext_Initialize...\n");

	if (!FileExists("main\\python32.zip"))
	{
		Com_Printf(0, "no main\\python32.zip found, skipping this call\n");
		return;
	}

	InitializeCriticalSection(&g_ext.pyLock);

	try
	{
		PyImport_AppendInittab("_socket", &PyInit__socket);
		PyImport_AppendInittab("iw4m", &PyInit_iw4m);
		Py_SetPath(L"main\\m2;main\\python32.zip;main\\m2.zip");
		Py_Initialize();

		g_ext.m2admin = boost::python::import("m2admin");

	    boost::python::class_<wsgi_response, boost::shared_ptr<wsgi_response>, boost::noncopyable>(
	        "wsgi_response", boost::python::no_init
		).def("start_response", &wsgi_response::start_response, start_response_overloads(boost::python::args("status_", "headers_", "exc_info_")));

		boost::python::class_<wsgi_errors, boost::shared_ptr<wsgi_errors>, boost::noncopyable>(
			"wsgi_errors", boost::python::no_init
		).def("flush", &wsgi_errors::flush)
		 .def("write", &wsgi_errors::write)
		 /*.def("writelines", &wsgi_errors::writelines)*/;

		boost::python::class_<wsgi_input, boost::shared_ptr<wsgi_input>, boost::noncopyable>(
			"wsgi_input", boost::python::no_init
			).def("read", &wsgi_input::read);

		Cmd_AddCommand("web_reload", Ext_ReloadWeb, &web_reload, 0);

		g_ext.initialized = true;
	}
	catch (boost::python::error_already_set const &)
	{
		std::string error = parse_python_exception();
		Com_Printf(0, "Error during Ext_Initialize:\n%s\n", error.c_str());
	}
}

bool Ext_Initialized()
{
	return g_ext.initialized;
}

void Ext_ReloadWeb()
{
	EnterCriticalSection(&g_ext.pyLock);
	try
	{
		boost::python::object imp = boost::python::import("imp");
		imp.attr("reload")(g_ext.m2admin);
	}
	catch (boost::python::error_already_set const &)
	{
		std::string error = parse_python_exception();
		Com_Printf(0, "Error during Ext_ReloadWeb:\n%s\n", error.c_str());
	}
	LeaveCriticalSection(&g_ext.pyLock);
}

void Ext_HandleWebRequest(mg_connection* conn, const mg_request_info* request_info)
{
	// set up environment
	dvar_t* net_port = Dvar_FindVar("net_port");

	EnterCriticalSection(&g_ext.pyLock);

	try
	{
		boost::shared_ptr<wsgi_errors> errors(new wsgi_errors);
		boost::shared_ptr<wsgi_input> input(new wsgi_input(conn));

		boost::python::dict environ;
		environ["wsgi.url_scheme"] = boost::python::str("http");
		environ["wsgi.version"] = boost::python::make_tuple(1, 0);
		environ["wsgi.multithread"] = true;
		environ["wsgi.multiprocess"] = false;
		environ["wsgi.run_once"] = false;
		environ["wsgi.errors"] = errors;
		environ["wsgi.input"] = input;

		std::string request_method = request_info->request_method;
		std::string uri = request_info->uri;
		std::string query_string = (request_info->query_string) ? request_info->query_string : "";
		std::string server_port = va("%i", net_port->current.integer);
		environ["REQUEST_METHOD"] = boost::python::str(request_method);
		environ["SCRIPT_NAME"] = boost::python::str("");
		environ["PATH_INFO"] = boost::python::str(uri);
		environ["QUERY_STRING"] = boost::python::str(query_string);
		environ["SERVER_NAME"] = boost::python::str("localhost");
		environ["SERVER_PORT"] = boost::python::str(server_port);
		environ["SERVER_PROTOCOL"] = boost::python::str("HTTP/1.0");

		for (int i = 0; i < request_info->num_headers; i++)
		{
			std::string headerName = request_info->http_headers[i].name;
			std::string headerValue = request_info->http_headers[i].value;
			boost::python::str name = boost::python::str(headerName).replace("-", "_").upper();
			environ["HTTP_" + name] = boost::python::str(headerValue);
		}

		const char* contentLength = mg_get_header(conn, "Content-Length");

		if (contentLength)
		{
			std::string length = contentLength;
			environ["CONTENT_LENGTH"] = boost::python::str(length);
		}

		const char* contentType = mg_get_header(conn, "Content-Type");

		if (contentType)
		{
			std::string type = contentType;
			environ["CONTENT_TYPE"] = boost::python::str(type);
		}

		// handle request
		boost::shared_ptr<wsgi_response> response(new wsgi_response);
		boost::python::object responseObject(response);

		boost::python::object str = g_ext.application(environ, boost::python::getattr(responseObject, "start_response"));

		boost::python::stl_input_iterator<boost::python::object> current(str), end;
		boost::python::object first_string;

		if (current != end)
		{
			first_string = *current++;
		}

		mg_printf(conn, "HTTP/1.1 %s\r\n", response->status.c_str());

		for (std::map<std::string, std::string>::iterator i = response->headers.begin(); i != response->headers.end(); i++)
		{
			mg_printf(conn, "%s: %s\r\n", (*i).first.c_str(), (*i).second.c_str());
		}

		mg_printf(conn, "\r\n");

		std::string string;

		// check for None
		if (first_string.ptr() != Py_None)
		{
			string = boost::python::extract<std::string>(first_string)();

			mg_write(conn, string.c_str(), string.size());
		}

		while (current != end)
		{
			boost::python::object curString = *current++;
			if (curString.ptr() != Py_None)
			{
				string = boost::python::extract<std::string>(curString)();

				mg_write(conn, string.c_str(), string.size());
			}
		}
	}
	catch (boost::python::error_already_set const &)
	{
		std::string error = parse_python_exception();
		Com_Printf(0, "Web error:\n%s\n", error.c_str());

		mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\n\r\n");
		mg_write(conn, error.c_str(), error.size());
	}
	
	
	LeaveCriticalSection(&g_ext.pyLock);
}

// taken from http://thejosephturner.com/blog/2011/06/15/embedding-python-in-c-applications-with-boostpython-part-2/
std::string parse_python_exception()
{
	PyObject *type_ptr = NULL, *value_ptr = NULL, *traceback_ptr = NULL;
	PyErr_Fetch(&type_ptr, &value_ptr, &traceback_ptr);
	std::string ret("Unfetchable Python error");

	if (type_ptr != NULL) 
	{
		boost::python::handle<> h_type(type_ptr);
		boost::python::str type_pstr(h_type);
		boost::python::extract<std::string> e_type_pstr(type_pstr);
		if (e_type_pstr.check())
		{
			ret = e_type_pstr();
		}
		else
		{
			ret = "Unknown exception type";
		}
	}

	if (value_ptr != NULL)
	{
		boost::python::handle<> h_val(value_ptr);
		boost::python::str a(h_val);
		boost::python::extract<std::string> returned(a);
		if (returned.check())
		{
			ret +=  ": " + returned();
		}
		else
		{
			ret += ": Unparseable Python error: ";
		}
	}

	if (traceback_ptr != NULL)
	{
		boost::python::handle<> h_tb(traceback_ptr);
		boost::python::object tb(boost::python::import("traceback"));
		boost::python::object fmt_tb(tb.attr("format_tb"));
		boost::python::object tb_list(fmt_tb(h_tb));
		boost::python::object tb_str(boost::python::str("\n").join(tb_list));
		boost::python::extract<std::string> returned(tb_str);
		if (returned.check())
		{
			ret += ": " + returned();
		}
		else
		{
			ret += ": Unparseable Python traceback";
		}
	}
	return ret;
}
#else
void Ext_Initialize()
{

}

bool Ext_Initialized()
{
	return false;
}

void Ext_HandleWebRequest(mg_connection* conn, const mg_request_info* request_info)
{

}
#endif