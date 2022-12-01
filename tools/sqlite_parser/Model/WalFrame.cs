namespace SqliteParser.Model;
using System.Linq;
using System.Diagnostics;

class WalFrame
{
    public WalFrame(ReadOnlySpan<byte> data)
    {
        Debug.Assert(data.Length > 24);
        ParseHeader(data.Slice(0, 24));
        Data = data.ToArray();
        Page = DbReader.ParsePage(data.Slice(24), PageNumber);
    }

    public bool IsChecksumValid(UInt32 checksum1, UInt32 checksum2, bool isBe)
    {
        var headerChecksumData = Data.AsSpan().Slice(0, 8);
        var pageChecksumData = Data.AsSpan().Slice(24);
        (checksum1, checksum2) = Helpers.WalChecksum(headerChecksumData, checksum1, checksum2, isBe);
        (checksum1, checksum2) = Helpers.WalChecksum(pageChecksumData, checksum1, checksum2, isBe);
        return checksum1 == Checksum1 && checksum2 == Checksum2;
    }

    private void ParseHeader(ReadOnlySpan<byte> header)
    {
        Debug.Assert(header.Length == 24);
        PageNumber = Helpers.ParseU32(header.Slice(0, 4));
        DbSize = Helpers.ParseU32(header.Slice(4, 4));
        Salt1 = Helpers.ParseU32(header.Slice(8, 4));
        Salt2 = Helpers.ParseU32(header.Slice(12, 4));
        Checksum1 = Helpers.ParseU32(header.Slice(16, 4));
        Checksum2 = Helpers.ParseU32(header.Slice(20, 4));
    }

    public UInt32 PageNumber;
    public UInt32 DbSize;
    public UInt32 Salt1;
    public UInt32 Salt2;
    public UInt32 Checksum1;
    public UInt32 Checksum2;

    public byte[] Data;
    public Page Page;
}

