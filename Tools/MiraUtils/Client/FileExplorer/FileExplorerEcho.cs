using System.IO;

namespace MiraUtils.Client.FileExplorer
{
    public class FileExplorerEcho : MessageSerializable
    {
        public ushort Length;
        public char[] Message;

        public FileExplorerEcho()
        {
            Length = 0;
            Message = new char[0];
        }

        public FileExplorerEcho(string p_Message)
        {
            if (p_Message.Length > ushort.MaxValue)
            {
                Length = 0;
                Message = new char[0];
                return;
            }

            Message = p_Message.ToCharArray();
            Length = (ushort)Message.Length;
        }

        public override byte[] Serialize()
        {
            if (Message.Length > ushort.MaxValue)
            {
                Length = 0;
                Message = new char[0];
            }

            if (Length != Message.Length)
                Length = (ushort)Message.Length;

            using (var s_Writer = new BinaryWriter(new MemoryStream()))
            {
                s_Writer.Write(Length);
                s_Writer.Write(Message);

                return ((MemoryStream)s_Writer.BaseStream).ToArray();
            }
        }

        public override void Deserialize(BinaryReader p_Reader)
        {
            // TOOD: Implement
        }
    }
}
