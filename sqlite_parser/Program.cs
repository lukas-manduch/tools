namespace SqliteParser
{
    class Program
    {
        public static void Main(string[] args)
        {
            if (args.Length != 1)
            {
                Console.WriteLine("Wrong number of args. Pass sqlite file as arg 1");
                Environment.Exit(1);
            }
            
            try
            {
                byte[] headerBytes = new byte[100];

                using (var fileStream = File.Open(args[0], FileMode.Open))
                {
                    using (var binaryReader = new BinaryReader(fileStream))
                    {
                        binaryReader.Read(headerBytes, 0, 100);
                    }
                }
                var header = Header.FromBytes(headerBytes);
                Console.WriteLine($"Header: {header.Heading} ");
                Console.WriteLine($"Page size: { header.PageSize} ");
                Console.WriteLine($"Read&Write ver: { header.ReadVersion} {header.WriteVersion} ");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error while processing file '{args[0]}': {ex.Message}");
            }
            Environment.Exit(0);
        }
    }
}