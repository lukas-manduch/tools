namespace SqliteParser
{
    static class Constants
    {
        public const string SQLITE_HEADER = "SQLite format 3";
        public const int HEADER_SIZE = 100;
        public const int SQLITE_HEADER_INDEX_INTERNAL = 2;
        public const int SQLITE_HEADER_TABLE_INTERNAL = 5;
        public const int SQLITE_HEADER_INDEX_LEAF = 10;
        public const int SQLITE_HEADER_TABLE_LEAF = 13;
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
                Console.WriteLine($"Page size {reader.Header.PageSize}");
                var page = reader.GetPage(0);

                Console.WriteLine($"Free block start {page.FreeBlockStart}");
                Console.WriteLine($"Cell count {page.CellCount}");
                Console.WriteLine($"Cell start {page.CellStart}");
                Console.WriteLine($"Fragmented bytes {page.FragmentedFreeBytes}");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error while processing file '{args[0]}': {ex.Message}");
                Console.WriteLine();
                Console.WriteLine(ex);
            }
            Environment.Exit(0);
        }
    }
}