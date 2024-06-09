using IO.Lib.Exceptions;
using IO.Lib.IO;
using System.Linq;

namespace AIMCentreOutdoorLightingController
{
    public class Worker : BackgroundService
    {
        private readonly ILogger<Worker> _logger;
        protected SensorInput _sensorInput;

        public Worker(ILogger<Worker> logger)
        {
            _logger = logger;
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            try
            {

                var ports = UsbPortHandler.ListPorts();
                string? port = ports.Length > 0 ? ports[0] : null;

                _sensorInput = new SensorInput(port);


                while (!stoppingToken.IsCancellationRequested)
                {
                    //_logger.LogInformation($"Ports: {string.Join(", ", ports)}");
                    string data = _sensorInput.ReadLine();
                    Console.WriteLine(data);

                    await Task.Delay(1000, stoppingToken);
                }
            }
            catch(LightingControllerException ex)
            {
                _logger.LogError(ex.Message);

            }
        }
    }
}
