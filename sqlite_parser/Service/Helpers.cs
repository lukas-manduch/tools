namespace SqliteParser;
class Helpers
{
    public static UInt32 ParseU32(byte[] sqlBytes)
    {
        if (sqlBytes == null || sqlBytes.Length != 4)
        {
            throw new ArgumentException(nameof(sqlBytes), "Must be exactly 4 bytes");
        }
        if (BitConverter.IsLittleEndian == false)
        {
            throw new ApplicationException("LE systems not implemented");
        }
        UInt32 b0 = sqlBytes[0];
        UInt32 b1 = sqlBytes[1];
        UInt32 b2 = sqlBytes[2];
        UInt32 b3 = sqlBytes[3];

        UInt32 result = (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;
        return result;
    }

    public static UInt16 ParseU16(byte[] sqlBytes)
    {
        if (sqlBytes == null || sqlBytes.Length != 2)
        {
            Console.WriteLine($"Size is {sqlBytes.Length}");
            throw new ArgumentException(nameof(sqlBytes), "Must be exactly 2 bytes");
        }
        if (BitConverter.IsLittleEndian == false)
        {
            throw new ApplicationException("LE systems not implemented");
        }
        UInt32 b0 = sqlBytes[0];
        UInt32 b1 = sqlBytes[1];

        UInt32 result = (b0 << 8) | b1;
        return (UInt16) result;
    }

    public static string ParseU8Str(byte[] sqlString)
    {
        int position = Array.IndexOf(sqlString, (byte)0u);
        if (position != -1)
            sqlString = sqlString[0..position];
        return System.Text.Encoding.UTF8.GetString(sqlString);
    }
}