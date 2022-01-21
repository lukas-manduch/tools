namespace SqliteParser;
using SqliteParser.Model;

class DbReader
{
    public DbHeader Header { get { return _Header; }}

    public DbReader(string fileName)
    {
        _fileName = System.IO.Path.GetFullPath(fileName);
        parseHeader();
    }


    private void parseHeader()
    {
        byte[] headerBytes = new byte[Constants.HEADER_SIZE];

        using (var fileStream = File.Open(_fileName, FileMode.Open))
        {
            using (var binaryReader = new BinaryReader(fileStream))
            {
                binaryReader.Read(headerBytes, 0, Constants.HEADER_SIZE);
            }
        }
        _Header = new Model.DbHeader()
        {
            Heading = Helpers.ParseU8Str(headerBytes[0..15]),
            PageSize = Helpers.ParseU16(headerBytes[15..17]),
            WriteVersion = headerBytes[17],
            ReadVersion = headerBytes[18]
        };
    }

    private string _fileName;
    private DbHeader _Header = new DbHeader();
}

