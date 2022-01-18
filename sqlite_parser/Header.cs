namespace SqliteParser;

using System.Diagnostics;
using System.Runtime.InteropServices;

[StructLayout(LayoutKind.Sequential, Pack = 0)]
internal unsafe struct SqliteUnsafeHeader
{
    public fixed byte Heading[16];
    public fixed byte Rest[84];
}

struct SqliteHeader
{
    public string Heading;
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


        Console.WriteLine("Size is {0}", sizeof(SqliteUnsafeHeader));
        return new SqliteHeader()
        {
            Heading = System.Text.Encoding.Default.GetString(header.Heading, 16)
        };
    }
}

