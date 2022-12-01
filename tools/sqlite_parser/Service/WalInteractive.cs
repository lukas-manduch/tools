using SqliteParser.Model;
namespace SqliteParser.Service;

public class WalInteractive
{
    private Wal _wal;

    public WalInteractive(string fileName)
    {
        _wal = new Model.Wal(fileName);
    }

    public void Start()
    {
        Console.WriteLine("==Write Ahead Log browsing==");

        while(true)
        {
            Console.Write("\n> ");
            string readLine = Console.ReadLine() ?? "q";
            if (readLine == "q")
                break;
            if (readLine == "")
            {
                Console.WriteLine("q - quit. ? - list . NUM - show frame");
                continue;
            }
            if (readLine == "?")
            {
                Console.WriteLine("Pick wal frame number for details");
                Console.WriteLine("Wal contents:");
                PrintWal();
                continue;
            }

            try
            {
                int requestedIndex = (int)System.Convert.ToUInt32(readLine);
                PrintFrame(requestedIndex);
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
            }
        }
    }

    public void PrintWal()
    {
        for (int i = 0; i < _wal.Frames.Count; i++)
        {
            Console.WriteLine($"[{i}] contains db page {_wal.Frames[i].PageNumber}");
            if (_wal.Frames[i].DbSize != 0)
            {
                Console.WriteLine($" Is commit. New page count: {_wal.Frames[i].DbSize}");
            }
        }
    }

    public void PrintFrame(int frameIndex)
    {
        if (frameIndex >= _wal.Frames.Count)
        {
            Console.WriteLine("Frame doesn't exist");
        }

        Console.WriteLine($"Page: {_wal.Frames[frameIndex].PageNumber}");
        if (_wal.Frames[frameIndex].DbSize != 0)
        {
            Console.WriteLine($"-- Commit -- (new size: {_wal.Frames[frameIndex].DbSize})");
        }
        ConsolePrinter.PrintPage(_wal.Frames[frameIndex].Page);
    }
}
