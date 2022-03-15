using SqliteParser.Service;
using System.Diagnostics;

namespace SqliteParser.Model;

class Wal
{
    private readonly FileReader _reader;

    public Wal(string fileName)
    {
        _reader = new Service.FileReader(fileName);
        if (_reader.FileSize < 32)
            throw new ArgumentException("Invalid WAL (too small)");
        byte[] header = _reader.ReadBytes(0, 32);
        Debug.Assert(header.Length == 32);
        // Check magic numbers
        MagicNumber = Helpers.ParseU32(header[0..4]);
        if (MagicNumber != 0x377f0682 && MagicNumber != 0x377f0683)
            throw new ArgumentException("Invalid wal file (bad magic)");
        if (Helpers.ParseU32(header[4..8]) != 3007000)
            throw new ArgumentException("Invalid WAL format");

        PageSize = Helpers.ParseU32(header[8..12]);
        Checkpoint = Helpers.ParseU32(header[12..16]);
        Salt1 = Helpers.ParseU32(header[16..20]);
        Salt2 = Helpers.ParseU32(header[20..24]);
        Checksum1 = Helpers.ParseU32(header[24..28]);
        Checksum2 = Helpers.ParseU32(header[28..32]);
        LoadWal();
    }

    private delegate (UInt32 S0, UInt32 S1) ComputeChecksum(ReadOnlySpan<byte> data, UInt32 s0, UInt32 s1);

    /// <summary>
    /// Load all valid wal frames to memory.
    /// </summary>
    private void LoadWal()
    {
        Debug.Assert(PageSize != 0);
        bool bigEndian = true;
        if (MagicNumber == 0x377f0682)
            bigEndian = false;
        ComputeChecksum computeCHecksum = (d, s0, s1) => Helpers.WalChecksum(d, s0, s1, isBE: bigEndian);
        long frameCount = _reader.FileSize - 32; // 32 -> wal head
        if (frameCount % (PageSize + 24) != 0)
            throw new ArgumentException("Wrong wal file size");
        frameCount /= PageSize + 24; // 24 -> Stack header
        var (s0, s1) = Helpers.WalChecksum(_reader.ReadBytes(0, 24), 0, 0);
        if (s0 != Checksum1 || s1 != Checksum2)
            throw new ArgumentException("Wal header checksum is wrong");
        // Now we have valid wal file.
        // Let's load all valid frames one by one
        int walFrameSize = (int)PageSize + 24;
        int offset = 32;
        // Loop until first invalid frame or end.  Save all valid frames
        while (offset < _reader.FileSize)
        {
            var data = _reader.ReadBytes(offset, offset + walFrameSize);
            var frame = new WalFrame(data);
            bool checksumValid = frame.IsChecksumValid(s0, s1, bigEndian);
            bool saltMatch = frame.Salt1 == Salt1 && frame.Salt2 == Salt2;

            if (!checksumValid || !saltMatch)
                // This is invalid frame, so everythin after that is also invalid
                break;

            (s0, s1) = (frame.Checksum1, frame.Checksum2);
            Frames.Add(frame);
            offset += walFrameSize;
        }
    }


    public UInt32 MagicNumber;
    public UInt32 Salt1 = 0;
    public UInt32 Salt2 = 0;
    public UInt32 PageSize = 0;
    public UInt32 Checkpoint = 0;
    public UInt32 Checksum1 = 0;
    public UInt32 Checksum2 = 0;
    public List<WalFrame> Frames = new();
}
