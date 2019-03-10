using System.IO;

namespace MiraUtils.Client.OrbisUtils
{
    public class OrbisUtilsShutdownMiraRequest : MessageSerializable
    {
        public bool ShutdownMira;
        public bool RebootConsole;

        public override void Deserialize(BinaryReader p_Reader)
        {
            ShutdownMira = p_Reader.ReadByte() == 1;
            RebootConsole = p_Reader.ReadByte() == 1;
        }

        public override byte[] Serialize()
        {
            using (var s_Writer = new BinaryWriter(new MemoryStream()))
            {
                s_Writer.Write(ShutdownMira);
                s_Writer.Write(RebootConsole);

                return ((MemoryStream)s_Writer.BaseStream).ToArray();
            }
        }
    }
}
