
using Microsoft.Extensions.Logging;
using System.Globalization;
using System.IO.Ports;

namespace IO.Lib.IO
{
    public class UsbPortHandler
    {
        protected readonly string _portName;
        public readonly SerialPort _comPort;

        public static string[] ListPorts() => SerialPort.GetPortNames();

        public UsbPortHandler(string portName, int boadRate, int dataBits)
        {
            _portName = portName;
            //_comPort = new SerialPort(portName, boadRate, Parity.Odd, dataBits, StopBits.One);
            _comPort = new SerialPort(portName, 9600);
            Console.WriteLine("relay");
        }

        public UsbPortHandler(string portName, int boadRate)
        {
            _portName = portName;
            _comPort = new SerialPort(portName, boadRate);
        }

        public bool IsConnected => ListPorts().Contains(_portName);
        public bool IsOpened => _comPort.IsOpen;

        public virtual bool TryOpen()
        {
            try
            {
                _comPort.Open();
                return true;
            }
            catch (Exception)
            {
                return false;
            }
        }

        public void Close()
        {
            try
            {
                _comPort.DtrEnable = false;
                _comPort.Close();
            }
            catch (Exception)
            {

            }
        }

        public bool CheckIOModule(string moduleName)
        {
            if (!IsConnected)
            {
                Console.WriteLine($"{moduleName} Not Connected");
                return false;
            }

            if (!IsOpened)
            {
                if (!TryOpen())
                    Console.WriteLine($"Canot Open {moduleName} Port");
            }

            if (!IsOpened)
            {
                Console.WriteLine($"{moduleName} Port Not Opened");
                return false;
            }

            return true;
        }

        public void Write(string hexString)
        {
            Console.WriteLine(hexString);

            byte[] bytes = Hex2Binary(hexString);
            _comPort.Write(bytes, 0, bytes.Length);
        }

        private byte[] Hex2Binary(string hex)
        {
            var chars = hex.ToCharArray();
            var bytes = new List<byte>();
            for (int index = 0; index < chars.Length; index += 2)
            {
                var chunk = new string(chars, index, 2);
                bytes.Add(byte.Parse(chunk, NumberStyles.AllowHexSpecifier));
            }
            return bytes.ToArray();
        }

    }
}
