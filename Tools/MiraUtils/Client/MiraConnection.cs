using System;
using System.IO;
using System.Net.Sockets;
using System.Text;

namespace MiraUtils.Client
{
    public class MiraConnection
    {
        private TcpClient m_Socket;
        private string m_NickName;

        private const string c_DefaultAddress = "127.0.0.1";
        private const ushort c_DefaultPort = 9999;

        /// <summary>
        /// Is the current connection active/connected
        /// </summary>
        public bool IsConnected => m_Socket?.Connected ?? false;

        /// <summary>
        /// Connection hostname or ip address
        /// </summary>
        public string Address { get; protected set; }

        /// <summary>
        /// Connection port number
        /// </summary>
        public ushort Port { get; protected set; }

        /// <summary>
        /// User settable nickname, or address:port
        /// </summary>
        public string Nickname
        {
            get { return string.IsNullOrWhiteSpace(m_NickName) ? $"{Address}:{Port}" : m_NickName; }
            set { m_NickName = value; }
        }

        /// <summary>
        /// Send and recv timeout for information, default 2s
        /// </summary>
        public int TimeoutInSeconds = 2;

        /// <summary>
        /// Maximum buffer size, this should optimally match the host
        /// </summary>
        public int MaxBufferSize = 0x8000;

        // Temporary buffer for holding our data
        private byte[] m_Buffer;

        /// <summary>
        /// Creates a new MiraConnection
        /// </summary>
        /// <param name="p_Address">Hostname or IP address of target</param>
        /// <param name="p_Port">Port to connect to, default(9999)</param>
        public MiraConnection(string p_Address = c_DefaultAddress, ushort p_Port = c_DefaultPort)
        {
            Address = p_Address;
            Port = p_Port;

            // Allocate some space for our buffer
            m_Buffer = new byte[MaxBufferSize];
        }

        /// <summary>
        /// Connects to the target
        /// </summary>
        /// <returns>True on success, false otherwise</returns>
        public bool Connect()
        {
            // If we are already connected disconnect first
            if (IsConnected)
                Disconnect();

            try
            {
                // Attempt to connect to the host
                m_Socket = new TcpClient(Address, Port)
                {
                    ReceiveTimeout = 1000 * TimeoutInSeconds,
                    SendTimeout = 1000 * TimeoutInSeconds,

                    SendBufferSize = MaxBufferSize,
                    ReceiveBufferSize = MaxBufferSize
                };
            }
            catch (Exception p_Exception)
            {
                Console.WriteLine($"exception: {p_Exception.InnerException}");
                return false;
            }

            return IsConnected;
        }

        /// <summary>
        /// Disconnects from the target
        /// </summary>
        public void Disconnect()
        {
            m_Socket?.Close();
        }

        /// <summary>
        /// Sends a message without expecting a response
        /// NOTE: If you incorrectly use this one instead of SendMessageWithResponse stuff may not work
        /// </summary>
        /// <param name="p_OutoingMessage">Outgoing message with all fields set and ready to go</param>
        /// <returns>True on success, false otherwise</returns>
        public bool SendMessage(Message p_OutoingMessage)
        {
            // Validate that we are connected
            if (!IsConnected)
                return false;

            // Write this, should call serialize which will have header + payload data
            using (var s_Writer = new BinaryWriter(m_Socket.GetStream(), Encoding.ASCII, true))
                s_Writer.Write(p_OutoingMessage.Serialize());

            return true;
        }

        /// <summary>
        /// Send a request and get a response type back
        /// </summary>
        /// <typeparam name="T">Type of expected response</typeparam>
        /// <param name="p_Message">Outoing message to send</param>
        /// <returns>Tuple (Message response, casted/deserialized response type), 
        /// (ResponseMessage, null) if no payload, 
        /// (null, null) on error</returns>
        public (Message, T) SendMessageWithResponse<T>(Message p_Message) where T : MessageSerializable, new()
        {
            // Send the request to the target
            if (!SendMessage(p_Message))
                return (null, null);

            // Read out the entire response + payload
            Message s_Message = null;
            using (var s_Reader = new BinaryReader(m_Socket.GetStream(), Encoding.ASCII, true))
                s_Message = new Message(s_Reader);

            // If there is no payload expected, then return the response message
            if (s_Message.PayloadLength == 0 || s_Message.Payload.Count == 0)
                return (s_Message, null);

            // Parse out the payload
            var s_PayloadType = new T();
            using (var s_Reader = new BinaryReader(new MemoryStream(s_Message.Payload.ToArray())))
                s_PayloadType.Deserialize(s_Reader);

            // Return both the message, and the parsed payload
            return (s_Message, s_PayloadType);
        }

        public T ReadSerializable<T>() where T : MessageSerializable, new()
        {
            var s_PayloadType = new T();
            using (var s_Reader = new BinaryReader(m_Socket.GetStream(), Encoding.ASCII, true))
                s_PayloadType.Deserialize(s_Reader);

            return s_PayloadType;
        }
    }
}
