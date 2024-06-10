using IO.Lib.Exceptions;
using IO.Lib.IO;
using System.Linq;

namespace AIMCentreOutdoorLightingController
{
    public class Worker : BackgroundService
    {
        private readonly ILogger<Worker> _logger;
        protected ADCInput _adcInput;

        public Worker(ILogger<Worker> logger)
        {
            _logger = logger;
            _adcInput = new ADCInput("COM4");
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            try
            {
                while (!stoppingToken.IsCancellationRequested)
                {
                    CheckAdc();

                    await Task.Delay(1000, stoppingToken);
                }

                if (_adcInput.IsOpened)
                {
                    Console.WriteLine("Closing ADC Port");
                    _adcInput.Close();
                }

            }
            catch(LightingControllerException ex)
            {
                _logger.LogError(ex.Message);

            }
        }

        protected void CheckAdc()
        {
            if (!_adcInput.IsConnected)
            {
                Console.WriteLine("ADC Not Connected");
                return;
            }

            if (!_adcInput.IsOpened)
            {
                if (!_adcInput.TryOpen())
                    Console.WriteLine("Canot Open ADC Port");
            }

            if (!_adcInput.IsOpened)
            { 
                Console.WriteLine("ADC Port Not Opened");
                return;
            }
        }
    }
}
