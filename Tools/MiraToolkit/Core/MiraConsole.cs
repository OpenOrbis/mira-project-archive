using MiraLib;
using System;
using System.Diagnostics;
using System.IO;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Threading;

namespace MiraToolkit.Core
{
    /// <summary>
    /// Representation of a TTY console
    /// </summary>
    public class MiraConsole
    {
        public delegate void AppendToLog(char p_Character);

        public event AppendToLog OnLogAppend;

        enum ConsoleCmds : uint
        {
            ConsoleCmd_Open = 0x2E8DE0C6,
            ConsoleCmd_Close = 0xB0377CD3
        };

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 0)]
        public struct ConsoleOpen
        {
            public int TTY;
            public ushort Port;
        }


        // Parent device
        private MiraDevice m_Device;

        private string m_TtyPath;

        // File descriptor that is open on Mira's end
        private int m_Descriptor;

        // Current open port
        private ushort m_Port;

        // Socket for reading the incoming buffer
        private Socket m_Socket;
        
        // Thread for listening
        private Thread m_Thread;

        /// <summary>
        /// Creates a new console which will record all output to file
        /// </summary>
        /// <param name="p_Device">Device that this console is attached to</param>
        /// <param name="p_OutputFilePath">Optional output file path if you want to write to file</param>
        public MiraConsole(MiraDevice p_Device, string p_TtyPath)
        {
            m_Device = p_Device ?? throw new ArgumentNullException(nameof(p_Device));

            if (string.IsNullOrWhiteSpace(p_TtyPath))
                throw new ArgumentException("invalid tty path");

            m_TtyPath = p_TtyPath;
        }

        public bool IsOpen()
        {
            if (m_Thread == null || m_Socket == null || m_Descriptor == -1)
                return false;

            if (!m_Thread.IsAlive)
                return false;

            if (!m_Socket.Connected)
                return false;

            return true;
        }

        public bool Open()
        {
            if (IsOpen())
                return true;

            var s_Connection = m_Device.Connection;

            // Open the handle to the tty we want, /dev/console, /dev/deci_tty7, etc
            m_Descriptor = s_Connection.Open(m_TtyPath, 0/*O_RDONLY*/, 0);
            if (m_Descriptor < 0)
            {
                Program.SetStatus($"could not open tty {m_TtyPath}", 0);
                return false;
            }

            // Send the RPC message
            s_Connection.SendMessage(new RpcMessageHeader
            {
                Category = (int)RPC_CATEGORY.RPCCAT_LOG,
                Magic = MiraConnection.c_Magic,
                ErrorType = (int)ConsoleCmds.ConsoleCmd_Open,
                Request = true,
                PayloadSize = (ushort)Marshal.SizeOf<ConsoleOpen>()
            }, new ConsoleOpen
            {
                Port = 0,
                TTY = m_Descriptor
            });

            var s_Header = new RpcMessageHeader(s_Connection.ReceiveObject<ulong>());
            if (s_Header.ErrorType != 0)
                return false;

            var s_OpenArgs = s_Connection.ReceiveObject<ConsoleOpen>();


            m_Port = s_OpenArgs.Port;
            if (m_Port <= 0)
                return false;

            m_Socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp)
            {
                ReceiveTimeout = 240
            };
            
            try
            {
                m_Socket.Connect(m_Device.Hostname, m_Port);
            }
            catch (SocketException p_Exception)
            {
                Console.WriteLine(p_Exception.Message);
                return false;
            }

            // Create a new listen thread
            m_Thread = new Thread(ConnectAndListen);
            m_Thread.Start();

            return true;
        }

        public void Close()
        {
            if (m_Thread.ThreadState == System.Threading.ThreadState.Running)
                m_Thread.Abort();

            m_Socket?.Close();
        }

        private void ConnectAndListen()
        {
            var s_Data = new byte[1];

            try
            {
                var s_DataReceiveLength = 0;

                while ((s_DataReceiveLength = m_Socket.Receive(s_Data, 1, SocketFlags.None)) > 0)
                    OnLogAppend?.Invoke((char)s_Data[0]);
            }
            catch (Exception p_Exception)
            {
                Debug.WriteLine(p_Exception.Message);
            }
        }

        public override string ToString()
        {
            return $"{m_TtyPath} : {m_Port} - {(IsOpen() ? "Connected" : "Disconnected")}";
        }
    }
}
