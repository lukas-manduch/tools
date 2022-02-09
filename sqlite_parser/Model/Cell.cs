namespace SqliteParser.Model;
using System.Diagnostics;
class Cell
{
    public Cell(byte[] data)
    {
        Data = data.ToList();
    }

    public List<byte> Data;
}

class TableLeafCell : Cell
{
    public TableLeafCell(byte[] data)
        :base(data)
    {
        var payloadSize = Helpers.ParseVarint(data);
        var rowId = Helpers.ParseVarint(data[payloadSize.Length..]);
        RowID = rowId.Value;
        int realPayloadSize = data.Length - payloadSize.Length - rowId.Length;

        if (realPayloadSize == (int)payloadSize.Value)
        {
            int headerSize = payloadSize.Length + rowId.Length;
            PayloadData = data[headerSize..].ToList();
        }
        else
        {
            throw new ArgumentException("Overflown cell, not implemented");
        }
        Entries = Helpers.ParseLeafCellPayload(PayloadData);
    }

    public static int GetSize(ReadOnlySpan<byte> data)
    {
        var payloadVarint = Helpers.ParseVarint(data);
        var rowVarint = Helpers.ParseVarint(data.Slice(payloadVarint.Length));
        return payloadVarint.Length + rowVarint.Length + (int)payloadVarint.Value;
    }

    public UInt64 RowID;
    public List<byte> PayloadData;
    public List<CellEntry> Entries;
}

class TableInteriorCell : Cell
{
    public TableInteriorCell(byte[] data)
        : base(data)
    {
        // DWORD + VARINT
        Debug.Assert(data.Length < (4 + 9), $"too big cell {data.Length}");
        PagePointer = Helpers.ParseU32(data[0..4]);
        RowID = Helpers.ParseVarint(data[4..]).Value;
    }

    public static int GetSize(ReadOnlySpan<byte> data)
    {
        return 4 + Helpers.ParseVarint(data.Slice(4)).Length;
    }
    public UInt32 PagePointer;
    public UInt64 RowID;
}

class IndexLeafCell : Cell
{
    public IndexLeafCell(byte[] data)
        :base(data)
    {
        var headerSizeVarint = Helpers.ParseVarint(data);


        List<byte> b = data
            .AsSpan()
            .Slice(headerSizeVarint.Length, (int)headerSizeVarint.Value)
            .ToArray() // Idk how to avoid this step
            .ToList();

        CellSize = (int)headerSizeVarint.Value;
        Entries = Helpers.ParseLeafCellPayload(b);
    }

    public static int GetSize(ReadOnlySpan<byte> data)
    {
        var varint = Helpers.ParseVarint(data);
        // Sizeof varint + sizeof payload
        return varint.Length + (int)varint.Value;
    }

    public int CellSize;
    public List<CellEntry> Entries = new();
}
