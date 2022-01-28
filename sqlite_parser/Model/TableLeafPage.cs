namespace SqliteParser.Model;
using System.Diagnostics;
using System.Collections.Generic;

class Cell
{
    public Cell(byte[] data)
    {
        var payloadSize = Helpers.ParseVarint(data);
        var rowId = Helpers.ParseVarint(data[payloadSize.Length..]);
        RowID = rowId.Value;
        int realPayloadSize = data.Length - payloadSize.Length - rowId.Length;

        if (realPayloadSize == (int)payloadSize.Value)
        {
            int headerSize = payloadSize.Length + rowId.Length;
            Data = data[headerSize..].ToList();
        }
        else
        {
            throw new ArgumentException("Overflown cell, not implemented");
        }
        Console.WriteLine(System.Text.Encoding.UTF8.GetString(Data.ToArray()));
    }

    public UInt64 RowID;
    public List<byte> Data;
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
    public Cell GetCell(int index)
    {
        Debug.Assert(index < CellCount);

        // We have to find nearest bigger index
        var sorted = CellPointers.ToArray();
        Array.Sort(sorted, (a,b) => a.CompareTo(b));  // Ascending sort
        int position = Array.BinarySearch(sorted, CellPointers[index]);
        Debug.Assert(position >= 0); // We didn't find ourselves?
        if (position == (CellCount-1))
        {
            // This is last cell, so till the end. There could be extension data
            // but we don't support that scenario. So we would fail somewhere
            return new(Data[CellPointers[index]..]);
        }
        // We return
        return new(Data[sorted[position]..sorted[position+1]]);
    }
}
