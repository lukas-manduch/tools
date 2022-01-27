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


    /// <summary>
    /// Read varint from given byte array.  Returns it's value and length
    /// </summary>
    public static (Int64 Value, ushort Length) ParseVarint(byte[] varint)
    {
        byte overflowFlag = 0x80; // 1000 0000
        UInt32 result = 0;
        const ushort one = 1;
        for (ushort i = 0; i < 8; i++)
        {
            if (varint.Length == i)
            {
                // We can't index here anymore
                return (Value: result, Length: (ushort)(i+1));
            }
            result = result << 7;
            result = result | (varint[i] & 0x7Fu);
            if ((overflowFlag & varint[i]) == 0)
            {
                // This is last entry
                return (Value: result, Length: (ushort)(i+1));
            }
        }
        throw new InvalidOperationException("9 byte varint not implemented");
    }
}