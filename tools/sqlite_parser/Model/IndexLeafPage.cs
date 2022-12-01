namespace SqliteParser.Model;
class IndexLeafPage : Page
{
    public IndexLeafPage(ReadOnlySpan<byte> data, uint index)
        : base(data, index)
    {
    }

    public override Cell GetCell(int index)
    {
        return new IndexLeafCell(GetCellInternal(index));
    }

    protected override int GetCellSize(ReadOnlySpan<byte> data)
    {
        return IndexLeafCell.GetSize(data);
    }
}
