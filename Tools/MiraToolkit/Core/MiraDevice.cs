using MiraLib;
using System.Collections.Generic;

namespace MiraToolkit.Core
{
    public class MiraDevice
    {
        // Open TTY consoles for /dev/console, /dev/deci_tty and whatever else
        private List<MiraConsole> m_Consoles;

        /// <summary>
        /// Hostname or IP address
        /// </summary>
        public string Hostname { get; protected set; }

        /// <summary>
        /// Mira RPC port
        /// </summary>
        public ushort Port { get; protected set; }

        /// <summary>
        /// Mira RPC Connection
        /// </summary>
        public MiraConnection Connection { get; protected set; }

        /// <summary>
        /// List of currently opened consoles
        /// </summary>
        public MiraConsole[] Consoles => m_Consoles?.ToArray();

        public MiraDevice(string p_Hostname, ushort p_Port)
        {
            Hostname = p_Hostname;
            Port = p_Port;

            Connection = new MiraConnection(Hostname, Port);

            m_Consoles = new List<MiraConsole>();
        }
        
        public void AddConsole(string p_DevicePath)
        {
            var s_Console = new MiraConsole(this, p_DevicePath);

            if (!s_Console.Open())
                return;

            m_Consoles.Add(s_Console);
        }
    }
}
