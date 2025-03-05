using IO.Lib.Exceptions;
using IO.Lib.IO;
using Microsoft.AspNetCore.DataProtection.XmlEncryption;
using System.Globalization;
using System.IO.Ports;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;

namespace AIMCentreOutdoorLightingController
{
    public class Worker : BackgroundService
    {
        private readonly ILogger<Worker> _logger;
        protected ADC _adc;
        protected Relay _relay;

        public Worker(ILogger<Worker> logger)
        {
            _logger = logger;
            _adc = null!;
            _relay = null!;
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            try
            {
                bool on = true;

                while (!stoppingToken.IsCancellationRequested)
                {
                    if(_relay == null)
                    {
                        _relay = new Relay("COM7");
                        _relay.TryOpen();
                    }

                    await _relay.TurnAll(on);
                    on = !on;

                    ////if (_adc == null)
                    ////{
                    ////    _adc = new ADC("COM4");
                    ////}

                    ////_adc.CheckIOModule("ADC");

                    //if(_relay == null || !_relay.IsConnected || _adc == null || _adc.IsConnected)
                    //{
                    //    InitDevices();

                    //    ////if (_adc == null || !_adc.IsConnected)
                    //    ////{
                    //    ////    var device = devices.FirstOrDefault(dev => dev.SerialPortDescription.StartsWith("USB-SERIAL CH340"));
                    //    ////    if (device != null)
                    //    ////    {
                    //    ////        if (_relay == null)
                    //    ////            _relay = new Replay(device.SerialPortName);
                    //    ////        else
                    //    ////            _relay.SetPortName(device.SerialPortName);
                    //    ////    }
                    //    ////}

                    //    ////if (_relay == null || !_relay.IsConnected)
                    //    ////{
                    //    ////    var device = devices.FirstOrDefault();
                    //    ////    if (device != null)
                    //    ////    {
                    //    ////        if (_relay == null)
                    //    ////            _relay = new Replay(device.SerialPortName);
                    //    ////        else
                    //    ////            _relay.SetPortName(device.SerialPortName);
                    //    ////    }
                    //    ////}



                    //}


                    ////////if (_adc != null)
                    ////////{
                    ////////    _adc.CheckIOModule("ADC");
                    ////////}

                    ////////if (_relay != null)
                    ////////{
                    ////////    if (_relay.CheckIOModule("Relay"))
                    ////////    {
                    ////////        await _relay.TurnAll(true);
                    ////////        await Task.Delay(1000, stoppingToken);
                    ////////        await _relay.TurnAll(false);
                    ////////    }
                    ////////}

                    await Task.Delay(1000, stoppingToken);

                }

                CloseDevices();

            }
            catch(LightingControllerException ex)
            {
                _logger.LogError(ex.Message);
            }
        }

        protected async Task InitDevices()
        {
            CloseDevices();

            List<SerialPortDeviceInfo> devices = SerialPortHandler.GetDeviceInfo()
              .Where(device => device.SerialPortDescription.StartsWith("USB-SERIAL CH340"))
              .ToList();

            //Cycle through the devices and try to
            for(int i=0; i<devices.Count; i++)
            {
                ADC adc = await DetectSerialDataReaderAsADC(devices, i, 2);

                if (adc != null)
                {
                    if(!adc.CheckIOModule("ADC"))
                    {
                        if(adc.IsOpened)
                            adc.Close();
                        continue;
                    }


                    Relay relay = await DetectRelay(devices, adc, 0);

                    if(relay != null)
                    {
                        _adc = adc;
                        _relay = relay;
                        return;
                    }
                }
            }

        }

        protected void CloseDevices()
        {
            if (_adc != null && _adc.IsOpened)
            {
                Console.WriteLine("Closing ADC Port");
                _adc.Close();
            }

            if (_relay != null && _relay.IsOpened)
            {
                Console.WriteLine("Closing Relay Port");
                _relay.Close();
            }
        }

        protected async Task<ADC> DetectSerialDataReaderAsADC(List<SerialPortDeviceInfo> devices, int offset, int maxWaitTimePerDeviceInSeconds)
        {
            for(int i=offset; i<devices.Count; i++)
            {
                string port = devices[i].SerialPortName;
                ADC adc = new ADC(port);
                if (adc.CheckIOModule("Try As ADC"))
                {
                    for(int j=0; j< 2 * maxWaitTimePerDeviceInSeconds; ++j)
                    {
                        await Task.Delay(500);
                        if (adc.DataSetReceived)
                            return adc;
                    }
                }

                //If we come here, that means that no data was received. So, the current port
                //may not be an ADC. Close the port and proceed to trying the next port.
                if(adc.IsOpened)
                    adc.Close();
            }

            //No serial reader device was found
            return null!;
        }

        protected async Task<Relay> DetectRelay(List<SerialPortDeviceInfo> devices, ADC adc, int offset)
        {
            foreach (SerialPortDeviceInfo device in devices.Where(dev => dev.SerialPortName != adc._comPort.PortName))
            {
                //Configure device as a relay
                Relay relay = new Relay(device.SerialPortName);

                if(!relay.CheckIOModule("Relay"))
                {
                    if(relay.IsOpened)
                        relay.Close();
                    continue;
                }

                for (int i = 0; i < 16; ++i)
                {
                    await relay.Set(i, true);
                    await relay.Set(i, false);
                }

                //Turn on Check pin
                await relay.SetCheckPin(true);

                //Wait for a bit and see if the ADC check pin is set
                await Task.Delay(1000);
                if (adc.CHECK_PIN < 100)
                {
                    //Not the relay
                    relay.Close();
                    continue;
                }

                return relay;
            }

            return null!;
        }
    }
}
