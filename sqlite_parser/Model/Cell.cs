namespace SqliteParser.Model;
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
        Entries = Helpers.ParseTableLeafCellPayload(PayloadData);
    }

    public UInt64 RowID;
    public List<byte> PayloadData;
    public List<CellEntry> Entries;
}
