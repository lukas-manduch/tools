namespace SqliteParser;
using SqliteParser.Model;

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
    public static (UInt64 Value, ushort Length) ParseVarint(IList<byte> varint)
    {
        byte overflowFlag = 0x80; // 1000 0000
        UInt32 result = 0;
        for (ushort i = 0; i < 8; i++)
        {
            if (varint.Count == i)
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

    public static CellEntry ParseCellEntry(IList<byte> bytes)
    {
        var header = ParseVarint(bytes);
        byte[] payload = bytes.Skip(header.Length).ToArray();

        string value = header.Value switch
        {
            _ => header.Value.ToString(),
        };

        var valLength = 0;

        if (header.Value == 1)
        {
            valLength = 1;
        }
        else if (header.Value == 2)
        {
            valLength = 2;
        }
        else if ((header.Value % 2 == 1) && (header.Value >= 13))
        {
            valLength = (int)(header.Value - 13) / 2;
            value = System.Text.Encoding.UTF8.GetString(payload[..valLength]);
        }
        else if ((header.Value % 2 == 0) && (header.Value >= 12))
        {
            valLength = (int)(header.Value - 12) / 2;
            value = System.Text.Encoding.UTF8.GetString(payload);
        }
        else if (header.Value == 0)
        {
            valLength = 1;
        }
        else
        {
            throw new ArgumentException($"This is f up {header.Value}");
        }


        return new CellEntry()
        {
            Value = value,
            Raw = payload,
            Varint = header.Value,
            Size = header.Length + valLength,
            Type = "Idk"
        };
    }
}