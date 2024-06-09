using AIMCentreOutdoorLightingController;
using IO.Lib.Estensions;
using IO.Lib.Helpers;


try
{
    var builder = Host.CreateApplicationBuilder(args);
    ConfigurationManager configuration = builder.Configuration;
    ConfigHelper config = ConfigHelper.Initialize(configuration);

    var logLevelString = config.GetValue("Logging:LogLevel:Default")!;
    builder.AddSerilogLogging(logLevelString);

    builder.Services.AddHostedService<Worker>();

    var host = builder.Build();
    host.Run();
}
catch (Exception ex)
{
    StartupHelper.LogStartupError(ex);
    throw;
}