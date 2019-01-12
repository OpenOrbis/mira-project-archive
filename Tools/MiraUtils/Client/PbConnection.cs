using Google.Protobuf;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace MiraUtils.Client
{
    public class PbConnection
    {
        protected const int c_MaxBufferSize = 0x8000;
        protected const int c_DefaultPort = 9999;

        protected readonly string m_Address;
        protected readonly ushort m_Port;

        protected TcpClient m_Client;

        public bool IsConnected => m_Client?.Connected ?? false;
        public string Address => m_Address;

        public PbConnection(string p_Address, ushort p_Port = c_DefaultPort)
        {
            m_Address = p_Address;
            m_Port = p_Port;
        }

        /// <summary>
        /// Connect to a remote client
        /// </summary>
        /// <returns>True on success, false otherwise</returns>
        public bool Connect()
        {
            if (IsConnected)
                return true;

            try
            {
                m_Client = new TcpClient(m_Address, m_Port)
                {
                    SendTimeout = 1000 * 10,
                    ReceiveTimeout = 1000,

                    SendBufferSize = c_MaxBufferSize,
                    ReceiveBufferSize = c_MaxBufferSize
                };
            }
            catch (Exception p_Exception)
            {
                Console.WriteLine(p_Exception);
                return false;
            }

            return IsConnected;
        }

        public bool SendMessage(PbMessage p_Message)
        {
            if (!IsConnected)
                return false;

            if (p_Message == null)
                return false;

            var s_PacketData = p_Message.ToByteArray();

            using (var s_Writer = new BinaryWriter(m_Client.GetStream(), Encoding.ASCII, true))
            {
                s_Writer.Write((ulong)s_PacketData.LongLength);
                s_Writer.Write(s_PacketData);
            }

            return true;
        }

        public PbMessage ReceiveResponse()
        {
            if (!IsConnected)
                return null;

            PbMessage s_Message = null;
            using (var s_Reader = new BinaryReader(m_Client.GetStream(), Encoding.ASCII, true))
            {
                var s_Length = s_Reader.ReadUInt64();
                if (s_Length == 0 || s_Length > c_MaxBufferSize)
                {
                    Console.WriteLine($"length {s_Length} > max size {c_MaxBufferSize}.");
                    Disconnect();
                    return null;
                }

                var s_MessageData = s_Reader.ReadBytes((int)s_Length);
                if (s_MessageData == null)
                {
                    Console.WriteLine($"message data is null.");
                    Disconnect();
                    return null;
                }

                s_Message = PbMessage.Parser.ParseFrom(s_MessageData);
            }

            return s_Message;
        }

        public void Disconnect()
        {
            // TODO: Disconnect
            throw new NotImplementedException();
        }

        public static PbMessage CreateMessage(MessageCategory p_Category, uint p_Type, ByteString p_RequestBytes)
        {
            if (p_Category < MessageCategory.None || p_Category > MessageCategory.Max)
                return null;

            if (p_RequestBytes == null)
                return null;

            var s_Message = new PbMessage
            {
                Category = p_Category,
                Type = p_Type,
                Payload = p_RequestBytes,
            };

            return s_Message;
        }

        public bool CreateAndSend(MessageCategory p_Category, uint p_Type, ByteString p_RequestBytes)
        {
            return SendMessage(CreateMessage(p_Category, p_Type, p_RequestBytes));
        }
    }
}
