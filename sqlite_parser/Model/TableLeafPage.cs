namespace SqliteParser.Model;

class TableLeafPage : Page
{
    public TableLeafPage(byte[] data, uint index)
        : base(data, index)
    {

    }
    public override Cell GetCell(int index)
    {
        return new TableLeafCell(GetCellInternal(index));
    }
    
    protected override int GetCellSize(ReadOnlySpan<byte> data)
    {
        return TableLeafCell.GetSize(data);
    }
}
