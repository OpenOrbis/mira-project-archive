using System;
using System.IO;
using System.Runtime.Serialization;

namespace MiraRpcGenerator.Messages
{
    public class MessageHeader : ISerializable
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
        private UInt64 Bits;

        public const int c_Magic = 0b10;
        private const string c_Bits = "bits";

        /*
 * 0        2           6           7               23                                                  55          64
 * | Magic  | Category  | IsRequest | PayloadLength | (OnRequest: uint Type) / (OnResponse: int Error)  | Reserved  |
 */

        // b:0 l:2
        public byte Magic
        {
            
            get
            {
                // Only the first 2 bits make up the magic
                return (byte)(Bits & 0x3);
            }
            set
            {
                if (value > 3 || value < 0)
                    value = 0;

                // Clear our lower 2 bits
                Bits = (Bits & 0xFFFFFFFFFFFFFFFC);

                // Set our bits
                Bits |= ((ulong)value & 0x3);
            }
        }

        // b:2 l:4
        public MessageCategory Category
        {
            get
            {
                return (MessageCategory)((Bits & 0x3C) >> 2);
            }
            set
            {
                var s_Category = value;
                if (s_Category < 0 || s_Category >= MessageCategory.Max)
                    s_Category = MessageCategory.None;

                Bits = (Bits & 0xFFFFFFFFFFFFFFC3);
                Bits |= ((ulong)s_Category << 2);
            }
        }

        // b:6 l:1
        public bool IsRequest
        {
            get
            {
                return ((Bits & 0x40) >> 6) == 1;
            }
            set
            {
                Bits = (Bits & 0xFFFFFFFFFFFFFFBF);
                Bits |= (ulong)(value ? 1 : 0) << 6;
            }
        }

        // b:7 l:32
        public int Error
        {
            get
            {
                return (int)((Bits & 0x7FFFFFFF80) >> 7);
            }
            set
            {
                Bits = (Bits & 0xFFFFFF800000007F);
                Bits |= (uint)(value << 7);
            }
        }

        // b:7 l:32
        public uint Type
        {
            get
            {
                return (uint)((Bits & 0x7FFFFFFF80) >> 7);
            }
            set
            {
                Bits = (Bits & 0xFFFFFF800000007F);
                Bits |= (uint)(value << 7);
            }
        }
        
        // b:39 l:16
        public ushort PayloadLength
        {
            get
            {
                return (ushort)((Bits & 0x7FFF8000000000) >> 39);
            }
            set
            {
                Bits = (Bits & 0xFF80007FFFFFFFFF);
                Bits |= (ulong)value << 39;
            }
        }

        public MessageHeader()
        {
            Bits = 0;
        }

        public MessageHeader(UInt64 p_Bits)
        {
            Bits = p_Bits;
        }

        public MessageHeader(BinaryReader p_Reader)
        {
            Bits = p_Reader.ReadUInt64();
        }

        public MessageHeader(SerializationInfo p_Info, StreamingContext p_Context)
        {
            Bits = (ulong)p_Info.GetValue(c_Bits, typeof(ulong));
        }

        public override string ToString()
        {
            return Convert.ToString((long)Bits, 2).PadLeft(64, '0');
        }

        public void GetObjectData(SerializationInfo info, StreamingContext context)
        {
            if (info == null)
                return;

            info.AddValue(c_Bits, Bits);
        }
    }
}
