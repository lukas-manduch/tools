namespace SqliteParser;
using SqliteParser.Model;
using System.Collections.Generic;
using System.Diagnostics;


class Helpers
{
    public static UInt32 ParseU32(ReadOnlySpan<byte> sqlBytes)
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
    /// Read varint from given byte enumerable.  Returns it's value and length
    /// </summary>
    public static (UInt64 Value, ushort Length) ParseVarint(IEnumerable<byte> varint)
    {
        const byte overflowFlag = 0x80; // 1000 0000
        int index = 0;
        UInt64 value = 0;
        List<byte> bs = new();
        foreach (byte b in varint)
        {
            if (index == 8)
                throw new ArgumentException("Varint is supported only up to 8 bytes");

            bs.Add((byte)(b & (~overflowFlag)));
            index++;
            if ((b & overflowFlag) == 0)
                break;
        }
        bs.Reverse();
        for (int i = 0; i < bs.Count; i++)
        {
            value |= (UInt64)bs[i] << (7 * i);
        }
        return (Value: value, Length: (ushort)bs.Count);
    }

    /// <summary>
    /// Read varint from given byte array.  Returns it's value and length
    /// </summary>
    public static (UInt64 Value, ushort Length) ParseVarint(ReadOnlySpan<byte> varint)
    {
        const byte overflowFlag = 0x80; // 1000 0000
        UInt32 result = 0;
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

    public static (UInt64 Value, ushort Length) ParseVarint(byte[] varint)
    {
        return Helpers.ParseVarint((ReadOnlySpan<byte>)varint);
    }


    public static List<CellEntry> ParseLeafCellPayload(IReadOnlyList<byte> payload)
    {
        var headerSize = ParseVarint(payload);

        byte[] header = payload.Skip(headerSize.Length).Take((int)(headerSize.Value - headerSize.Length)).ToArray();

        
        List<CellEntry> result = new();
        
        int hearderIndex = 0;
        int payloadOffset = (int)headerSize.Value;
        
        while(hearderIndex < header.Length)
        {
            var entryType = ParseVarint(header[hearderIndex..]);
            CellEntry current;

            switch (entryType.Value)
            {
                case 0:
                    current = new()
                    {
                        HeaderValue = entryType.Value,
                        Value = "0",
                        ValueType = "NULL"
                    };
                    break;
                case 1:
                    current = new()
                    {
                        Value = payload[payloadOffset].ToString(),
                        ValueType = "byte",
                        HeaderValue = entryType.Value,
                    };
                    payloadOffset += 1;
                    break;
                case 2:
                    current = new()
                    {
                        Value = ParseU16(payload.Skip(payloadOffset).Take(2).ToArray()).ToString(),
                        ValueType = "WORD",
                        HeaderValue = entryType.Value,
                    };
                    payloadOffset += 2;
                    break;
                case 9:
                    current = new()
                    {
                        Value = "1",
                        ValueType = "Type1 (special no data)",
                        HeaderValue = entryType.Value,
                    };
                    break;
                case (>= 13) when (entryType.Value % 2 == 1):
                    int stringLength = (int) (entryType.Value -13 ) / 2;
                    byte[] stringPayload = payload.Skip(payloadOffset).Take(stringLength).ToArray();
                    current = new()
                    {
                        ValueType = "string",
                        Value = System.Text.Encoding.UTF8.GetString(stringPayload),
                        HeaderValue = entryType.Value,
                        Raw = stringPayload
                    };
                    payloadOffset += stringLength;
                    break;
                default:
                    throw new ArgumentException($"Parse cell failed, unknown type {entryType.Value}");
            }
            Debug.Assert(current is not null);
            hearderIndex += entryType.Length;
            result.Add(current);
        }
        return result;
    }

    public enum FileType
    {
        Database,
        WAL,
        Empty,
        Unknown
    }


    public static FileType DetectFileFormat(string filePath)
    {
        var reader = new Service.FileReader(filePath);
        // Check minimal size
        if (reader.FileSize == 0)
            return FileType.Empty;
        if (reader.FileSize < 32)
            return FileType.Unknown;
        byte[] data = reader.ReadBytes(0, 15);

        // Check for WAL magic numbers
        UInt32 header1 = Helpers.ParseU32(data[0..4]);
        if (header1 == 0x377f0682 || header1 == 0x377f0683)
            return FileType.WAL;

        // Check for sqlite magic string
        string header2 = System.Text.Encoding.ASCII.GetString(data);
        if (header2 == Constants.SQLITE_HEADER)
            return FileType.Database;

        return FileType.Unknown;
    }
}