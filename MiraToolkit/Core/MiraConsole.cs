using System;
using System.Diagnostics;
using System.IO;
using System.Net.Sockets;
using System.Threading;

namespace MiraToolkit.Core
{
    /// <summary>
    /// Representation of a TTY console
    /// </summary>
    public class MiraConsole
    {
        // Parent device
        private MiraDevice m_Parent;

        // File name
        private string m_OutputFileName;

        // The output file stream that gets written to disk
        private TextWriter m_Writer;

        // Current open port
        private ushort m_Port;

        // Socket for reading the incoming buffer
        private Socket m_Socket;

        // Is the socket currently open/listening
        private bool m_IsOpen;
        
        // Thread for listening
        private Thread m_Thread;

        /// <summary>
        /// Device path, optional
        /// </summary>
        public string Path { get; set; }

        /// <summary>
        /// Creates a new console which will record all output to file
        /// </summary>
        /// <param name="p_Device">Device that this console is attached to</param>
        /// <param name="p_Port">Port to connect to</param>
        /// <param name="p_OutputFilePath">Optional output file path if you want to write to file</param>
        public MiraConsole(MiraDevice p_Device, ushort p_Port, string p_OutputFilePath = "")
        {
            if (p_Device == null)
                throw new ArgumentNullException(nameof(p_Device));

            if (p_Port < 1024)
                throw new ArgumentOutOfRangeException(nameof(p_Port));

            // Assign the port
            m_Port = p_Port;

            // If the output file path is empty, we do not enable logging to file
            if (string.IsNullOrWhiteSpace(p_OutputFilePath))
                return;

            // Assign the output file name
            m_OutputFileName = p_OutputFilePath;
        }

        public bool Open()
        {
            if (m_IsOpen)
                Close();

            m_Socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp)
            {
                ReceiveTimeout = 240
            };

            try
            {
                m_Socket.Connect(m_Parent.Hostname, m_Port);
            }
            catch (SocketException p_Exception)
            {
                Console.WriteLine(p_Exception.Message);
                return false;
            }

            // Create a new output file writer
            if (!string.IsNullOrWhiteSpace(m_OutputFileName))
            {
                // Create a new file writer
                m_Writer = new StreamWriter(new FileStream(m_OutputFileName, FileMode.Create, FileAccess.Write));
            }

            // Create a new listen thread
            m_Thread = new Thread(ConnectAndListen);
            m_Thread.Start();

            m_IsOpen = true;
            return true;
        }

        public void Close()
        {
            if (m_Thread.ThreadState == System.Threading.ThreadState.Running)
                m_Thread.Abort();

            m_Socket?.Close();

            m_Writer?.Flush();
            m_Writer?.Close();

            m_IsOpen = false;
        }

        private void ConnectAndListen()
        {
            var s_Data = new byte[1];

            try
            {
                var s_DataReceiveLength = 0;

                while ((s_DataReceiveLength = m_Socket.Receive(s_Data, 1, SocketFlags.None)) > 0)
                    m_Writer.Write(s_Data);
            }
            catch (Exception p_Exception)
            {
                Debug.WriteLine(p_Exception.Message);
            }
        }

        public override string ToString()
        {
            return $"{Path} : {m_Port} - {(m_IsOpen ? "Connected" : "Disconnected")}";
        }
    }
}
