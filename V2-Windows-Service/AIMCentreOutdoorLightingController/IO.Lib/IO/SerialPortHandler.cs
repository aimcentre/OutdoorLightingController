using System.IO.Ports;
using System.Management;

namespace IO.Lib.IO
{
    public static class SerialPortHandler
    {
        public static SerialPortDeviceInfo[] GetDeviceInfo()
        {
            //var portNames = SerialPort.GetPortNames();
            //foreach (var portName in portNames)
            //{
            //    Console.WriteLine(portName);
            //}

            SerialPortDeviceInfo[] deviceInfos = null!;
            using (var searcher = new ManagementObjectSearcher("SELECT * FROM Win32_PnPEntity WHERE Caption like '%(COM%'"))
            {
                var portnames = SerialPort.GetPortNames();
                var x = searcher.Get().Cast<ManagementBaseObject>().ToList();
                var ports = searcher.Get().Cast<ManagementBaseObject>().ToList().Select(p => p["Caption"].ToString());

                deviceInfos = portnames.Select(n => new SerialPortDeviceInfo()
                {
                    SerialPortName = n,
                    SerialPortDescription = ports.FirstOrDefault(s => s.Contains(n))!
                }).ToArray();
            }

            return deviceInfos;
        }
    }
}
