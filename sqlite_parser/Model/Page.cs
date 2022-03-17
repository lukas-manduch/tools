using System.Diagnostics;
namespace SqliteParser.Model;
internal class Page
{
    public byte[] Data;
    public uint PageIndex;

    public byte PageType { get => StartData[0]; }
    public UInt16 FreeBlockStart { get => Helpers.ParseU16(StartData[1..3]); }
    public UInt16 CellCount { get => Helpers.ParseU16(StartData[3..5]); }
    public UInt32 CellStart
    {
        get
        {
            UInt16 cellStart = Helpers.ParseU16(StartData[5..7]);
            return cellStart == 0 ? 65536u : cellStart;
        }
    }
    public UInt32 FragmentedFreeBytes { get => StartData[7]; }
    public bool HasRightmostPointer
    {
        get
        {
            // These are codes for interior pages
            return (PageType == Constants.SQLITE_HEADER_TABLE_INTERNAL) ||
                (PageType == Constants.SQLITE_HEADER_INDEX_INTERNAL);
        }
    }
    public UInt32 RightmostPointer
    {
        get
        {
            if (!HasRightmostPointer)
                throw new InvalidOperationException("Page type doesn't have righmost pointer");
            return Helpers.ParseU32(StartData[8..12]);
        }
    }
    public List<UInt16> CellPointers
    {
        get
        {
            byte[] data;
            List<UInt16> result = new();
            data = StartData[8..(8 + CellCount * 2)];
            if (HasRightmostPointer)
                data = StartData[12..(12 + CellCount * 2)];
            Debug.Assert((data.Length % 2) == 0);
            for (int i = 0; i < data.Length; i += 2)
            {
                result.Add(Helpers.ParseU16(data[i..(i + 2)]));
            }

            return result;
        }
    }

    public Page(ReadOnlySpan<byte> pageBytes, uint index)
    {
        Data = pageBytes.ToArray();
        PageIndex = index;
        StartData = pageBytes.ToArray();
        if (index == 1)
            StartData = pageBytes.Slice(100).ToArray();
    }


    /// <summary>
    /// Given cell pointer index, convert entry to row and return it
    /// </summary>
    public virtual Cell GetCell(int index)
    {
        throw new NotImplementedException();
    }

    protected virtual int GetCellSize(ReadOnlySpan<byte> data)
    {
        throw new NotImplementedException("Operation not supported");
    }


    protected byte[] GetCellInternal(int index)
    {
        Debug.Assert(index < CellCount);
        var cellAddress = CellPointers[index];
        // Memory from cell start till the end
        Span<byte> cellSlice = Data.AsSpan(cellAddress);
        int cellSize = GetCellSize(cellSlice);
        Debug.Assert(cellSize > 0);
        return Data[cellAddress..(cellAddress + cellSize)];
    }

    private byte[] StartData;

}
