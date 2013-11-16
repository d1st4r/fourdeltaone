using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using SQLite;

namespace InfinityScript
{
    public class PlayerCountStatisticPoint
    {
        [PrimaryKey]
        public DateTime Time { get; set; }

        public int PlayerCount { get; set; }
    }
}
