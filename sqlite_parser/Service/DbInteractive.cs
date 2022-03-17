using System;
namespace SqliteParser.Service
{
	class DbInteractive
	{
		public DbInteractive(string fileName)
		{
			_reader = new DbReader(fileName);
		}

        public void Start()
        {
            Console.WriteLine($"Page size {_reader.Header.PageSize}");
            Console.WriteLine($"Page Count {_reader.Header.PageCount}");
            Console.WriteLine($"Free list page {_reader.Header.FreeListPage}");
            Console.WriteLine();
            while (true)
            {
                Console.Write("Show page (or q): ");
                string readLine = Console.ReadLine() ?? "q";
                if (readLine == "q")
                    return;
                if (readLine == "?")
                {
                    PrintPages();
                    continue;
                }
                if (readLine == "")
                    continue;

                try
                {
                    uint requestedIndex = System.Convert.ToUInt32(readLine);
                    PrintPage(_reader.GetPage(requestedIndex));
                }
                catch (Exception e)
                {
                    Console.WriteLine(e);
                }
            }
        }

        private void PrintPages()
        {
            for (uint i = 1; i <= _reader.Header.PageCount; i++)
            {
                var page = _reader.GetPage(i);
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

        // TODO: Delete these 5 functions. They are duplicated in ConsolePrinter
        private static void PrintTableLeafPage(Model.TableLeafPage page)
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

        private static void PrintTableInteriorPage(Model.TableInteriorPage page)
        {
            for (int index = 0; index < page.CellCount; index++)
            {
                var cell = (Model.TableInteriorCell)page.GetCell(index);
                Console.WriteLine($" &{page.CellPointers[index].ToString()} - RowID {cell.RowID} PageP => {cell.PagePointer}");
            }
        }

        private static void PrintIndexLeafPage(Model.IndexLeafPage page)
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

        private static void PrintIndexInteriorPage(Model.IndexInteriorPage page)
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

        private static void PrintPage(Model.Page page)
        {
            Console.WriteLine($"-- Page {page.PageIndex} --");
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

        private DbReader _reader;
	}
}

