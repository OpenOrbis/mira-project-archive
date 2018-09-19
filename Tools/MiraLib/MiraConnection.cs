using System;
using System.IO;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Text;

namespace MiraLib
{
    /// <summary>
    /// Categories, these MUST MATCH THE C IMPLEMENTATION
    /// </summary>
    public enum RPC_CATEGORY
    {
        /// <summary>
        /// Nothing
        /// </summary>
        RPCCAT_NONE = 0,

        /// <summary>
        /// System category
        /// </summary>
        RPCCAT_SYSTEM,

        /// <summary>
        /// Logger category
        /// </summary>
        RPCCAT_LOG,

        /// <summary>
        /// Debugger category
        /// </summary>
        RPCCAT_DBG,

        /// <summary>
        /// File category
        /// </summary>
        RPCCAT_FILE,

        /// <summary>
        /// Command category
        /// </summary>
        RPCCAT_CMD,

        /// <summary>
        /// Category count
        /// </summary>
        RPCCAT_COUNT
    };

    public class RpcMessageHeader
    {
        private ulong m_Bits;

        public byte Magic
        {
            get
            {
                return (byte)(m_Bits & 0x7);
            }
            set
            {
                var s_Bits = (m_Bits & ~(ulong)0x7);
                m_Bits = s_Bits | (value & (ulong)0x7);
            }
        }

        public byte Category
        {
            get
            {
                return (byte)(m_Bits & (0xF << 3));
            }
            set
            {
                const ulong V = 0xF;
                var s_Bits = (m_Bits & (~(V << 3)));
                m_Bits = s_Bits | (((ulong)value & V) << 3);
            }
        }

        public int ErrorType
        {
            get
            {
                return (int)(m_Bits & (0xFFFFFFFF << 0x7));
            }
            set
            {
                const ulong V = 0xFFFFFFFF;
                var s_Bits = (m_Bits & (~(V << 0x7)));
                m_Bits = s_Bits | (((ulong)value & V) << 0x7);
            }
        }

        public ushort PayloadSize
        {
            get
            {
                return (ushort)(m_Bits & (0xFFFF << 0x27));
            }
            set
            {
                const ulong V = 0xFFFF;
                var s_Bits = (m_Bits & (~(V << 0x27)));
                m_Bits = s_Bits | (((ulong)value & V) << 0x27);
            }
        }

        public bool Request
        {
            get
            {
                return ((m_Bits & (0x1 << 0x37)) != 0);
            }
            set
            {
                const ulong V = 0x1;
                var s_Bits = (m_Bits & (~(V << 0x37)));
                m_Bits = s_Bits | (((ulong)(value ? 1 : 0) & V) << 0x37);
            }
        }

        public byte[] Serialize()
        {
            return BitConverter.GetBytes(m_Bits);
        }

        public ulong ToUInt64()
        {
            return m_Bits;
        }

        public RpcMessageHeader()
        {
            Magic = MiraConnection.c_Magic;
        }

        public RpcMessageHeader(ulong p_Bits)
        {
            m_Bits = p_Bits;
        }
    }

    /// <summary>
    /// The bread and butter
    /// </summary>
    public class MiraConnection
    {
        // Header magic, THIS MUST MATCH C IMPLEMENTATION
        public const byte c_Magic = 0x5;

        // Buffer size, THIS MUST MATCH C IMPLEMENTATION
        public const int c_BufferSize = 0x4000;

        // IP address
        protected readonly string m_Address;

        // Server port
        protected readonly ushort m_Port;

        // Client
        internal TcpClient m_Client;

        /// <summary>
        /// Returns if the client is connected
        /// </summary>
        public bool Connected => m_Client?.Connected ?? false;

        /// <summary>
        /// IP Address
        /// </summary>
        public string Address => m_Address;

        /// <summary>
        /// Creates a new connection instance
        /// </summary>
        /// <param name="p_IpAddress">Ip address of the server</param>
        /// <param name="p_Port">Port to connect to</param>
        public MiraConnection(string p_IpAddress, ushort p_Port)
        {
            m_Address = p_IpAddress;
            m_Port = p_Port;
        }

        /// <summary>
        /// Connect to a remote client
        /// </summary>
        /// <returns>True on success, false otherwise</returns>
        public bool Connect()
        {
            try
            {
                m_Client = new TcpClient(m_Address, m_Port)
                {
                    // Large file transfers will stall if this is set to low
                    SendTimeout = 1000 * 10,
                    ReceiveTimeout = 1000 * 10,

                    // These must match the c implementation
                    SendBufferSize = c_BufferSize,
                    ReceiveBufferSize = c_BufferSize
                };
            }
            catch (Exception p_Exception)
            {
                Console.WriteLine(p_Exception);
                return false;
            }

            return m_Client.Connected;
        }

        /// <summary>
        /// Disconnects the client
        /// </summary>
        public void Disconnect()
        {
            try
            {
                m_Client.Close();
            }
            catch (Exception p_Exception)
            {
                Console.WriteLine(p_Exception);
            }
        }

        /// <summary>
        /// Serializes an object into a byte array
        /// </summary>
        /// <param name="p_Object">Structure to serialize</param>
        /// <returns>Byte array of object</returns>
        public byte[] SerializeObject(object p_Object)
        {
            var s_Size = Marshal.SizeOf(p_Object);

            var s_Data = new byte[s_Size];

            var s_Ptr = Marshal.AllocHGlobal(s_Size);

            Marshal.StructureToPtr(p_Object, s_Ptr, true);

            Marshal.Copy(s_Ptr, s_Data, 0, s_Size);

            Marshal.FreeHGlobal(s_Ptr);

            return s_Data;
        }

        /// <summary>
        /// Deserializes a object to the specified structure
        /// </summary>
        /// <typeparam name="T">Structure to deserialize</typeparam>
        /// <param name="p_Data">Incoming data to deserialize from</param>
        /// <returns>Object or null</returns>
        public T DeserializeObject<T>(byte[] p_Data)
        {
            var s_Size = Marshal.SizeOf<T>();
            if (p_Data.Length < s_Size)
                throw new InvalidDataException($"Data size is too small to deserialize {typeof(T).FullName}");

            var s_Ptr = Marshal.AllocHGlobal(s_Size);
            Marshal.Copy(p_Data, 0, s_Ptr, s_Size);
            var s_Object = (T)Marshal.PtrToStructure(s_Ptr, typeof(T));
            Marshal.FreeHGlobal(s_Ptr);

            return s_Object;
        }

        /// <summary>
        /// Receive's an object from over the network, will not accept data afterwards
        /// </summary>
        /// <typeparam name="T">Structure type to download</typeparam>
        /// <returns>Object created</returns>
        public T ReceiveObject<T>()
        {
            var s_Size = Marshal.SizeOf<T>();

            byte[] s_Data;
            using (var s_Reader = new BinaryReader(m_Client.GetStream(), Encoding.ASCII, true))
                s_Data = s_Reader.ReadBytes(s_Size);

            if (s_Data.Length < s_Size)
                throw new InvalidDataException("incoming data length < required size");

            var s_Object = DeserializeObject<T>(s_Data);

            return s_Object;
        }

        /// <summary>
        /// Sends a structure to the server
        /// </summary>
        /// <typeparam name="T">Type of structure</typeparam>
        /// <param name="p_Object">Structure to send</param>
        public void SendMessage<T>(T p_Object)
        {
            var s_Data = SerializeObject(p_Object);

            using (var s_Writer = new BinaryWriter(m_Client.GetStream(), Encoding.ASCII, true))
                s_Writer.Write(s_Data);
        }

        /// <summary>
        /// Sends a message with the supplied header, and payload
        /// </summary>
        /// <param name="p_Object">Header</param>
        /// <param name="p_Payload">Payload data</param>
        public void SendMessage(RpcMessageHeader p_Object, byte[] p_Payload)
        {
            var s_Bits = p_Object.ToUInt64();
            var s_HeaderData = SerializeObject(s_Bits);

            using (var s_Writer = new BinaryWriter(m_Client.GetStream(), Encoding.ASCII, true))
            {
                s_Writer.Write(s_HeaderData);
                s_Writer.Write(p_Payload);
            }
        }

        /// <summary>
        /// Sends a message with a structure payload
        /// </summary>
        /// <typeparam name="T">Type of payload</typeparam>
        /// <param name="p_Header">Header</param>
        /// <param name="p_Payload">Structure to send</param>
        public void SendMessage<T>(RpcMessageHeader p_Header, T p_Payload)
        {
            var s_Bits = p_Header.ToUInt64();
            var s_HeaderData = SerializeObject(s_Bits);
            var s_PayloadData = SerializeObject(p_Payload);

            using (var s_Writer = new BinaryWriter(m_Client.GetStream(), Encoding.ASCII, true))
            {
                s_Writer.Write(s_HeaderData);
                s_Writer.Write(s_PayloadData);
            }
        }

        public (RpcMessageHeader, byte[]) ReceiveHeaderAndPayload()
        {
            using (var s_Reader = new BinaryReader(m_Client.GetStream(), Encoding.ASCII, true))
            {

                var s_Header = new RpcMessageHeader(s_Reader.ReadUInt64());

                var s_PayloadSize = (ushort)(s_Header.PayloadSize > c_BufferSize ? c_BufferSize : s_Header.PayloadSize);

                var s_Payload = s_Reader.ReadBytes(s_PayloadSize);

                return (s_Header, s_Payload);
            }
        }

        public (RpcMessageHeader, T) ReceiveHeaderAndObject<T>()
        {
            using (var s_Reader = new BinaryReader(m_Client.GetStream(), Encoding.ASCII, true))
            {

                var s_Header = new RpcMessageHeader(s_Reader.ReadUInt64());

                var s_PayloadSize = (ushort)(s_Header.PayloadSize > c_BufferSize ? c_BufferSize : s_Header.PayloadSize);

                var s_Payload = s_Reader.ReadBytes(s_PayloadSize);

                var s_Object = DeserializeObject<T>(s_Payload);

                return (s_Header, s_Object);
            }
        }

        public NetworkStream GetStream()
        {
            return m_Client.GetStream();
        }
    }
}
