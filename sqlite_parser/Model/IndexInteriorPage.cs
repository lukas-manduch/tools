namespace SqliteParser.Model;
class IndexInteriorPage : Page
{
    public IndexInteriorPage(byte[] data, uint index)
        :base(data, index)
    {
    }
    public override Cell GetCell(int index)
    {
        return new IndexInteriorCell(GetCellInternal(index));
    }

    protected override int GetCellSize(ReadOnlySpan<byte> data)
    {
        return IndexInteriorCell.GetSize(data);
    }

}

