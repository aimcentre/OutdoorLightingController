using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IO.Lib.IO
{
    /// <summary>
    /// This class represents a relay configuration entry, which consists of: 
    /// 1. The command for turning on a relay pin
    /// 2. The command for turning off a relay pin
    /// 3. A boolean status flag that represents whether the relay pin is currently on.
    /// </summary>
    public class RelayConfig
    {
        public string OnCommand { get; private set; }
        public string OffCommand { get; private set; }
        public bool IsOn { get; set; }

        public RelayConfig(string offCommand, string onCommand)
        {
            OffCommand = offCommand;
            OnCommand = onCommand;
            IsOn = false;
        }
    }
}
