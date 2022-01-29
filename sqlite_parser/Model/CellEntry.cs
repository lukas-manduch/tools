namespace SqliteParser.Model;

public record CellEntry
{
    public string Type = "";
    public string Value = "";
    /// <summary>
    /// Total length of entry including varint header
    /// </summary>
    public int Size;
    public UInt64 Varint;
    public byte[] Raw = { };
}
