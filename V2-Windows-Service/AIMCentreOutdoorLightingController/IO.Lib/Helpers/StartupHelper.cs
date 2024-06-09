namespace IO.Lib.Helpers
{
    public static class StartupHelper
    {
        public static void LogStartupError(Exception ex)
            => File.WriteAllText("_STARTUP_ERRORS.txt", $"{DateTime.Now}{Environment.NewLine}{ex.Message}{Environment.NewLine}{ex.StackTrace}");
    }
}
