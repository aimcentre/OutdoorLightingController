
using Microsoft.Extensions.Logging;
using System.IO.Ports;

namespace IO.Lib.IO
{
    public class UsbPortHandler
    {
        public static string[] ListPorts() => SerialPort.GetPortNames();

    }
}
