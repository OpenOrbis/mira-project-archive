using System.IO;

namespace MiraUtils.Client
{
    public abstract class MessageSerializable
    {
        public abstract byte[] Serialize();

        public abstract void Deserialize(BinaryReader p_Reader);
    }
}
