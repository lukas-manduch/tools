namespace SqliteParser;

using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

[StructLayout(LayoutKind.Sequential, Pack = 0)]
internal unsafe struct SqliteUnsafeHeader
{
    public fixed byte Heading[16];
    public fixed byte PageSize[2];
    public byte WriteVersion;
    public byte ReadVersion;
    public fixed byte Rest[80];
}

struct SqliteHeader
{
    public uint PageSize;
    public string Heading;
    public byte WriteVersion;
    public byte ReadVersion;
}


class Header
{
    public static unsafe SqliteHeader FromBytes(byte [] bytes)
    {
        if (bytes == null || bytes.Length != 100)
        {
            throw new ArgumentException(nameof(bytes), "Must be exactly 100 bytes");
        }

        
        var header = new SqliteUnsafeHeader();
        byte* p = (byte*)&header;
        Debug.Assert(bytes.Length == sizeof(SqliteUnsafeHeader), "Size of Sqlite header is wrong");
        for (int i = 0; i < bytes.Length; i++)
        {
            p[i] = bytes[i];
        }
        
        uint pageSize = BytesToShort(BytesToSafe(header.PageSize, 2), false);
        if (pageSize == 1) // Special condition for sqlite
            pageSize = 0x10000;

        Console.WriteLine("Size is {0}", sizeof(SqliteUnsafeHeader));
        return new SqliteHeader()
        {
            Heading = System.Text.Encoding.Default.GetString(header.Heading, 16),
            PageSize = pageSize,
            ReadVersion = header.ReadVersion,
            WriteVersion = header.WriteVersion,
        };
    }

    private unsafe static byte[] BytesToSafe(byte *p, uint size)
    {
        byte[] ret = new byte[size];
        for (int i = 0; i < size; i++)
        {
            ret[i] = p[i];
        }
        return ret;
    }

    private static ushort BytesToShort(byte[] encodedInt, bool isLittle)
    {
        Debug.Assert(sizeof(ushort) >= encodedInt.Length, "Sizeof uint doesn't match ");
        if (BitConverter.IsLittleEndian != isLittle)
        {
            // We must reverse array
            System.Array.Reverse(encodedInt);
        }
        return BitConverter.ToUInt16(encodedInt);
    }

    private static uint BytesToInt(byte[] encodedInt, bool isLittle)
    {
        Debug.Assert(sizeof(uint) >= encodedInt.Length, "Sizeof uint doesn't match ");
        if (BitConverter.IsLittleEndian != isLittle)
        {
            // We must reverse array
            System.Array.Reverse(encodedInt);
        }
        return BitConverter.ToUInt32(encodedInt);
    }

}

