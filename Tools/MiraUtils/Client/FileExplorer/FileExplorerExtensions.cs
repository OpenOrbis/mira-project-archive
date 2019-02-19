using System;
using System.Collections.Generic;
using System.Linq;
using static MiraUtils.Client.MessageHeader;

namespace MiraUtils.Client.FileExplorer
{
    public static class FileExplorerExtensions
    {
        public enum OpenOnlyFlags
        {
            O_RDONLY = 0,
            O_WRONLY = 1,
            O_RDWR = 2,
            O_ACCMODE = 3
        }
        public static int Open(this MiraConnection p_Connection, string p_Path, int p_Flags, int p_Mode)
        {
            if (p_Connection == null)
                return -1;

            if (string.IsNullOrWhiteSpace(p_Path))
                return -1;

            var s_RequestMessage = new Message(
                MessageCategory.File,
                (uint)FileExplorerCommands.FileExplorer_Open,
                true,
                new FileExplorerOpenRequest
                {
                    PathLength = (ushort)p_Path.Length,
                    Path = p_Path.ToCharArray(),
                    Flags = p_Flags,
                    Mode = p_Mode
                }.Serialize());

            var (s_Response, s_Payload) = p_Connection.SendMessageWithResponse<FileExplorerOpenResponse>(s_RequestMessage);
            if (s_Response == null)
                return -1;

            return s_Response.Error;
        }

        public static void Close(this MiraConnection p_Connection, int p_Handle)
        {
            if (p_Connection == null)
                return;

            if (p_Handle < 0)
                return;

            var s_RequestMessage = new Message(
                MessageCategory.File,
                (uint)FileExplorerCommands.FileExplorer_Close,
                true,
                new FileExplorerCloseRequest
                {
                    Handle = p_Handle
                }.Serialize());

            var (s_Response, s_Payload) = p_Connection.SendMessageWithResponse<FileExplorerCloseResponse>(s_RequestMessage);
            if (s_Response == null)
                return;
        }

        public static byte[] Read(this MiraConnection p_Connection, int p_Handle, ulong p_Offset, int p_Count)
        {
            if (p_Connection == null)
                return null;

            if (p_Handle < 0)
                return null;

            var s_RequestMessage = new Message(
                MessageCategory.File,
                (uint)FileExplorerCommands.FileExplorer_Read,
                true,
                new FileExplorerReadRequest
                {
                    Handle = p_Handle,
                    Offset = p_Offset,
                    Count = p_Count
                }.Serialize());

            var (s_Response, s_Payload) = p_Connection.SendMessageWithResponse<FileExplorerReadResponse>(s_RequestMessage);
            if (s_Response == null)
                return null;

            if (s_Response.Error < 0)
                return null;

            return s_Payload?.Data;
        }

        public static bool Write(this MiraConnection p_Connection, int p_Handle, byte[] p_Data)
        {
            if (p_Connection == null)
                return false;

            if (p_Handle < 0)
                return false;

            var s_RequestMessage = new Message(
                MessageCategory.File,
                (uint)FileExplorerCommands.FileExplorer_Write,
                true,
                new FileExplorerWriteRequest
                {
                    Data = p_Data,
                    Count = p_Data.Length,
                    Handle = p_Handle
                }.Serialize());

            var (s_Response, s_Payload) = p_Connection.SendMessageWithResponse<FileExplorerWriteResponse>(s_RequestMessage);
            if (s_Response == null)
                return false;

            return s_Response.Error >= 0;
        }

        public static List<FileExplorerDent> GetDents(this MiraConnection p_Connection, string p_Path)
        {
            var s_DentList = new List<FileExplorerDent>();

            if (p_Connection == null)
                return s_DentList;

            if (string.IsNullOrWhiteSpace(p_Path))
                return s_DentList;

            var s_Request = new Message(
                MessageCategory.File,
                (uint)FileExplorerCommands.FileExplorer_GetDents,
                true,
                new FileExplorerGetdentsRequest
                {
                    Length = (ushort)p_Path.Length,
                    Path = p_Path.ToCharArray()
                }.Serialize());

            var (s_Response, s_Payload) = p_Connection.SendMessageWithResponse<FileExplorerGetdentsResponse>(s_Request);
            if (s_Response == null)
                return s_DentList;

            var s_DentCount = s_Payload.TotalDentCount;
            for (ulong i = 0; i < s_DentCount; ++i)
            {
                var s_Dent = p_Connection.ReadSerializable<FileExplorerDent>();
                s_DentList.Add(s_Dent);
            }

            return s_DentList;
        }

        public static FileExplorerStat Stat(this MiraConnection p_Connection, string p_Path)
        {
            if (p_Connection == null)
                return null;

            if (string.IsNullOrWhiteSpace(p_Path))
                return null;

            var s_Handle = p_Connection.Open(p_Path, (int)OpenOnlyFlags.O_RDONLY, 0777);
            if (s_Handle < 0)
                return null;

            var s_Stat = p_Connection.Stat(s_Handle);

            p_Connection.Close(s_Handle);

            return s_Stat;
        }

        public static FileExplorerStat Stat(this MiraConnection p_Connection, int p_Handle)
        {
            if (p_Connection == null)
                return null;

            if (p_Handle < 0)
                return null;

            var s_Request = new Message(
                MessageCategory.File,
                (uint)FileExplorerCommands.FileExplorer_Stat,
                true,
                new FileExplorerStatRequest
                {
                    Handle = p_Handle,
                    PathLength = 0,
                    Path = new char[0]
                }.Serialize());

            var (s_Response, s_Payload) = p_Connection.SendMessageWithResponse<FileExplorerStat>(s_Request);
            if (s_Response == null)
                return null;

            if (s_Response.Error < 0)
                return null;

            return s_Payload;
        }

        public static bool Echo(this MiraConnection p_Connection, string p_Message)
        {
            if (p_Connection == null)
                return false;

            if (string.IsNullOrWhiteSpace(p_Message))
                return false;

            if (p_Message.Length > ushort.MaxValue)
                return false;

            var s_Message = new Message(
                MessageCategory.File, 
                (uint)FileExplorerCommands.FileExplorer_Echo, 
                true,
                new FileExplorerEcho(p_Message).Serialize());

            return p_Connection.SendMessage(s_Message);
        }
    }
}
