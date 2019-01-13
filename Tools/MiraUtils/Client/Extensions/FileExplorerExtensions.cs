using Google.Protobuf;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace MiraUtils.Client.Extensions
{
    public static class FileExplorerExtensions
    {
        public delegate void OnDownloadProgressChanged(int p_Progress, bool p_Error);

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
                Payload = s_Request.ToByteString()
            };

            if (!p_Connection.SendMessage(s_Message))
                return -1;

            var s_ResponseMessage = p_Connection.ReceiveResponse();
            if (s_ResponseMessage == null)
                return -1;

            var s_Response = OpenResponse.Parser.ParseFrom(s_ResponseMessage.Payload);
            if (s_Response.Error < 0)
            {
                Console.WriteLine($"could not open {p_FileName} returned error {s_Response.Error}");
                return s_Response.Error;
            }

            return s_Response.Handle;
        }

        public static void Close(this PbConnection p_Connection, int p_Handle)
        {
            if (p_Connection == null)
                return;

            if (p_Handle < 0)
                return;

            var s_Message = new PbMessage
            {
                Category = MessageCategory.File,
                Type = (uint)FileTransferCommands.Close,
                Payload = new CloseRequest
                {
                    Handle = p_Handle
                }.ToByteString()
            };

            if (!p_Connection.SendMessage(s_Message))
                return;

            var s_ResponseMessage = p_Connection.ReceiveResponse();
            if (s_ResponseMessage == null)
                return;
        }

        public static byte[] Read(this PbConnection p_Connection, int p_Handle, ulong p_Offset, ulong p_Size)
        {
            if (p_Connection == null)
                return null;

            if (p_Handle < 0)
                return null;

            var s_Message = new PbMessage
            {
                Category = MessageCategory.File,
                Type = (uint)FileTransferCommands.Read,
                Payload = new ReadRequest
                {
                    Handle = p_Handle,
                    Offset  = p_Offset,
                    Size = p_Size
                }.ToByteString()
            };

            if (!p_Connection.SendMessage(s_Message))
                return null;

            var s_ResponseMessage = p_Connection.ReceiveResponse();
            if (s_ResponseMessage == null)
                return null;

            var s_Response = ReadResponse.Parser.ParseFrom(s_ResponseMessage.Payload);
            if (s_Response == null)
                return null;

            if (s_Response.Error < 0)
                return null;

            return s_Response.Data.ToByteArray();
        }

        public static StatResponse Stat(this PbConnection p_Connection, int p_Handle)
        {
           if (p_Connection == null)
                return null;

            if (p_Handle < 0)
                return null;

            var s_Message = new PbMessage
            {
                Category = MessageCategory.File,
                Type = (uint)FileTransferCommands.Stat,
                Payload = new StatRequest
                {
                    Handle = p_Handle,
                }.ToByteString()
            };

            if (!p_Connection.SendMessage(s_Message))
                return null;

            var s_ResponseMessage = p_Connection.ReceiveResponse();
            if (s_ResponseMessage == null)
                return null;

            var s_Response = StatResponse.Parser.ParseFrom(s_ResponseMessage.Payload);
            if (s_Response == null)
                return null;

            return s_Response;
        }

        public static StatResponse Stat(this PbConnection p_Connection, string p_Path)
        {
            if (p_Connection == null)
                return null;

            if (string.IsNullOrWhiteSpace(p_Path))
                return null;

            var s_Message = new PbMessage
            {
                Category = MessageCategory.File,
                Type = (uint)FileTransferCommands.Stat,
                Payload = new StatRequest
                {
                    Path = p_Path
                }.ToByteString()
            };

            if (!p_Connection.SendMessage(s_Message))
                return null;

            var s_ResponseMessage = p_Connection.ReceiveResponse();
            if (s_ResponseMessage == null)
                return null;

            var s_Response = StatResponse.Parser.ParseFrom(s_ResponseMessage.Payload);
            if (s_Response == null)
                return null;

            if (s_Response.Error < 0)
                return null;

            return s_Response;
        }

        public static byte[] DownloadFile(this PbConnection p_Connection, string p_Path, Action<int, bool> p_ProgressCallback = null)
        {
            if (p_Connection == null)
                return null;

            var s_Stat = Stat(p_Connection, p_Path);
            if (s_Stat == null)
                return null;

            var s_Handle = Open(p_Connection, p_Path, OpenFlags.O_RDONLY, 0);
            if (s_Handle < 0)
                return null;

            const long c_BlockSize = 0x2000;

            var s_CurrentPos = (ulong)0;
            var s_BlockCount = s_Stat.Size / c_BlockSize;
            var s_Leftover = s_Stat.Size % c_BlockSize;

            byte[] s_FinalData = null;
            using (var s_Writer = new BinaryWriter(new MemoryStream()))
            {
                for (long i = 0; i < s_BlockCount; ++i)
                {
                    var l_Data = Read(p_Connection, s_Handle, s_CurrentPos, c_BlockSize);
                    if (l_Data == null)
                    {
                        Close(p_Connection, s_Handle);
                        return null;
                    }

                    s_Writer.Write(l_Data);
                    s_CurrentPos += (ulong)l_Data.LongLength;

                    p_ProgressCallback?.Invoke((int)((s_CurrentPos / (double)s_Stat.Size) * 100.0), false);
                }

                var s_Data = Read(p_Connection, s_Handle, s_CurrentPos, (ulong)s_Leftover);
                s_Writer.Write(s_Data);

                p_ProgressCallback?.Invoke(100, false);
                s_FinalData = ((MemoryStream)s_Writer.BaseStream).ToArray();
            }

            Close(p_Connection, s_Handle);

            return s_FinalData;
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
                Payload = s_Request.ToByteString()
            };

            if (!p_Connection.SendMessage(s_Message))
                return false;

            var s_ResponseMessage = p_Connection.ReceiveResponse();
            if (s_ResponseMessage == null)
                return false;

            return true;
        }

        public static List<DirEnt> GetDirEnts(this PbConnection p_Connection, string p_Path)
        {
            var s_Entries = new List<DirEnt>();

            if (p_Connection == null)
                return s_Entries;

            if (string.IsNullOrWhiteSpace(p_Path))
                return s_Entries;

            var s_Request = new GetDentsRequest
            {
                Path = p_Path
            };

            var s_Message = new PbMessage
            {
                Category = MessageCategory.File,
                Type = (uint)FileTransferCommands.GetDents,
                Payload = s_Request.ToByteString()
            };

            if (!p_Connection.SendMessage(s_Message))
                return s_Entries;

            var s_ResposeMessage = p_Connection.ReceiveResponse();
            if (s_ResposeMessage == null)
                return s_Entries;
            
            var s_Response = GetDentsResponse.Parser.ParseFrom(s_ResposeMessage.Payload);
            if (s_Response == null)
                return s_Entries;

            return s_Response.Entries.ToList();
        }
    }
}
