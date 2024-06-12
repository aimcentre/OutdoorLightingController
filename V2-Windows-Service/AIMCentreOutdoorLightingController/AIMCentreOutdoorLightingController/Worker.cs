using IO.Lib.Exceptions;
using IO.Lib.IO;
using System.Globalization;
using System.IO.Ports;
using System.Linq;
using System.Text;

namespace AIMCentreOutdoorLightingController
{
    public class Worker : BackgroundService
    {
        private readonly ILogger<Worker> _logger;
        protected ADC _adc;
        protected Replay _relay;

        public Worker(ILogger<Worker> logger)
        {
            _logger = logger;
            _adc = new ADC("COM4");
            _relay = new Replay("COM4");
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            bool adc = !true;
            try
            {
                

                var rates = new int[] { 50, 75, 100, 110, 150, 300, 600, 900, 1200, 1800, 2400, 3600, 4800, 9600, 14400, 19200, 33600, 38400, 56000, 57600, 76800, 115200 };

                SerialPort port = _relay._comPort;
                await Task.Delay(2000, stoppingToken);

                int delay = 1000;

                while (!stoppingToken.IsCancellationRequested)
                {
                    if (adc)
                    {
                        _adc.CheckIOModule("ADC");
                    }
                    else
                    {
                        if (_relay.CheckIOModule("Relay"))
                        {
                            await _relay.TurnAll(true);
                            await Task.Delay(delay, stoppingToken);
                            await _relay.TurnAll(false);
                        }

                        //if (_relay.CheckIOModule("Relay"))
                        //{
                        //    _relay.Toggle(i);
                        //}
                        //i = (++i) % 16;
                    }

                    await Task.Delay(delay, stoppingToken);
                }

                if (_adc.IsOpened)
                {
                    Console.WriteLine("Closing ADC Port");
                    _adc.Close();
                }

            }
            catch(LightingControllerException ex)
            {
                _logger.LogError(ex.Message);
            }
        }

    }
}
