namespace SqliteParser.Service;
class ConsolePrinter
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
}
