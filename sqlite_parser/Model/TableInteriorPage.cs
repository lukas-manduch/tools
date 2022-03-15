namespace SqliteParser.Model;
class TableInteriorPage : Page
{
    public TableInteriorPage(byte[] data, uint index)
        : base(data, index)
    {

    }
    public override Cell GetCell(int index)
    {
        return new TableInteriorCell(GetCellInternal(index));
    }

    protected override int GetCellSize(ReadOnlySpan<byte> data)
    {
        return TableInteriorCell.GetSize(data);
    }
}

