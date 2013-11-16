using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;

using Nancy.Bootstrapper;
using System.Threading.Tasks;
using Nancy;
using Nancy.IO;
using System.IO;
using System.Net;

namespace InfinityScript
{
    class WebManager
    {
        private static INancyEngine _engine;

        public static void Initialize()
        {
            var bootstrapper = new WebBootstrapper();
            bootstrapper.Initialise();
            _engine = bootstrapper.GetEngine();
            //_host = new NancyOwinHost(null, NancyBootstrapperLocator.Bootstrapper);
        }

        public static byte[] HandleRequest(string method, string uri, string query, Dictionary<string, string> headers, IPAddress ip)
        {
            var nuri = new Url
            {
                Scheme = "http",
                HostName = GetHeader(headers, "Host"),
                Port = null,
                BasePath = "",
                Path = uri,
                Query = query
            };

            var contentLength = ExpectedLength(headers);
            var bodyStream = Stream.Null;

            if (contentLength > 0)
            {
                var bodyBytes = GameInterface.ReadHTTPBody((int)contentLength);
                bodyStream = new MemoryStream(bodyBytes);
            }

            var body = new RequestStream(bodyStream, contentLength, false);

            var headerDictionary = headers.ToDictionary(kv => kv.Key, kv => (IEnumerable<string>)new[] { kv.Value }, StringComparer.OrdinalIgnoreCase);
            var request = new Request(method, nuri, body, headerDictionary, ip.ToString());

            var context = _engine.HandleRequest(request);
            var responseStatusCode = context.Response.StatusCode;
            var responseStream = new MemoryStream();
            context.Response.Contents(responseStream);
            responseStream.Position = 0;

            var response = new StringBuilder();
            response.Append("HTTP/1.0 ");
            response.Append(((int)responseStatusCode).ToString());
            response.Append(" ");
            response.Append(_statusCodes[(int)responseStatusCode]);
            response.Append("\r\n");

            foreach (var header in context.Response.Headers)
            {
                response.Append(header.Key);
                response.Append(": ");
                response.Append(string.Join(", ", header.Value));
                response.Append("\r\n");
            }

            if (context.Response.ContentType != null)
            {
                response.Append("Content-Type: ");
                response.Append(context.Response.ContentType);
                response.Append("\r\n");
            }

            if (context.Response.Cookies != null && context.Response.Cookies.Count != 0)
            {
                var cookies = context.Response.Cookies.Select(cookie => cookie.ToString()).ToArray();

                response.Append("Set-Cookie: ");
                response.Append(string.Join(", ", cookies));
                response.Append("\r\n");
            }


            response.Append("\r\n");

            var headerBytes = Encoding.UTF8.GetBytes(response.ToString());
            var dataBytes = new byte[responseStream.Length + headerBytes.Length];
            Array.Copy(headerBytes, dataBytes, headerBytes.Length);

            responseStream.Read(dataBytes, headerBytes.Length, (int)responseStream.Length);

            //Log.Debug(context.Trace.TraceLog.ToString());
            context.Dispose();

            return dataBytes;
        }

        private static string GetHeader(IDictionary<string, string> headers, string key)
        {
            string value;
            return headers.TryGetValue(key, out value) && value != null ? value : null;
        }

        private static long ExpectedLength(IDictionary<string, string> headers)
        {
            var header = GetHeader(headers, "Content-Length");
            if (string.IsNullOrWhiteSpace(header))
                return 0;

            int contentLength;
            return int.TryParse(header, NumberStyles.Any, CultureInfo.InvariantCulture, out contentLength) ? contentLength : 0;
        }


        static WebManager()
        {
            if (_statusCodes == null)
            {
                _statusCodes = new Dictionary<int, string>();

                _statusCodes[100] = "Continue";
                _statusCodes[101] = "Switching Protocols";
                _statusCodes[200] = "OK";
                _statusCodes[201] = "Created";
                _statusCodes[202] = "Accepted";
                _statusCodes[203] = "Non-Authoritative Information";
                _statusCodes[204] = "No Content";
                _statusCodes[205] = "Reset Content";
                _statusCodes[206] = "Partial Content";
                _statusCodes[300] = "Multiple Choices";
                _statusCodes[301] = "Moved Permanently";
                _statusCodes[302] = "Found";
                _statusCodes[303] = "See Other";
                _statusCodes[304] = "Not Modified";
                _statusCodes[305] = "Use Proxy";
                _statusCodes[307] = "Temporary Redirect";
                _statusCodes[400] = "Bad Request";
                _statusCodes[401] = "Unauthorized";
                _statusCodes[402] = "Payment Required";
                _statusCodes[403] = "Forbidden";
                _statusCodes[404] = "Not Found";
                _statusCodes[405] = "Method Not Allowed";
                _statusCodes[406] = "Not Acceptable";
                _statusCodes[407] = "Proxy Authentication Required";
                _statusCodes[408] = "Request Time-out";
                _statusCodes[409] = "Conflict";
                _statusCodes[410] = "Gone";
                _statusCodes[411] = "Length Required";
                _statusCodes[412] = "Precondition Failed";
                _statusCodes[413] = "Request Entity Too Large";
                _statusCodes[414] = "Request-URI Too Large";
                _statusCodes[415] = "Unsupported Media Type";
                _statusCodes[416] = "Requested range not satisfiable";
                _statusCodes[417] = "Expectation Failed";
                _statusCodes[500] = "Internal Server Error";
                _statusCodes[501] = "Not Implemented";
                _statusCodes[502] = "Bad Gateway";
                _statusCodes[503] = "Service Unavailable";
                _statusCodes[504] = "Gateway Time-out";
                _statusCodes[505] = "HTTP Version not supported";
            }
        }

        private static Dictionary<int, string> _statusCodes;
    }

    public class SimpleTaskScheduler : TaskScheduler
    {
        public override int MaximumConcurrencyLevel
        {
            get
            {
                return 1;
            }
        }

        protected override void QueueTask(Task task)
        {
            TryExecuteTask(task);
        }

        protected override IEnumerable<Task> GetScheduledTasks()
        {
            return new List<Task>();
        }

        protected override bool TryExecuteTaskInline(Task task, bool taskWasPreviouslyQueued)
        {
            return TryExecuteTask(task);
        }
    }
}
