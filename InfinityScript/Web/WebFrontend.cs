using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Nancy;
using System.Net;
using Nancy.Cookies;

namespace InfinityScript
{
    struct RemoteUserData
    {
        public int UserID { get; set; }
        public string UserName { get; set; }
    }

    public class WebFrontend : NancyModule
    {
        internal static Dictionary<string, RemoteUserData> _sessionIDs = new Dictionary<string, RemoteUserData>();

        public WebFrontend()
        {
            Before.AddItemToEndOfPipeline(ctx =>
            {
                ctx.ViewBag.LoggedIn = false;

                if (ctx.Request.Cookies.ContainsKey("fdo_sid"))
                {
                    var sid = ctx.Request.Cookies["fdo_sid"];

                    if (!ctx.Request.Cookies.ContainsKey("error") || ctx.Request.Cookies["error"] == "")
                    {
                        RemoteUserData rud;
                        if (_sessionIDs.TryGetValue(sid, out rud))
                        {
                            ctx.ViewBag.LoggedIn = true;
                            ctx.ViewBag.UserID = rud.UserID;
                            ctx.ViewBag.UserName = rud.UserName;
                        }
                    }
                }

                return null;
            });

            Before.AddItemToEndOfPipeline(ctx =>
            {
                if (!ctx.ViewBag.LoggedIn)
                {
                    if (ctx.Request.Path != "/login")
                    {
                        return ProcessAuthentication(ctx);
                    }
                }
                else
                {
                    if (WebBaseScript.OwnerID == 0)
                    {
                        ctx.ViewBag.LoggedIn = false;

                        if (ctx.Request.Path != "/associate")
                        {
                            return Response.AsRedirect("/associate");
                        }
                    }
                    else
                    {
                        if (ctx.ViewBag.UserID != WebBaseScript.OwnerID)
                        {
                            ctx.ViewBag.LoggedIn = false;
                            return Response.AsRedirect("/login").AddCookie("error", "1");/*AddCookie("fdo_sid", "", DateTime.UtcNow - new TimeSpan(30, 0, 0, 0));*/
                        }
                    }
                }

                return null;
            });
        }

        private Response ProcessAuthentication(NancyContext ctx)
        {
            return Response.AsRedirect("/login");
        }
    }
}
