using System.IO;

namespace MiraUtils.Client.FileExplorer
{
    public enum FileTypes : byte
    {
        DT_UNKNOWN = 0,
        DT_FIFO = 1,
        DT_CHR = 2,
        DT_DIR = 4,
        DT_BLK = 6,
        DT_REG = 8,
        DT_LNK = 10,
        DT_SOCK = 12,
        DT_WHT = 14
    }

    public class FileExplorerDent : MessageSerializable
    {
        public uint Fileno;
        public ushort Reclen;
        public byte Type;
        public byte Namelen;
        public char[] Name;

        public FileExplorerDent()
        {
            Fileno = 0;
            Reclen = 0;
            Type = (byte)FileTypes.DT_UNKNOWN;
            Namelen = 0;
            Name = new char[0];
        }

        public FileExplorerDent(BinaryReader p_Reader)
        {
            Deserialize(p_Reader);
        }

        public override void Deserialize(BinaryReader p_Reader)
        {
            Fileno = p_Reader.ReadUInt32();
            Reclen = p_Reader.ReadUInt16();
            Type = p_Reader.ReadByte();
            Namelen = p_Reader.ReadByte();
            Name = p_Reader.ReadChars(Namelen);
        }

        public override byte[] Serialize()
        {
            using (var s_Writer = new BinaryWriter(new MemoryStream()))
            {
                s_Writer.Write(Fileno);
                s_Writer.Write(Reclen);
                s_Writer.Write(Type);
                s_Writer.Write(Namelen);
                s_Writer.Write(Name);

                return ((MemoryStream)s_Writer.BaseStream).ToArray();
            }
        }
    }
    public class FileExplorerGetdentsRequest : MessageSerializable
    {
        public ushort Length;
        public char[] Path;

        public FileExplorerGetdentsRequest()
        {
            Length = 0;
            Path = new char[0];
        }

        public override void Deserialize(BinaryReader p_Reader)
        {
            Length = p_Reader.ReadUInt16();
            Path = p_Reader.ReadChars(Length);
        }

        public override byte[] Serialize()
        {
            using (var s_Writer = new BinaryWriter(new MemoryStream()))
            {
                s_Writer.Write(Length);
                s_Writer.Write(Path);

                return ((MemoryStream)s_Writer.BaseStream).ToArray();
            }
        }
    }

    public class FileExplorerGetdentsResponse : MessageSerializable
    {
        public ulong TotalDentCount;

        public FileExplorerGetdentsResponse()
        {
            TotalDentCount = 0;
        }

        public override void Deserialize(BinaryReader p_Reader)
        {
            TotalDentCount = p_Reader.ReadUInt64();
        }

        public override byte[] Serialize()
        {
            using (var s_Writer = new BinaryWriter(new MemoryStream()))
            {
                s_Writer.Write(TotalDentCount);

                return ((MemoryStream)s_Writer.BaseStream).ToArray();
            }
        }
    }
}
