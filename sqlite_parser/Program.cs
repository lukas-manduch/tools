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
        public static void PrintTableLeafPage(Model.TableLeafPage page)
        {
            for (int index = 0; index < page.CellCount; index++)
            {
                var cell = (Model.TableLeafCell)page.GetCell(index);
                Console.WriteLine($" &{page.CellPointers[index].ToString()} - RowID {cell.RowID}");
                foreach (var entry in cell.Entries)
                {
                    Console.WriteLine($"  {entry}");
                }
            }
        }

        public static void PrintTableInteriorPage(Model.TableInteriorPage page)
        {
            for (int index = 0; index < page.CellCount; index++)
            {
                var cell = (Model.TableInteriorCell)page.GetCell(index);
                Console.WriteLine($" &{page.CellPointers[index].ToString()} - RowID {cell.RowID} PageP => {cell.PagePointer}");
            }
        }

        public static void PrintIndexLeafPage(Model.IndexLeafPage page)
        {
            for (int index = 0; index < page.CellCount; index++)
            {
                var cell = (Model.IndexLeafCell)page.GetCell(index);
                Console.WriteLine($" &{page.CellPointers[index].ToString()}");
                foreach (var entry in cell.Entries)
                {
                    Console.WriteLine($"  {entry}");
                }
            }
        }

        public static void PrintIndexInteriorPage(Model.IndexInteriorPage page)
        {
            for (int index = 0; index < page.CellCount; index++)
            {
                var cell = (Model.IndexInteriorCell)page.GetCell(index);
                Console.WriteLine($" &{page.CellPointers[index].ToString()} - PageP => {cell.LeftChildPointer}");
                foreach (var entry in cell.Entries)
                {
                    Console.WriteLine($"  {entry}");
                }
            }
        }

        public static void PrintPage(Model.Page page)
        {           Console.WriteLine($"-- Page {page.PageIndex} --");
            Console.WriteLine($"Free block start {page.FreeBlockStart}");
            Console.WriteLine($"Cell count {page.CellCount}");
            Console.WriteLine($"Cell start {page.CellStart}");
            Console.WriteLine($"Fragmented bytes {page.FragmentedFreeBytes}");
            Console.WriteLine($"Cell pointers count: {page.CellPointers.Count}");
            if (page.HasRightmostPointer)
                Console.WriteLine($"Rightmost pointer: {page.RightmostPointer} ");

            if (page.PageType == Constants.SQLITE_HEADER_TABLE_LEAF)
                PrintTableLeafPage((Model.TableLeafPage)page);
            if (page.PageType == Constants.SQLITE_HEADER_TABLE_INTERNAL)
                PrintTableInteriorPage((Model.TableInteriorPage)page);
            if (page.PageType == Constants.SQLITE_HEADER_INDEX_LEAF)
                PrintIndexLeafPage((Model.IndexLeafPage)page);
            if (page.PageType == Constants.SQLITE_HEADER_INDEX_INTERNAL)
                PrintIndexInteriorPage((Model.IndexInteriorPage)page);
        }

        public static void PrintPages(DbReader reader)
        {
            for (uint i = 1; i <= reader.Header.PageCount; i++)
            {
                var page = reader.GetPage(i);
                string pageDesc = page.PageType switch
                {
                    Constants.SQLITE_HEADER_TABLE_LEAF => "Table leaf",
                    Constants.SQLITE_HEADER_TABLE_INTERNAL => "Table interior page",
                    Constants.SQLITE_HEADER_INDEX_LEAF => "Index leaf",
                    Constants.SQLITE_HEADER_INDEX_INTERNAL => "Index interior page",
                    _ => "ERROR (unrecognized/not implemented)"
                };
                Console.WriteLine($"[{i}] - ({page.PageType}) {pageDesc}");
            }
        }

        public static void Main(string[] args)
        {
            if (args.Length != 1)
            {
                Console.WriteLine("Wrong number of args. Pass sqlite file as arg 1");
                Environment.Exit(1);
            }

            try
            {
                Helpers.FileType ft = Helpers.DetectFileFormat(args[0]);
                if (ft != Helpers.FileType.Database)
                {
                    Console.WriteLine("Wrong file format");
                    Environment.Exit(1);
                }

                DbReader reader = new DbReader(args[0]);
                Console.WriteLine($"Page size {reader.Header.PageSize}");
                Console.WriteLine($"Page Count {reader.Header.PageCount}");
                Console.WriteLine($"Free list page {reader.Header.FreeListPage}");
                Console.WriteLine();
                while (true)
                {
                    Console.Write("Show page (or q): ");
                    string readLine = Console.ReadLine() ?? "q";
                    if (readLine == "q")
                        Environment.Exit(0);
                    if (readLine == "?")
                    {
                        PrintPages(reader);
                        continue;
                    }
                    if (readLine == "")
                        continue;

                    try
                    {
                        uint requestedIndex = System.Convert.ToUInt32(readLine);
                        PrintPage(reader.GetPage(requestedIndex));
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine(e);
                    }
                }
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