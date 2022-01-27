namespace SqliteParser.Model;
using System.Diagnostics;
class Row
{

}

class TableLeafPage : Page
{
    public TableLeafPage(byte[] data, uint index)
        : base(data, index)
    {

    }

    /// <summary>
    /// Given cell pointer index, convert entry to row and return it
    /// </summary>
    public Row GetRow(int index)
    {
        Debug.Assert(index < CellCount);
        byte[] varint = Data[CellPointers[index]..(CellPointers[index] + 9)];
        Console.WriteLine(Helpers.ParseVarint(varint));
        return new();
    }
}
