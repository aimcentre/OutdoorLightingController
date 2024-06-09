using IO.Lib.Exceptions;
using Microsoft.Extensions.Configuration;


namespace IO.Lib.Helpers
{
    public class ConfigHelper
    {
        #region Singleton set-up
        private static ConfigHelper _instance = null!;
        public static ConfigHelper Initialize(ConfigurationManager configuration)
            => _instance = new ConfigHelper(configuration);

        public static ConfigHelper Instance
            => _instance ?? throw new LightingControllerException("ConfigHelper singleton is not initialized. Please call \"ConfigHelper.Initialize(ConfigurationManager configuration)\" in the start-up routine of the application.");
        #endregion


        private readonly ConfigurationManager _configuration;
        private ConfigHelper(ConfigurationManager configuration)
        {
            _configuration = configuration;
        }
        public string GetConnectionString(string connectionStringName)
        {
            var connectionString = _configuration.GetConnectionString(connectionStringName);
            if (string.IsNullOrEmpty(connectionString))
                throw new LightingControllerException($"Connection string \"{connectionStringName}\" is not found.");
            return connectionString;
        }

        public string? GetValue(string key, bool isRequired = true)
        {
            var val = _configuration.GetSection(key).Value;
            if (isRequired && string.IsNullOrEmpty(val))
                throw new LightingControllerException($"Configuration value \"{key}\" is not found.");
            return val;
        }

        public T GetValue<T>(string key)
            => _configuration.GetSection(key).Get<T>() ?? throw new LightingControllerException($"Configuration section \"{key}\" is not found or invalid.");

    }
}
