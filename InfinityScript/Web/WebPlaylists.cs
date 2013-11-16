using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using Nancy;
using Nancy.Responses.Negotiation;

namespace InfinityScript
{
    public class WebPlaylists : WebFrontend
    {
        public WebPlaylists()
            : base()
        {
            Func<string, Negotiator> renderPlaylistsPage = (error) =>
            {
                return View["Playlists"];
            };

            Get["/playlists"] = parameters =>
            {
                return renderPlaylistsPage(null);
            };
        }
    }
}
