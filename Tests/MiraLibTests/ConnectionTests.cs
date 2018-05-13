using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using MiraLib;
using MiraLib.Extensions;

namespace MiraLibTests
{
    [TestClass]
    public class ConnectionTests
    {
        // Change this to the target IP address
        private const string c_Address = "192.168.1.2";

        // CHange this to the target port (default: 9999)
        private const ushort c_Port = 9999;

        private MiraConnection m_Connection;

        [TestInitialize]
        public void TestInitialization()
        {
            m_Connection = new MiraConnection(c_Address, c_Port);
        }

        [TestMethod]
        public void TestIsConnected()
        {
            // Verify the connection is valid
            Assert.IsNotNull(m_Connection);

            // Verify that the connection is actually connected
            Assert.IsTrue(m_Connection.Connected);
        }

        [TestMethod]
        public void TestFileTransfer()
        {
            // Verify the connection is valid
            Assert.IsNotNull(m_Connection);

            // Verify that the connection is actually connected
            Assert.IsTrue(m_Connection.Connected);

            var s_Flags = (int)MiraFileTransferExtensions.FileTransferFlags.O_RDONLY;

            var s_FileHandle = m_Connection.Open("/user", s_Flags, 0);
            Assert.AreNotEqual(-1, s_FileHandle);

            m_Connection.Close(s_FileHandle);
        }
    }
}
