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
    }
}
