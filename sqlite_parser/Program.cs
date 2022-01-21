namespace SqliteParser
{
    static class Constants
    {
        public const string SQLITE_HEADER = "SQLite format 3";
        public const int HEADER_SIZE = 100;
    }

    class Program
    {
        public static void Main(string[] args)
        {
            if (args.Length != 1)
            {
                Console.WriteLine("Wrong number of args. Pass sqlite file as arg 1");
                Environment.Exit(1);
            }
            
            try
            {
                DbReader reader = new DbReader(args[0]);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error while processing file '{args[0]}': {ex.Message}");
            }
            Environment.Exit(0);
        }
    }
}