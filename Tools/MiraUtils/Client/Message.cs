using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace MiraUtils.Client
{
    public class Message : MessageHeader
    {
        public List<byte> Payload;

        public Message(BinaryReader p_Reader)
        {
            Deserialize(p_Reader);
        }

        public Message(MessageCategory p_Category, uint p_Type, bool p_IsRequest, byte[] p_Payload)
        {
            Magic = c_Magic;
            Category = p_Category;
            IsRequest = p_IsRequest;
            Type = p_Type;

            if (p_Payload == null)
            {
                Payload = new List<byte>();
                PayloadLength = 0;
                return;
            }

            Payload = p_Payload.ToList();
            if (Payload.Count > ushort.MaxValue)
                throw new Exception("too much data in payload");

            PayloadLength = (ushort)Payload.Count;
        }

        public override byte[] Serialize()
        {
            using (var s_Writer = new BinaryWriter(new MemoryStream()))
            {
                // Write out the header information
                var s_HeaderData = base.Serialize();

                // Write out the payload
                s_Writer.Write(s_HeaderData);
                s_Writer.Write(Payload.ToArray());

                return ((MemoryStream)s_Writer.BaseStream).ToArray();
            }
        }

        public override void Deserialize(BinaryReader p_Reader)
        {
            // Deserialize the header
            base.Deserialize(p_Reader);

            // Deserialize the payload itself
            Payload = p_Reader.ReadBytes(PayloadLength).ToList();
        }
    }
}
