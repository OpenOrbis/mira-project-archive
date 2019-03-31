using System.IO;

namespace MiraUtils.Client
{
    public class TimeSpec : MessageSerializable
    {
        public long Seconds;
        public long NanoSeconds;

        public TimeSpec()
        {
            Seconds = 0;
            NanoSeconds = 0;
        }

        public TimeSpec(BinaryReader p_Reader)
        {
            Deserialize(p_Reader);
        }

        public override void Deserialize(BinaryReader p_Reader)
        {
            Seconds = p_Reader.ReadInt64();
            NanoSeconds = p_Reader.ReadInt64();
        }

        public override byte[] Serialize()
        {
            using (var s_Writer = new BinaryWriter(new MemoryStream()))
            {
                s_Writer.Write(Seconds);
                s_Writer.Write(NanoSeconds);

                return ((MemoryStream)s_Writer.BaseStream).ToArray();
            }
        }
    }
}
