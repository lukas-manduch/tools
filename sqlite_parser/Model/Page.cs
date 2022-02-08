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
            return PageType == Constants.SQLITE_HEADER_TABLE_INTERNAL;
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
            data = StartData[8..(8 + CellCount*2)];
            if (HasRightmostPointer)
                data = StartData[12..(12 + CellCount*2)];
            Debug.Assert((data.Length % 2) == 0);
            for (int i = 0; i < data.Length; i += 2)
            {
                result.Add(Helpers.ParseU16(data[i..(i + 2)]));
            }

            return result;
        }
    }

    public Page(byte[] pageBytes, uint index)
    {
        Data = pageBytes;
        PageIndex = index;
        StartData = pageBytes;
        if (index == 1)
            StartData = pageBytes[100..];
    }


    /// <summary>
    /// Given cell pointer index, convert entry to row and return it
    /// </summary>
    public virtual Cell GetCell(int index)
    {
        throw new NotImplementedException();
    }

    protected byte[] GetCellInternal(int index)
    {
        Debug.Assert(index < CellCount);

        // We have to find nearest bigger index
        var sorted = CellPointers.ToArray();
        Array.Sort(sorted, (a, b) => a.CompareTo(b));  // Ascending sort
        int position = Array.BinarySearch(sorted, CellPointers[index]);
        Debug.Assert(position >= 0); // We didn't find ourselves?
        if (position == (CellCount - 1))
        {
            // This is last cell, so till the end. There could be extension data
            // but we don't support that scenario. So we would fail somewhere
            return Data[CellPointers[index]..];
        }
        // We return
        return Data[sorted[position]..sorted[position + 1]];
    }

    private byte[] StartData;

}
