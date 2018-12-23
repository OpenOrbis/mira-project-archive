using Google.Protobuf;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MiraLibCS
{
    class Program
    {
        static void Main(string[] args)
        {
            var s_Address = "192.168.1.2";

            var s_Connection = new PbConnection(s_Address);

            if (!s_Connection.Connect())
            {
                Console.WriteLine($"could not connect to {s_Address}");
                return;
            }

            var s_RequestMessage = PbConnection.CreateMessage(MessageCategory.File, (uint)FileTransferCommands.Echo, new EchoRequest
            {
                Message = "Hello World",
            }.ToByteString());

            if (!s_Connection.SendMessage(s_RequestMessage))
            {
                Console.WriteLine("could not send message.");
                return;
            }

            var s_Response = s_Connection.ReceiveResponse();
        }
    }
}
