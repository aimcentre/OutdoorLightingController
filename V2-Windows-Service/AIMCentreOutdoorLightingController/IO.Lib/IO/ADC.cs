
using IO.Lib.Exceptions;
using Microsoft.Extensions.Logging;
using System;
using System.IO.Ports;
using System.Net.Security;
using System.Security.Cryptography.Xml;

namespace IO.Lib.IO
{
    public class ADC : SerialPortDevice
    {
        public int IN0 { get; private set; }
        public int IN1 { get; private set; }
        public int IN2 { get; private set; }
        public int IN3 { get; private set; }
        public int IN4 { get; private set; }
        public int IN5 { get; private set; }
        public int IN6 { get; private set; }
        public int IN7 { get; private set; }
        public int IN8 { get; private set; }
        public int IN9 { get; private set; }

        public int CHECK_PIN => IN8;
        public int NIGHT_PIN => IN9;

        //public static readonly int SENSOR1 = 1;
        //public static readonly int SENSOR2 = 3;
        //public static readonly int SENSOR3 = 5;
        //public static readonly int SENSOR4 = 7;


        private string? _lastLine;
        public bool DataSetReceived { get; protected set; }

        public ADC(string portName)
            : base(portName, 115200)
        {
            _comPort.DataReceived += new SerialDataReceivedEventHandler(port_DataReceived);
            DataSetReceived = false;
        }

        public override bool TryOpen()
        {
            _lastLine = null;
            _comPort.DtrEnable = true;
            return base.TryOpen();
        }

        private void port_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            if (!_comPort.IsOpen)
                throw new LightingControllerException("Port is not opened");

            int n = _comPort.BytesToRead;

            if (n > 0)
                DataSetReceived = true;

            byte[] bytes = new byte[n];
            _comPort.Read(bytes, 0, n);
            string data = System.Text.Encoding.UTF8.GetString(bytes);
            string[] lines = data.Split(Environment.NewLine);
            for(int i=0; i<lines.Length; i++)
            {
                string line = lines[i];
                if (_lastLine != null)
                    line = _lastLine + line;

                if (!line.StartsWith("CH"))
                    continue; //Not a valid ADC channel value

                if (i == lines.Length - 1 && !line.EndsWith("V"))
                {
                    //A full line has not yet been read
                    _lastLine = line;
                    continue;
                }
                else
                    _lastLine = null;

                if (int.TryParse(line.Substring(2,1), out int channelNumber))
                {
                    if(!int.TryParse(line.Substring(4,4), out int channelValue))
                        channelValue = 0;

                    Console.WriteLine($"{line} => CH{channelNumber}:{channelValue}");
                    if (channelNumber == 9)
                        Console.WriteLine();

                    switch (channelNumber)
                    {
                        case 0: IN0 = channelValue; break;
                        case 1: IN1 = channelValue; break;
                        case 2: IN2 = channelValue; break;
                        case 3: IN3 = channelValue; break;
                        case 4: IN4 = channelValue; break;
                        case 5: IN5 = channelValue; break;
                        case 6: IN6 = channelValue; break;
                        case 7: IN7 = channelValue; break;
                        case 8: IN8 = channelValue; break;
                        case 9: IN9 = channelValue; break;
                    }
                }
            }
        }
    }
}













































































