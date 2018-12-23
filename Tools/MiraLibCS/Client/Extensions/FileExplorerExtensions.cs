using Google.Protobuf;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MiraLibCS.Client.Extensions
{
    public static class FileExplorerExtensions
    {
        public enum OpenFlags
        {
            O_RDONLY = 0x0000,
            O_WRONLY = 0x0001,
            O_RDWR = 0x0002,
        }
        public static int Open(this PbConnection p_Connection, string p_FileName, OpenFlags p_Flags, int p_Mode)
        {
            if (p_Connection == null)
                return -1;

            if (string.IsNullOrWhiteSpace(p_FileName))
                return -1;

            if (p_Flags < OpenFlags.O_RDONLY || p_Flags > OpenFlags.O_RDWR)
                return -1;

            // TODO: Finish full implemementation
            var s_Request = new OpenRequest
            {
                Path = p_FileName,
                Flags = (int)p_Flags,
                Mode = p_Mode
            };

            var s_Message = new PbMessage
            {
                Category = MessageCategory.File,
                Type = (uint)FileTransferCommands.Open,
                Data = s_Request.ToByteString()
            };

            if (!p_Connection.SendMessage(s_Message))
                return -1;

            var s_ResponseMessage = p_Connection.ReceiveResponse();
            if (s_ResponseMessage == null)
                return -1;

            var s_Response = OpenResponse.Parser.ParseFrom(s_ResponseMessage.Data);
            if (s_Response.Error < 0)
            {
                Console.WriteLine($"could not open {p_FileName} returned error {s_Response.Error}");
                return s_Response.Error;
            }

            return s_Response.Handle;
        }

        public static bool Echo(this PbConnection p_Connection, string p_Message)
        {
            if (p_Connection == null)
                return false;

            if (string.IsNullOrWhiteSpace(p_Message))
                return false;

            if (p_Message.Length > 2048)
                return false;

            var s_Request = new EchoRequest
            {
                Message = p_Message
            };

            var s_Message = new PbMessage
            {
                Category = MessageCategory.File,
                Type = (uint)FileTransferCommands.Echo,
                Data = s_Request.ToByteString()
            };

            if (!p_Connection.SendMessage(s_Message))
                return false;

            var s_ResponseMessage = p_Connection.ReceiveResponse();
            if (s_ResponseMessage == null)
                return false;

            return true;
        }
    }
}
