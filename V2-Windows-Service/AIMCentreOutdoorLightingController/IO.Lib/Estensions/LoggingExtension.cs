

using Microsoft.Extensions.Hosting;
using Serilog;

namespace IO.Lib.Estensions
{
    public static class LoggingExtension
    {
        public static HostApplicationBuilder AddSerilogLogging(this HostApplicationBuilder builder, string logLevelString)
        {
            if (!Enum.TryParse(logLevelString, out Serilog.Events.LogEventLevel logLevel))
                logLevel = Serilog.Events.LogEventLevel.Warning;

            var logger = new LoggerConfiguration()
                .MinimumLevel.Is(logLevel)
                .WriteTo.File("logs/log.txt", rollingInterval: RollingInterval.Day)
                .CreateLogger();
            builder.Logging.AddSerilog(logger);
            return builder;
        }
    }
}
