namespace SqliteParser.Model;

struct DbHeader
{
    private int _PageSize;
    public int PageSize
    {
        get => _PageSize;
        set
        {
            if (value == 1) // Special sqlite case
                _PageSize = 0xFFFF;
            else
                _PageSize = value;
            if (_PageSize % 2 != 0)
                throw new ArgumentException(nameof(PageSize), "Must be pow of 2");
        }
    }
    public string Heading
    {
        get
        {
            return Constants.SQLITE_HEADER;
        }
        set
        {
            if (value != Constants.SQLITE_HEADER)
            {
                throw new ArgumentException("Heading", "Invalid header for sqlite3");
            }
        }
    }
    public uint FreeListPage;
    public uint PageCount;
    public byte WriteVersion;
    public byte ReadVersion;
}

