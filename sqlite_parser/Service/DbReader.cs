namespace SqliteParser;
using SqliteParser.Model;
using System.Diagnostics;

class DbReader
{
    public DbHeader Header { get { return _Header; }}

    public DbReader(string fileName)
    {
        _fileName = System.IO.Path.GetFullPath(fileName);
        _fileInfo = new FileInfo(_fileName);
        parseHeader();
        // TODO: Assert that no db extensions are enabled
    }


    private void parseHeader()
    {
        byte[] headerBytes = new byte[Constants.HEADER_SIZE];

        headerBytes = ReadBytes(0, Constants.HEADER_SIZE);
        _Header = new Model.DbHeader()
        {
            Heading = Helpers.ParseU8Str(headerBytes[0..16]),
            PageSize = Helpers.ParseU16(headerBytes[16..18]),
            PageCount = Helpers.ParseU32(headerBytes[28..32]),
            WriteVersion = headerBytes[18],
            ReadVersion = headerBytes[19]
        };
    }

    public byte[] ReadBytes(int from, int to)
    {
        Debug.Assert(from >= 0 && to >= 0);
        if (from >= to)
        {
            throw new ArgumentException(nameof(to), "From must be less than to");
        }
        if (to > _fileInfo.Length)
        {
            throw new ArgumentException("Reading out of file data");
        }

        byte[] fileBytes = new byte[to - from];

        using (var fileStream = File.Open(_fileName, FileMode.Open))
        {
            fileStream.Seek(from, SeekOrigin.Begin);

            using (var binaryReader = new BinaryReader(fileStream))
            {
                binaryReader.Read(fileBytes, 0, to-from);
            }
        }
        return fileBytes;
    }

    public Page GetPage(uint index)
    {
        byte[] data;
        data = ReadBytes((int) (Header.PageSize*index), (int) (Header.PageSize*(index+1)));
        int offset = 0;
        if (index == 0)
            offset = 100;

        switch (data[offset])
        {
            
            case Constants.SQLITE_HEADER_TABLE_LEAF:
                return new TableLeafPage(data, index);
            case Constants.SQLITE_HEADER_INDEX_INTERNAL:
            case Constants.SQLITE_HEADER_INDEX_LEAF:
            case Constants.SQLITE_HEADER_TABLE_INTERNAL:
            default:
                throw new ArgumentException("GetPage", $"Page[{index}] seems corrupted, type {data[0]}");
        }

    }

    private string _fileName;
    private FileInfo _fileInfo;
    private DbHeader _Header = new DbHeader();
}

