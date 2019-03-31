using System;
using System.IO;
using System.Runtime.Serialization;

namespace MiraUtils.Client
{
    public class MessageHeader : MessageSerializable, ISerializable
    {
        public enum MessageCategory : byte
        {
            /// <summary>
            /// Message has no category, this is considered invalid
            /// </summary>
            None = 0,

            /// <summary>
            /// system messages
            /// </summary>
            System,

            /// <summary>
            /// Logging messages
            /// </summary>
            Log,

            /// <summary>
            /// Debugger messages
            /// </summary>
            Debug,

            /// <summary>
            /// File messages
            /// </summary>
            File,

            /// <summary>
            /// Command messages
            /// </summary>
            Command,

            /// <summary>
            /// Maximum amount of categories that are allowed, otherwise message struct needs changing
            /// </summary>
            Max = 14
        }

        private ulong m_Bits;

        public ulong Bits => m_Bits;

        public const int c_Magic = 0b10;
        private const string c_Bits = "bits";

        /*
 * 0        2           6           7                                                   23              55          64
 * | Magic  | Category  | IsRequest | (OnRequest: uint Type) / (OnResponse: int Error) | PayloadLength  | Reserved  |
 */

        // b:0 l:2
        public byte Magic
        {

            get
            {
                // Only the first 2 bits make up the magic
                return (byte)(m_Bits & 0x3);
            }
            set
            {
                if (value > 3 || value < 0)
                    value = 0;

                // Clear our lower 2 bits
                m_Bits = (m_Bits & 0xFFFFFFFFFFFFFFFC);

                // Set our bits
                m_Bits |= ((ulong)value & 0x3);
            }
        }

        // b:2 l:4
        public MessageCategory Category
        {
            get
            {
                return (MessageCategory)((m_Bits & 0x3C) >> 2);
            }
            set
            {
                var s_Category = value;
                if (s_Category < 0 || s_Category >= MessageCategory.Max)
                    s_Category = MessageCategory.None;

                m_Bits = (m_Bits & 0xFFFFFFFFFFFFFFC3);
                m_Bits |= ((ulong)s_Category << 2);
            }
        }

        // b:6 l:1
        public bool IsRequest
        {
            get
            {
                return ((m_Bits & 0x40) >> 6) == 1;
            }
            set
            {
                m_Bits = (m_Bits & 0xFFFFFFFFFFFFFFBF);
                m_Bits |= (ulong)(value ? 1 : 0) << 6;
            }
        }

        // b:7 l:32
        public int Error
        {
            get
            {
                return (int)((m_Bits & 0x7FFFFFFF80) >> 7);
            }
            set
            {
                m_Bits = (m_Bits & 0xFFFFFF800000007F);
                m_Bits |= (uint)((ulong)value << 7);
            }
        }

        // b:7 l:32
        public uint Type
        {
            get
            {
                return (uint)((m_Bits & 0x7FFFFFFF80) >> 7);
            }
            set
            {
                m_Bits = (m_Bits & 0xFFFFFF800000007F);
                m_Bits |= ((ulong)value << 7);
            }
        }

        // b:39 l:16
        public ushort PayloadLength
        {
            get
            {
                return (ushort)((m_Bits & 0x7FFF8000000000) >> 39);
            }
            set
            {
                m_Bits = (m_Bits & 0xFF80007FFFFFFFFF);
                m_Bits |= ((ulong)value << 39);
            }
        }

        public MessageHeader()
        {
            m_Bits = 0;
        }

        public MessageHeader(UInt64 p_Bits)
        {
            m_Bits = p_Bits;
        }

        public MessageHeader(BinaryReader p_Reader)
        {
            m_Bits = p_Reader.ReadUInt64();
        }

        public MessageHeader(SerializationInfo p_Info, StreamingContext p_Context)
        {
            m_Bits = (ulong)p_Info.GetValue(c_Bits, typeof(ulong));
        }

        public override string ToString()
        {
            return Convert.ToString((long)m_Bits, 2).PadLeft(64, '0');
        }

        public void GetObjectData(SerializationInfo info, StreamingContext context)
        {
            if (info == null)
                return;

            info.AddValue(c_Bits, m_Bits);
        }

        public override byte[] Serialize()
        {
            using (var s_Writer = new BinaryWriter(new MemoryStream()))
            {
                s_Writer.Write(m_Bits);

                return ((MemoryStream)s_Writer.BaseStream).ToArray();
            }
        }

        public override void Deserialize(BinaryReader p_Reader)
        {
            m_Bits = p_Reader.ReadUInt64();
        }
    }
}
