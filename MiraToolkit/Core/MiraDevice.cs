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


    }
}
