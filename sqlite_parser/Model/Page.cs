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
            return PageType == 2 || PageType == 5;
        }
    }
    public List<UInt16> CellPointers
    {
        get
        {
            byte[] data;
            List<UInt16> result = new();
            if (HasRightmostPointer)
                data = StartData[12..(12 + CellCount*2)];
            data = StartData[8..(8 + CellCount*2)];
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
        if (index == 0)
            StartData = pageBytes[100..];
    }

    private byte[] StartData;

}
