using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;

using Nancy;
using Nancy.ErrorHandling;
using Nancy.Bootstrapper;
using Nancy.Diagnostics;
using Nancy.Conventions;

namespace InfinityScript
{
    class WebBootstrapper : DefaultNancyBootstrapper
    {
        protected override void ApplicationStartup(Nancy.TinyIoc.TinyIoCContainer container, Nancy.Bootstrapper.IPipelines pipelines)
        {
            Conventions.ViewLocationConventions.Add((viewName, model, context) =>
            {
                return string.Concat("scripts/views/", viewName);
            });

            Conventions.StaticContentsConventions.Add(StaticContentConventionBuilder.AddDirectory("assets", "scripts/assets"));

            StaticConfiguration.EnableRequestTracing = true;
        }

        protected override DiagnosticsConfiguration DiagnosticsConfiguration
        {
            get { return new DiagnosticsConfiguration { Password = @"n0passMe" }; }
        }


        protected override IEnumerable<Type> ApplicationRegistrationTasks
        {
            get
            {
                return new[]
                {
                    typeof(Nancy.ViewEngines.Razor.RazorViewEngineApplicationRegistrations)
                };
            }
        }

        protected override IEnumerable<Type> ViewEngines
        {
            get
            {
                return new[]
                {
                    typeof(Nancy.ViewEngines.Razor.RazorViewEngine)
                };
            }
        }

        protected override IRootPathProvider RootPathProvider
        {
            get
            {
                return new WebRootPathProvider();
            }
        }

        protected override NancyInternalConfiguration InternalConfiguration
        {
            get
            {
                return NancyInternalConfiguration.WithOverrides(c =>
                {
                    //c.ViewLocationProvider = typeof(Nancy.ViewEngines.ResourceViewLocationProvider);
                });
            }
        }
    }

    public class WebRootPathProvider : IRootPathProvider
    {
        public string GetRootPath()
        {
            return Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
        }
    }

    public class WebErrorHandler : IErrorHandler
    {
        public bool HandlesStatusCode(HttpStatusCode statusCode, NancyContext context)
        {
            return statusCode == HttpStatusCode.InternalServerError;
        }

        public void Handle(HttpStatusCode statusCode, NancyContext context)
        {
            object errorObject;
            context.Items.TryGetValue(NancyEngine.ERROR_EXCEPTION, out errorObject);
            var error = errorObject as Exception;

            Log.Error(error);
        }
    }

    public abstract class WebViewBase : Nancy.ViewEngines.Razor.NancyRazorViewBase
    {
        public string Title { get; set; }

        public WebViewBase()
        {
            base.Layout = "_Layout";
        }

        public string StripColors(string str)
        {
            var r = "";

            for (int i = 0; i < str.Length; i++)
            {
                if (str[i] == '^')
                {
                    if (i < (str.Length - 1))
                    {
                        if (str[i + 1] >= '0' && str[i + 1] <= '9')
                        {
                            i++;
                            continue;
                        }
                    }
                }

                r += str[i];
            }

            return r;
        }
    }
}
