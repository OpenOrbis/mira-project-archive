using System;
using System.IO;

namespace MiraUtils.Client.FileExplorer
{
    public class FileExplorerCloseRequest : MessageSerializable
    {
        public int Handle;

        public override void Deserialize(BinaryReader p_Reader)
        {
            Handle = p_Reader.ReadInt32();
        }

        public override byte[] Serialize()
        {
            return BitConverter.GetBytes(Handle);
        }
    }

    public class FileExplorerCloseResponse : MessageSerializable
    {
        public override void Deserialize(BinaryReader p_Reader)
        {
            
        }

        public override byte[] Serialize()
        {
            return new byte[0];
        }
    }
}
