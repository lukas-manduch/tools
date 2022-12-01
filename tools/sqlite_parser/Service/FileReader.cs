namespace SqliteParser.Service;
using System.Diagnostics;

class FileReader
{
    public FileReader(string fileName)
    {
        try
        {
            _filePath = System.IO.Path.GetFullPath(fileName);
            // Check if we can open this file
            using (var fileStream = File.Open(_filePath, FileMode.Open))
            {
                // Empty
            }
            _fileInfo = new FileInfo(_filePath);
        }
        catch (Exception e)
        {
            throw new ArgumentException(nameof(fileName), e.Message);
        }
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

        using (var fileStream = File.Open(_filePath, FileMode.Open))
        {
            fileStream.Seek(from, SeekOrigin.Begin);

            using (var binaryReader = new BinaryReader(fileStream))
            {
                binaryReader.Read(fileBytes, 0, to - from);
            }
        }
        return fileBytes;
    }

    public long FileSize { get => _fileInfo.Length; }

    private string _filePath;
    private FileInfo _fileInfo;
}

