
using IO.Lib.Exceptions;
using Microsoft.Extensions.Logging;
using System;
using System.IO.Ports;
using System.Security.Cryptography.Xml;

namespace IO.Lib.IO
{
    public class SensorInput : UsbPortHandler
    {
        private readonly SerialPort _comPort;
        public SensorInput(string port)
        {
            _comPort = new SerialPort(port, 115200);
            _comPort.DtrEnable = true;
            //_comPort.DataReceived += new SerialDataReceivedEventHandler(port_DataReceived);
            _comPort.Open();
        }

        public string ReadLine()
        {
            return _comPort.ReadExisting();
        }

        private void port_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            if (!_comPort.IsOpen)
                throw new LightingControllerException("Port is not opened");

            int bytesReceived = _comPort.BytesToRead;
            bytesReceived = bytesReceived / 2 * 2; // Make sure we read an even number of bytes

            if (bytesReceived >= 20)
            {
                int N = 20;
                byte[] bytes = new byte[N];

                _comPort.Read(bytes, 0, N);

                for (int i = 0; i < N; i++)
                {
                    int val = 0xFF & bytes[i];
                    //int val1 = bytes[2*i] | bytes[2*i+1] << 8;
                    //int val2 = bytes[2 * i] << 8 | bytes[2 * i + 1];
                    Console.Write($"{val} ");
                }
                Console.WriteLine();
                Console.WriteLine($"20 of {bytesReceived}");
                Console.WriteLine();
            }

            //if (BitConverter.IsLittleEndian)
            //    Array.Reverse(bytes);

            //int i = BitConverter.ToInt32(bytes, 0);
            //Console.WriteLine("int: {0}", i);

            //var str = System.Text.Encoding.Default.GetString(bytes);


        }
    }
}













































































