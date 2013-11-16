using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

// TODO: reimplement utility functions from _hud_util

namespace InfinityScript
{
    public class HudElem
    {
        public Entity Entity { get; set; }

        #region static uiparent
        private static HudElem _uiParent;

        public static HudElem UIParent
        {
            get
            {
                if (_uiParent == null)
                {
                    _uiParent = new HudElem();
                }

                return _uiParent;
            }
        }

        private HudElem()
        {
            Entity = null;
            Children = new List<HudElem>();
        }
        #endregion

        internal HudElem(Entity entity)
        {
            Entity = entity;
            Children = new List<HudElem>();
        }

        public static HudElem GetHudElem(int entRef)
        {
            return new HudElem(new Entity(entRef));
        }

        public float X
        {
            get
            {
                if (Entity == null)
                {
                    return 0;
                }

                return Entity.GetField<float>("x");
            }
            set
            {
                Entity.SetField("x", value);
            }
        }

        public float Y
        {
            get
            {
                if (Entity == null)
                {
                    return 0;
                }

                return Entity.GetField<float>("y");
            }
            set
            {
                Entity.SetField("y", value);
            }
        }

        public float Z
        {
            get
            {
                return Entity.GetField<float>("z");
            }
            set
            {
                Entity.SetField("z", value);
            }
        }

        public float FontScale
        {
            get
            {
                return Entity.GetField<float>("fontscale");
            }
            set
            {
                Entity.SetField("fontscale", value);
            }
        }

        public string Font
        {
            get
            {
                return Entity.GetField<string>("font");
            }
            set
            {
                Entity.SetField("font", value);
            }
        }

        public string AlignX
        {
            get
            {
                if (Entity == null)
                {
                    return "left";
                }

                return Entity.GetField<string>("alignx");
            }
            set
            {
                Entity.SetField("alignx", value);
            }
        }

        public string AlignY
        {
            get
            {
                if (Entity == null)
                {
                    return "top";
                }

                return Entity.GetField<string>("aligny");
            }
            set
            {
                Entity.SetField("aligny", value);
            }
        }

        public string HorzAlign
        {
            get
            {
                if (Entity == null)
                {
                    return "left";
                }

                return Entity.GetField<string>("horzalign");
            }
            set
            {
                Entity.SetField("horzalign", value);
            }
        }

        public string VertAlign
        {
            get
            {
                if (Entity == null)
                {
                    return "top";
                }

                return Entity.GetField<string>("vertalign");
            }
            set
            {
                Entity.SetField("vertalign", value);
            }
        }

        public float Alpha
        {
            get
            {
                return Entity.GetField<float>("alpha");
            }
            set
            {
                Entity.SetField("alpha", value);
            }
        }

        public float GlowAlpha
        {
            get
            {
                return Entity.GetField<float>("glowalpha");
            }
            set
            {
                Entity.SetField("glowalpha", value);
            }
        }

        public int Sort
        {
            get
            {
                return Entity.GetField<int>("sort");
            }
            set
            {
                Entity.SetField("sort", value);
            }
        }

        public bool HideWhenInMenu
        {
            get
            {
                return Entity.GetField<int>("hidewheninmenu") != 0;
            }
            set
            {
                Entity.SetField("hidewheninmenu", value);
            }
        }

        public bool Archived
        {
            get
            {
                return Entity.GetField<int>("archived") != 0;
            }
            set
            {
                Entity.SetField("archived", value);
            }
        }

        public bool Foreground
        {
            get
            {
                return Entity.GetField<int>("foreground") != 0;
            }
            set
            {
                Entity.SetField("foreground", value);
            }
        }

        public void SetText(string text)
        {
            //Entity.Call("clearalltextafterhudelem"); // frees configstrings - might cause all configstrings to resync, needs testing in a real-world scenario
            Entity.Call("settext", text);
        }

        public void SetShader(string shader, int w, int h)
        {
            Entity.Call("setshader", shader, w, h);
        }

        public Vector3 Color
        {
            get
            {
                return GetField<Vector3>("color");
            }
            set
            {
                SetField("color", value);
            }
        }

        public Vector3 GlowColor
        {
            get
            {
                return GetField<Vector3>("glowcolor");
            }
            set
            {
                SetField("glowcolor", value);
            }
        }

        #region calls
        public void Call(string func, params Parameter[] parameters)
        {
            Function.SetEntRef(Entity.EntRef);
            Function.Call(func, parameters);
        }

        /*public void Call(int identifier, params Parameter[] parameters)
        {
            Function.SetEntRef(Entity.EntRef);
            Function.Call(identifier, parameters);
        }*/

        public TReturn Call<TReturn>(string func, params Parameter[] parameters)
        {
            Function.SetEntRef(Entity.EntRef);
            return Function.Call<TReturn>(func, parameters);
        }

        /*public TReturn Call<TReturn>(int identifier, params Parameter[] parameters)
        {
            Function.SetEntRef(Entity.EntRef);
            return Function.Call<TReturn>(identifier, parameters);
        }*/

        public T GetField<T>(string name)
        {
            return Entity.GetField<T>(name);
        }

        public void SetField(string name, Parameter value)
        {
            Entity.SetField(name, value);
        }
        #endregion

        public static HudElem CreateFontString(Entity client, string font, float fontScale)
        {
            var elem = NewClientHudElem(client);
            elem.Font = font;
            elem.FontScale = fontScale;
            elem.X = 0;
            elem.Y = 0;
            elem.Height = (int)(12 * fontScale);
            elem.Parent = UIParent;
            return elem;
        }

        public static HudElem CreateServerFontString(string font, float fontScale)
        {
            var elem = NewHudElem();
            elem.Font = font;
            elem.FontScale = fontScale;
            elem.X = 0;
            elem.Y = 0;
            elem.Height = (int)(12 * fontScale);
            elem.Parent = UIParent;
            return elem;
        }

        public static HudElem CreateIcon(Entity client, string shader, int width, int height)
        {
            var elem = NewClientHudElem(client);
            elem.X = 0;
            elem.Y = 0;
            elem.Width = width;
            elem.Height = height;
            elem.Parent = UIParent;

            if (shader != null)
            {
                elem.SetShader(shader, width, height);
            }

            return elem;
        }

        public static HudElem CreateServerIcon(string shader, int width, int height, string team = null)
        {
            HudElem elem;
            if (team != null)
            {
                elem = NewTeamHudElem(team);
            }
            else
            {
                elem = NewHudElem();
            }
            elem.X = 0;
            elem.Y = 0;
            elem.Width = width;
            elem.Height = height;
            elem.Parent = UIParent;

            if (shader != null)
            {
                elem.SetShader(shader, width, height);
            }

            return elem;
        }

        public static HudElem NewClientHudElem(Entity client)
        {
            Function.SetEntRef(-1);
            return new HudElem(Function.Call<Entity>("newclienthudelem", client));
        }

        public static HudElem NewTeamHudElem(string team)
        {
            Function.SetEntRef(-1);
            return new HudElem(Function.Call<Entity>("newteamhudelem", team));
        }

        public static HudElem NewHudElem()
        {
            Function.SetEntRef(-1);
            return new HudElem(Function.Call<Entity>("newhudelem"));
        }

        #region utility functions
        #region parenting
        private HudElem _parent;
        public List<HudElem> Children { get; set; }

        public HudElem Parent
        {
            get
            {
                return _parent;
            }
            set
            {
                if (_parent != null)
                {
                    _parent.Children.Remove(this);
                }

                _parent = value;
                _parent.Children.Add(this);


            }
        }
        #endregion

        #region positioning
        public void SetPoint(string point)
        {
            SetPoint(point, point, 0, 0, 0);
        }

        public void SetPoint(string point, string relativePoint)
        {
            SetPoint(point, relativePoint, 0, 0, 0);
        }

        public void SetPoint(string point, string relativePoint, int xOffset)
        {
            SetPoint(point, relativePoint, xOffset, 0, 0);
        }

        public void SetPoint(string point, string relativePoint, int xOffset, int yOffset)
        {
            SetPoint(point, relativePoint, xOffset, yOffset, 0);
        }

        public void SetPoint(string point, string relativePoint, int xOffset, int yOffset, float moveTime)
        {
            var element = Parent;

            point = point.ToLowerInvariant();
            relativePoint = relativePoint.ToLowerInvariant();

            if (moveTime > 0)
            {
                Call("moveovertime", moveTime);
            }

            _xOffset = xOffset;
            _yOffset = yOffset;
            _point = point;

            AlignX = "center";
            AlignY = "middle";

            if (point.Contains("top"))
            {
                AlignY = "top";
            }

            if (point.Contains("bottom"))
            {
                AlignY = "bottom";
            }

            if (point.Contains("left"))
            {
                AlignX = "left";
            }

            if (point.Contains("right"))
            {
                AlignX = "right";
            }

            var relativeX = "center_adjustable";
            var relativeY = "middle";

            if (relativePoint.Contains("top"))
            {
                relativeY = "top_adjustable";
            }

            if (relativePoint.Contains("bottom"))
            {
                relativeY = "bottom_adjustable";
            }

            if (relativePoint.Contains("left"))
            {
                relativeX = "left_adjustable";
            }

            if (relativePoint.Contains("right"))
            {
                relativeX = "right_adjustable";
            }

            if (element.Entity == null)
            {
                HorzAlign = relativeX;
                VertAlign = relativeY;
            }
            else
            {
                HorzAlign = element.HorzAlign;
                VertAlign = element.VertAlign;
            }

            var xFactor = 0;
            var yFactor = 0;
            var offsetX = 0;
            var offsetY = 0;

            if (relativeX.Split('_')[0] == element.AlignX)
            {
                offsetX = 0;
                xFactor = 0;
            }
            else if (relativeX == "center" || element.AlignX == "center")
            {
                offsetX = (element.Width == 0) ? 0 : (element.Width / 2);

                if (relativeX == "left_adjustable" || element.AlignX == "right")
                {
                    xFactor = -1;
                }
                else
                {
                    xFactor = 1;
                }
            }
            else
            {
                offsetX = element.Width;

                if (relativeX == "left_adjustable")
                {
                    xFactor = -1;
                }
                else
                {
                    xFactor = 1;
                }
            }

            X = element.X + (offsetX * xFactor);

            if (relativeY.Split('_')[0] == element.AlignY)
            {
                offsetY = 0;
                yFactor = 0;
            }
            else if (relativeY == "middle" || element.AlignY == "middle")
            {
                offsetY = (element.Height == 0) ? 0 : (element.Height / 2);

                if (relativeY == "top_adjustable" || element.AlignY == "bottom")
                {
                    yFactor = -1;
                }
                else
                {
                    yFactor = 1;
                }
            }
            else
            {
                offsetY = element.Height;

                if (relativeY == "top_adjustable")
                {
                    yFactor = -1;
                }
                else
                {
                    yFactor = 1;
                }
            }

            Y = element.Y + (offsetY * yFactor);

            X += _xOffset;
            Y += _yOffset;

            // setpointbar stuff
            
            UpdateChildren();
        }

        public void ChangeFontScaleOverTime(float time, float endScale)
        {
            Call("changefontscaleovertime", time);
            FontScale = endScale;
        }
        private void UpdateChildren()
        {
            foreach (var child in Children)
            {
                child.SetPoint(child._point, child._relativePoint, child._xOffset, child._yOffset);
            }
        }

        public int Width
        {
            get;
            set;
        }

        public int Height
        {
            get;
            set;
        }

        private int _xOffset;
        private int _yOffset;
        private string _point;
        private string _relativePoint;
        #endregion
        #endregion
    }
}
