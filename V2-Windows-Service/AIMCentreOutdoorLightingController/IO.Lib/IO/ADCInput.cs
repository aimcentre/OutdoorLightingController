﻿
using IO.Lib.Exceptions;
using Microsoft.Extensions.Logging;
using System;
using System.IO.Ports;
using System.Security.Cryptography.Xml;

namespace IO.Lib.IO
{
    public class ADCInput : UsbPortHandler
    {
        private readonly string _portName;
        private readonly SerialPort _comPort;
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

        private string? _lastLine;
        public ADCInput(string portName)
        {
            _portName = portName;
            _comPort = new SerialPort(portName, 115200);
            _comPort.DataReceived += new SerialDataReceivedEventHandler(port_DataReceived);
        }

        public bool IsConnected => ListPorts().Contains(_portName);
        public bool IsOpened => _comPort.IsOpen;

        public bool TryOpen()
        {
            try
            {
                _comPort.DtrEnable = true;
                _comPort.Open();
                _lastLine = null;
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
            catch(Exception)
            {

            }
        }

        private void port_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            if (!_comPort.IsOpen)
                throw new LightingControllerException("Port is not opened");

            int n = _comPort.BytesToRead;
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













































































