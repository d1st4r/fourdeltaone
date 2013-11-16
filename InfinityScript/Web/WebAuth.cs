using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;

using Nancy;
using Nancy.Cookies;

namespace InfinityScript.Web
{
    public class WebAuth : WebFrontend
    {
        public WebAuth()
            : base()
        {
            Get["/login"] = parameters =>
            {
                if (Request.Query.Sid != null)
                {
                    if (!CheckSID(Request.Query.Sid))
                    {
                        return Response.AsRedirect("/login");
                    }

                    return Response.AsRedirect("/").AddCookie("fdo_sid", Request.Query.Sid);
                }

                var errorMsg = "";

                if (Request.Cookies.ContainsKey("error"))
                {
                    var error = Request.Cookies["error"];

                    if (error == "1")
                    {
                        errorMsg = "This account is not permitted to administer this server.";
                    }
                }

                return View["AuthenticateSSO", new { ErrorMsg = errorMsg }].WithCookie(new NancyCookie("error", ""));
            };

            Post["/login"] = parameters =>
            {
                return Response.AsRedirect("http://fourdeltaone.net/sso?redirect=" + Uri.EscapeUriString(Request.Url.ToString()));
            };

            Get["/associate"] = parameters =>
            {
                return View["AssociateServer", new { ErrorMsg = "" }];
            };

            Post["/associate"] = parameters =>
            {
                string errorMsg = null;

                if (!string.IsNullOrWhiteSpace(Request.Form.Password))
                {
                    if (Request.Form.Password == WebBaseScript.RconPassword)
                    {
                        WebBaseScript.OwnerID = Context.ViewBag.UserID;
                        WebBaseScript.NewOwnerID = Context.ViewBag.UserID;

                        return Response.AsRedirect("/");
                    }
                    else
                    {
                        errorMsg = "The password did not match the remote console password set for the server.";
                    }
                }
                else
                {
                    errorMsg = "No password was entered.";
                }

                return View["AssociateServer", new { ErrorMsg = errorMsg }];
            };
        }

        private bool CheckSID(string sid)
        {
            var wc = new WebClient();
            var result = wc.DownloadString("http://fourdeltaone.net/sso/check_sid/" + sid + "/" + Request.UserHostAddress);

            var data = result.Split(new[] { "\r\n" }, StringSplitOptions.RemoveEmptyEntries);

            if (data[0] == "nope")
            {
                return false;
            }

            if (data.Length != 3)
            {
                return false;
            }

            // sort-of checksum if some headers precede the response
            if (string.Join("", data[1].Reverse().ToArray()) != data[2])
            {
                return false;
            }

            WebFrontend._sessionIDs[sid] = new RemoteUserData() { UserID = int.Parse(data[0]), UserName = data[1] };

            return true;

        }
    }
}
