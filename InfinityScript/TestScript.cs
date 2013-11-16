using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;

namespace InfinityScript
{
    public class TestScript : BaseScript
    {
        private bool[] rainbow_enabled = new bool[18];
        private string rainbow = "12356";
        public override EventEat OnSay3(Entity player, ChatType type, string name, ref string message)
        {
            if (type == ChatType.All) // only change global messages, not team chat.
            {
                var changed = "";
                var rnd = new Random();
                if (rainbow_enabled[player.EntRef])
                {
                    message = Regex.Replace(message, @"\^\d", ""); // strip color tags
                    for (var i = 0; i < message.Length; i++)
                    {
                        changed += "^" + rainbow[rnd.Next(rainbow.Length)] + message[i];
                    }
                    message = changed;
                }
            }
            
            return EventEat.EatNone;
        }
        public TestScript() : base()
        {
            //return;

            Log.Debug("hie");

            OnNotify("connected", player =>
            {
                Log.Write(LogLevel.Trace, "connected {0}", player);

                player.As<Entity>().Notified += (type, paras) =>
                {
                    Log.Write(LogLevel.Trace, "tyep {0}", type);
                };

                player.As<Entity>().OnNotify("spawned_player", playerEnt =>
                {
                    Log.Write(LogLevel.Trace, "spawned {0}", playerEnt);

                    Call("iprintlnbold", "0mgSniPeZZ by xXxNTAxXx");

                    playerEnt.Call("takeallweapons");
                    playerEnt.Call("giveweapon", "cheytac_mp");
                    playerEnt.Call("setperk", "specialty_quickdraw", true);

                    Log.Write(LogLevel.Trace, "endSpawned {0}", playerEnt);
                });
            });

            return;

            PlayerConnected += entity => rainbow_enabled[entity.EntRef] = false;
            OnClientCommand("cool_rainbow", entity => rainbow_enabled[entity.EntRef] = !rainbow_enabled[entity.EntRef]);
           
            OnServerCommand("doabarrelroll", args =>
            {
                var j = 0;
                for (int i = 0; i < 2048; i++)
                {
                    var entity = Entity.GetEntity(i);
                    var targetname = entity.GetField<string>("targetname");
                    if (targetname == "explodable_barrel")
                    {
                        entity.AfterDelay(500 * j++, e =>
                        {
                            var oldHealth = e.Health;
                            if (args.Length == 2 && args[1] == "explode")
                            {
                                e.Health -= 150;
                            }
                            else
                            {
                                e.Health -= 1;
                            }
                            e.Notify("damage", (oldHealth - e.Health), e, new Vector3(0, 0, 0), new Vector3(0, 0, 0), "MOD_EXPLOSIVE", "", "", "", 0, "frag_grenade_mp");
                        });
                    }
                }
                return true;
            });

            /*OnClientCommand("whoami", entity =>
            {
                var name = entity.Name;
                var userid = entity.UserID;
                var ip = entity.IP;

                entity.Call("iprintlnbold", string.Format("Obviously you are {0}, connecting from {1}, with an ID of {2}.", name, ip, userid));
            });*/
            /*OnServerCommand("test_tokenize", args =>
            {
                if (args.Length == 2)
                {
                    try
                    {
                        var thing = Utilities.Tokenize(args[1]);
                        foreach (var token in thing)
                        {
                            Log.Info(token);
                        }
                    }
                    catch (ArgumentException e)
                    {
                        Log.Info(e.ToString());
                    }
                }
                return true;
            });*/
            /*OnNotify("weapon_fired", weaponName =>
            {
                GameInterface.Script_PushString((string)weaponName);
                GameInterface.Script_Call(362, 0, 1);
            });*/

            /*OnNotify("connected", player =>
            {
                Log.Write(LogLevel.Trace, "connected {0}", player);
                
                player.As<Entity>().Notified += (type, paras) =>
                {
                    Log.Write(LogLevel.Trace, "tyep {0}", type);
                };

                player.As<Entity>().OnNotify("spawned_player", playerEnt =>
                {
                    Log.Write(LogLevel.Trace, "spawned {0}", playerEnt);

                    Call("iprintlnbold", "0mgSniPeZZ by xXxNTAxXx");

                    playerEnt.Call("takeallweapons");
                    playerEnt.Call("giveweapon", "iw5_l96a1_mp_l96a1scope");
                    playerEnt.Call("setperk", "specialty_quickdraw");

                    Log.Write(LogLevel.Trace, "endSpawned {0}", playerEnt);
                });
            });*/

            /*PlayerConnected += new Action<Entity>(entity =>
            {
                Log.Write(LogLevel.Trace, "connected {0}", entity);

                entity.SpawnedPlayer += new Action(() =>
                {
                    Log.Write(LogLevel.Trace, "spawned {0} {1} {2} {3}", entity, entity.GetField<string>("classname"), entity.GetField<string>("sessionteam"), entity.GetField<Vector3>("origin"));

                    entity.TakeAllWeapons();
                    //entity.GiveWeapon("iw5_l96a1_mp_l96a1scope");
                    //entity.SwitchToWeaponImmediate("iw5_l96a1_mp_l96a1scope");
                    //entity.Call("givemaxammo", "iw5_l96a1_mp_l96a1scope");
                    entity.GiveWeapon("iw5_mp7_mp");
                    entity.SwitchToWeaponImmediate("iw5_mp7_mp");
                    entity.Call("givemaxammo", "iw5_mp7_mp");

                    entity.Call("iprintlnbold", "0mgMP7 by xXxNTAxXx");
                    entity.Call("setperk", "specialty_quickdraw", true, false);
                    entity.Call("setperk", "specialty_fastreload", true, false);
                });

                //var elem = HudElem.CreateFontString(entity, "default", 1.2f);
                //elem.X = 200;
                //elem.Y = 20;
                //elem.SetText("stuff. just ^2stuff^7.");
            });*/

            /*PlayerConnected += new Action<Entity>(entity =>
            {
                entity.Call("notifyOnPlayerCommand", "ohnoes", "+actionslot 4");

                entity.OnNotify("ohnoes", player =>
                {
                    var testClient = Utilities.AddTestClient();

                    if (testClient == null)
                    {
                        return;
                    }

                    testClient.OnNotify("joined_spectators", tc =>
                    {
                        tc.Notify("menuresponse", "team_marinesopfor", "autoassign");

                        tc.AfterDelay(500, meh =>
                        {
                            meh.Notify("menuresponse", "changeclass", "class1");
                        });
                    });
                });
            });*/

            //Notified += TestScript_Notified;
           // Tick += new Action(TestScript_Tick);
  
        }

        /*  int _lastTime;

          void TestScript_Tick()
          {
              var time = Function.Call<int>(283);

              if ((time - _lastTime) > 10)
              {
                  Function.Call(363, time);
                  _lastTime = time;
              }
          }
        */
        /*void TestScript_Notified(string arg1, Parameter[] arg2)
        {
            if (arg1 == "trigger" || arg1 == "weapon_fired" || arg1 == "touch")
            {
                return;
            }

            GameInterface.Script_PushString(arg1);
            GameInterface.Script_Call(363, 0, 1);
        }*/
    }
}
