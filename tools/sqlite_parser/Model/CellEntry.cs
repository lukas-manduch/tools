namespace SqliteParser.Model;

public record CellEntry
{
    /// <summary>
    /// Custom (non official) string represenation of value type
    /// </summary>
    public string ValueType = "";
    /// <summary>
    /// Value decoded to string
    /// </summary>
    public string Value = "";
    /// <summary>
    /// Header varint value (may be used to infer original sqlite type)
    /// </summary>
    public UInt64 HeaderValue;
    /// <summary>
    /// Raw value, without varint
    /// </summary>
    public byte[] Raw = { };

    public override string ToString()
    {
        return $"[{ValueType}] {Value}";
    }
}
