using System.IO;

namespace MiraUtils.Client.FileExplorer
{
    public class FileExplorerUnlinkRequest : MessageSerializable
    {
        public ushort PathLength;
        public char[] Path;

        public override void Deserialize(BinaryReader p_Reader)
        {
            PathLength = p_Reader.ReadUInt16();
            Path = p_Reader.ReadChars(PathLength);
        }

        public override byte[] Serialize()
        {
            using (var s_Writer = new BinaryWriter(new MemoryStream()))
            {
                s_Writer.Write(PathLength);
                s_Writer.Write(Path);
                return ((MemoryStream)s_Writer.BaseStream).ToArray();
            }
        }
    }
}
