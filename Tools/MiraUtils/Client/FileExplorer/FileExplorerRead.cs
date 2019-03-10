using System.IO;

namespace MiraUtils.Client.FileExplorer
{
    public class FileExplorerReadRequest : MessageSerializable
    {
        public int Handle;
        public ulong Offset;
        public int Count;

        public override void Deserialize(BinaryReader p_Reader)
        {
            Handle = p_Reader.ReadInt32();
            Offset = p_Reader.ReadUInt64();
            Count = p_Reader.ReadInt32();
        }

        public override byte[] Serialize()
        {
            using (var s_Writer = new BinaryWriter(new MemoryStream()))
            {
                s_Writer.Write(Handle);
                s_Writer.Write(Offset);
                s_Writer.Write(Count);

                return ((MemoryStream)s_Writer.BaseStream).ToArray();
            }
        }
    }

    public class FileExplorerReadResponse : MessageSerializable
    {
        public int Count;
        public byte[] Data;

        public override void Deserialize(BinaryReader p_Reader)
        {
            Count = p_Reader.ReadInt32();
            Data = p_Reader.ReadBytes(Count);
        }

        public override byte[] Serialize()
        {
            using (var s_Writer = new BinaryWriter(new MemoryStream()))
            {
                s_Writer.Write(Count);
                s_Writer.Write(Data);

                return ((MemoryStream)s_Writer.BaseStream).ToArray();
            }
        }
    }
}
