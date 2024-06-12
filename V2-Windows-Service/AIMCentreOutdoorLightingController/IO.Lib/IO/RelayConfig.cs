using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IO.Lib.IO
{
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
