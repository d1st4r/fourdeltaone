using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Runtime.InteropServices;

namespace InfinityScript
{
    public static class Utilities
    {
        public static string GetWeaponClass(string weapon)
        {
            Function.SetEntRef(-1);

            var tokens = weapon.Split('_');
            var weaponClass = "";

            if (tokens[0] == "iw5")
            {
                var concatName = tokens[0] + "_" + tokens[1];
                weaponClass = Function.Call<string>("tableLookup", "mp/statstable.csv", 4, concatName, 2);
            }
            else if (tokens[0] == "alt")
            {
                var concatName = tokens[1] + "_" + tokens[2];
                weaponClass = Function.Call<string>("tableLookup", "mp/statstable.csv", 4, concatName, 2);
            }
            else
            {
                weaponClass = Function.Call<string>("tableLookup", "mp/statstable.csv", 4, tokens[0], 2);
            }

            if (weaponClass == "")
            {
                weapon = Regex.Replace(weapon, "_mp$", "");

                weaponClass = Function.Call<string>("tableLookup", "mp/statstable.csv", 4, weapon, 2);
            }

            if (weapon == "none" || weaponClass == "")
            {
                weaponClass = "other";
            }

            return weaponClass;
        }

        public static string GetAttachmentType(string attachmentName)
        {
            Function.SetEntRef(-1);
            return Function.Call<string>("tableLookup", "mp/attachmenttable.csv", 4, attachmentName, 2);
        }

        public static string AttachmentMap(string attachmentName, string weaponName)
        {
            Function.SetEntRef(-1);

            var weaponClass = Function.Call<string>("tableLookup", "mp/statstable.csv", 4, weaponName, 2);

            switch (weaponClass)
            {
                case "weapon_smg":
                    if (attachmentName == "reflex")
                        return "reflexsmg";
                    else if (attachmentName == "eotech")
                        return "eotechsmg";
                    else if (attachmentName == "acog")
                        return "acogsmg";
                    else if (attachmentName == "thermal")
                        return "thermalsmg";

                    return attachmentName;
                case "weapon_lmg":
                    if (attachmentName == "reflex")
                        return "reflexlmg";
                    else if (attachmentName == "eotech")
                        return "eotechlmg";

                    return attachmentName;
                case "weapon_machine_pistol":
                    if (attachmentName == "reflex")
                        return "reflexsmg";
                    else if (attachmentName == "eotech")
                        return "eotechsmg";

                    return attachmentName;
                default:
                    return attachmentName;
            }
        }

        public static string BuildWeaponName(string baseName, string attachment1, string attachment2, int camo, int reticle)
        {
            Function.SetEntRef(-1);

            if (attachment1 == null)
            {
                attachment1 = "none";
            }

            if (attachment2 == null)
            {
                attachment2 = "none";
            }

            if (reticle > 0 && GetAttachmentType(attachment1) != "rail" && GetAttachmentType(attachment2) != "rail")
            {
                reticle = 0;
            }

            if (GetAttachmentType(attachment1) == "rail")
            {
                attachment1 = AttachmentMap(attachment1, baseName);
            }
            else if (GetAttachmentType(attachment2) == "rail")
            {
                attachment2 = AttachmentMap(attachment2, baseName);
            }

            var bareWeaponName = "";
            var weaponName = "";

            if (baseName.Contains("iw5_"))
            {
                weaponName = baseName + "_mp";
                bareWeaponName = baseName.Substring(4);
            }
            else
            {
                weaponName = baseName;
            }

            string[] attachments = new string[3];

            if (attachment1 != "none" && attachment2 != "none")
            {
                if (attachment1.CompareTo(attachment2) <= 0)
                {
                    attachments[0] = attachment1;
                    attachments[1] = attachment2;
                }
                else
                {
                    attachments[0] = attachment2;
                    attachments[1] = attachment1;
                }

                if (GetWeaponClass(baseName) == "weapon_sniper" && GetAttachmentType(attachment1) != "rail" && GetAttachmentType(attachment2) != "rail")
                {
                    if (attachment1 != "zoomscope" && attachment2 != "zoomscope")
                    {
                        attachments[2] = bareWeaponName + "scope";
                    }
                }
            }
            else if (attachment1 != "none")
            {
                attachments[0] = attachment1;

                if (GetWeaponClass(baseName) == "weapon_sniper" && GetAttachmentType(attachment1) != "rail" && attachment1 != "zoomscope")
                {
                    attachments[1] = bareWeaponName + "scope";
                }
            }
            else if (attachment2 != "none")
            {
                attachments[0] = attachment2;

                if (GetWeaponClass(baseName) == "weapon_sniper" && GetAttachmentType(attachment2) != "rail" && attachment2 != "zoomscope")
                {
                    attachments[1] = bareWeaponName + "scope";
                }
            }
            else if (GetWeaponClass(baseName) == "weapon_sniper")
            {
                attachments[0] = bareWeaponName + "scope";
            }

            if (attachments[0] != null && attachments[0] == "vzscope")
            {
                attachments[0] = bareWeaponName + "scopevz";
            }
            else if (attachments[1] != null && attachments[1] == "vzscope")
            {
                attachments[1] = bareWeaponName + "scopevz";
            }
            else if (attachments[2] != null && attachments[2] == "vzscope")
            {
                attachments[2] = bareWeaponName + "scopevz";
            }

            attachments = attachments.OrderBy(attachment => attachment ?? "zz").ToArray();

            foreach (var attachment in attachments)
            {
                if (string.IsNullOrEmpty(attachment))
                {
                    continue;
                }

                weaponName += "_" + attachment;
            }

            var weaponClass = GetWeaponClass(baseName);

            if (weaponName.Contains("iw5_"))
            {
                if (weaponClass != "weapon_pistol" && weaponClass != "weapon_machine_pistol" && weaponClass != "weapon_projectile")
                {
                    weaponName = BuildWeaponNameCamo(weaponName, camo);
                }

                weaponName = BuildWeaponNameReticle(weaponName, reticle);

                return weaponName;
            }
            else
            {
                if (weaponClass != "weapon_pistol" && weaponClass != "weapon_machine_pistol" && weaponClass != "weapon_projectile")
                {
                    weaponName = BuildWeaponNameCamo(weaponName, camo);
                }

                weaponName = BuildWeaponNameReticle(weaponName, reticle);

                return weaponName + "_mp";
            }
        }

        private static string BuildWeaponNameCamo(string weaponName, int camo)
        {
            if (camo <= 0)
            {
                return weaponName;
            }

            if (camo < 10)
            {
                weaponName += "_camo0";
            }
            else
            {
                weaponName += "_camo";
            }

            weaponName += camo.ToString();

            return weaponName;
        }

        private static string BuildWeaponNameReticle(string weaponName, int reticle)
        {
            if (reticle <= 0)
            {
                return weaponName;
            }

            return weaponName + "_scope" + reticle.ToString();
        }

        public static void SetDropItemEnabled(bool enabled)
        {
            GameInterface.SetDropItemEnabled(enabled);
        }

        public static Entity AddTestClient()
        {
            var entref = GameInterface.AddTestClient();

            if (entref == 0)
            {
                return null;
            }

            return Entity.GetEntity(entref);
        }

        public static void ExecuteCommand(string command)
        {
            GameInterface.Cbuf_AddText(command + "\n");
        }

        public static void ExecuteCommand(string format, params object[] args)
        {
            ExecuteCommand(String.Format(format, args));
        }

        #region say commands
        public static void SayTo(Entity ent, string message)
        {
            SayTo(ent.EntRef, message);
        }
        public static void SayTo(int entref, string message)
        {
            SayTo(entref, "console", message);
        }
        public static void SayTo(Entity ent, string name, string message)
        {
            SayTo(ent.EntRef, name, message);
        }
        public static void SayTo(int entref, string name, string message)
        {
            RawSayTo(entref, name + ": " + message);
        }

        public static void SayAll(string message)
        {
            SayAll("console", message);
        }
        public static void SayAll(string name, string message)
        {
            RawSayAll(name + ": " + message);
        }

        public static void RawSayAll(string message)
        {
            GameInterface.SV_GameSendServerCommand(-1, -1, message);
        }
        public static void RawSayTo(Entity ent, string message)
        {
            RawSayTo(ent.EntRef, message);
        }
        public static void RawSayTo(int entref, string message)
        {
            GameInterface.SV_GameSendServerCommand(entref, -1, message);
        }
        #endregion

        public static string[] Tokenize(string line)
        {
            GameInterface.Cmd_TokenizeString(line);
            var argc = GameInterface.Cmd_Argc();
            if(argc == 0)
            {
                return new string[0];
            }
            var tokenized = new string[argc];
            for (int i = 0; i < argc; i++)
            {
                tokenized[i] = GameInterface.Cmd_Argv(i);
            }
            GameInterface.Cmd_EndTokenizedString();
            return tokenized;
        }
    }
}
