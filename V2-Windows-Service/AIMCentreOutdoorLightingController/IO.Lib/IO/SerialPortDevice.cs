
using Microsoft.Extensions.Logging;
using System.Globalization;
using System.IO.Ports;

namespace IO.Lib.IO
{
    /// <summary>
    /// This class represents a generic USB serial port device.
    /// </summary>
    public class SerialPortDevice
    {
        protected readonly string _portName;
        private readonly int _interCommandDelayMilliseconds;
        public readonly SerialPort _comPort;

        /// <summary>
        /// Creates a serial port instance and configures it with the following parameters.
        /// </summary>
        /// <param name="portName">Name of the serial port.</param>
        /// <param name="boadRate">Speed of the serial port.</param>
        /// <param name="dataBits">Number of data bits in a data packet.</param>
        /// <param name="interCommandDelayMilliseconds">Number of milliseconds wait after writing each data packet.</param>
        public SerialPortDevice(string portName, int boadRate, int dataBits, int interCommandDelayMilliseconds)
        {
            _portName = portName;
            _comPort = new SerialPort(portName, boadRate, Parity.None, dataBits, StopBits.One);
            _interCommandDelayMilliseconds = interCommandDelayMilliseconds;
        }

        /// <summary>
        /// Creates a serial port instance and configures it with the following parameters.
        /// </summary>
        /// <param name="portName">Name of the serial port.</param>
        /// <param name="boadRate">Speed of the serial port.</param>
        public SerialPortDevice(string portName, int boadRate)
        {
            _portName = portName;
            _comPort = new SerialPort(portName, boadRate);
        }

        public void SetPortName(string portName)
        {
            _comPort.PortName = portName;
        }

        /// <summary>
        /// Returns an array of serial port names of this computer. 
        /// </summary>
        /// <returns></returns>
        public static string[] GetSerialPortNames() => SerialPort.GetPortNames();

        /// <summary>
        /// Returns true if the port name assigned for this SerialPortDevice is 
        /// listed as a port in the computer.
        /// </summary>
        public bool IsConnected => GetSerialPortNames().Contains(_portName);

        /// <summary>
        /// Returns true if the serial port with the name assigned for this SerialPortDevice
        /// is opened.
        /// </summary>
        public bool IsOpened => _comPort.IsOpen;

        /// <summary>
        /// Try to open the port assigned for this SerialPortDevice. 
        /// </summary>
        /// <returns>Returns true if the port was successfully opened, or false otherwise.</returns>
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

        /// <summary>
        /// Closes the serial port assigned to SerialPortDevice.
        /// </summary>
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

        /// <summary>
        /// Check whether the port assigned for SerialPortDevice is opened. 
        /// If it's not opened then attempt to open it. If the port is opened or
        /// the attempt to open is successfuly, return true. Otherwise, return false.
        /// </summary>
        /// <param name="moduleName"></param>
        /// <returns></returns>
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

        /// <summary>
        /// Write the input string value to the port assigned to SerialPortDevice.
        /// </summary>
        /// <param name="hexString">The value to be written.</param>
        /// <returns></returns>
        public async Task Write(string hexString)
        {
            Console.WriteLine(hexString);

            byte[] bytes = Hex2Binary(hexString);
            _comPort.Write(bytes, 0, bytes.Length);

            await Task.Delay(_interCommandDelayMilliseconds);
        }

        /// <summary>
        /// This is a private function that converts an input string to a byte array.
        /// This conversion is needed before writing a value to a serial port.
        /// </summary>
        /// <param name="hex"></param>
        /// <returns></returns>
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
