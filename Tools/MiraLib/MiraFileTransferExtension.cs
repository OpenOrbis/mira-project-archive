using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace MiraLib.Extensions
{
    public static class MiraFileTransferExtensions
    {
        /*
         *    73 #define O_RDONLY        0x0000          /* open for reading only 
   74 #define O_WRONLY        0x0001          /* open for writing only 
   75 #define O_RDWR          0x0002          /* open for reading and writing 
         */

        public enum FileTransferFlags
        {
            O_RDONLY = 0x0000,
            O_WRONLY = 0x0001,
            O_RDWR = 0x0002,

            O_TRUNC     =    0x0400,
            O_CREAT      =   0x0200,
            O_APPEND     =   0x0008,
        }

        enum FileTransferCmds : uint
        {
            FileTransfer_Open = 0x58AFA0D4,
            FileTransfer_Close = 0x43F82FDB,
            FileTransfer_GetDents = 0x7433E67A,
            FileTransfer_Read = 0x64886217,
            FileTransfer_ReadFile = 0x13B55E44,
            FileTransfer_Write = 0x2D92D440,
            FileTransfer_WriteFile = 0x3B91E812,
            FileTransfer_Delete = 0xB74A88DC,
            FileTransfer_Move = 0x13E11408,
            FileTransfer_Stat = 0xDC67DC51,
            FileTransfer_COUNT
        };

        public enum FileTransferDirentType : int
        {
            Unknown = 0,
            NamedPipe = 1,
            CharacterDevice = 2,
            Directory = 4,
            BlockDevice = 8,
            Symlink = 10,
            Socket = 12,
            Wht = 14
        };

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 0)]
        public struct FileTransferOpen
        {
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 255)]
            public string Path;

            public int Flags;

            public int Mode;

            public int Handle;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 0)]
        public struct FileTransferReadFile
        {
            public int Handle;
            public ulong Size;
            // Payload
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 0)]
        public struct FileTransferWriteFile
        {
            public int Handle;
            public ulong Size;
            // Payload
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 0)]
        public struct FileTransferClose
        {
            public int Handle;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 0)]
        public struct FileTransferGetdents
        {
            public int Handle;
            public byte Type;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 255)]
            public string Path;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 0)]
        public struct FileTransferDelete
        {
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 255)]
            public string Path;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 0)]
        public struct FileTransferRead
        {
            public int Handle;
            public ulong Offset;
            public ulong Size;
            // Payload data gets sent afterwards
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 0)]
        public struct FileTransferStat
        {
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 255)]
            public string Path;
            public int Mode;
            public int Uid;
            public int Gid;
            public ulong Size;
        }

        public static int Open(this MiraConnection p_Connection, string p_Path, int p_Flags, int p_Mode)
        {
            if (!p_Connection.Connected)
                return -1;

            if (p_Path.Length > 255)
                return -1;

            p_Connection.SendMessage(new RpcMessageHeader
            {
                Magic = MiraConnection.c_Magic,
                Category = (int)RPC_CATEGORY.RPCCAT_FILE,
                Request = true,
                ErrorType = (int)FileTransferCmds.FileTransfer_Open,
                PayloadSize = (ushort)Marshal.SizeOf<FileTransferOpen>()
            }, p_Connection.SerializeObject(new FileTransferOpen
            {
                Flags = p_Flags,
                Handle = -1,
                Mode = p_Mode,
                Path = p_Path
            }));

            var s_Result = new RpcMessageHeader(p_Connection.ReceiveObject<ulong>());
            if (s_Result.ErrorType != 0)
                return -1;

            var s_Response = p_Connection.ReceiveObject<FileTransferOpen>();
            return s_Response.Handle;
        }

        public static void Close(this MiraConnection p_Connection, int p_Descriptor)
        {
            if (!p_Connection.Connected)
                return;

            if (p_Descriptor < 0)
                return;

            p_Connection.SendMessage(new RpcMessageHeader
            {
                Magic = MiraConnection.c_Magic,
                Category = (int)RPC_CATEGORY.RPCCAT_FILE,
                Request = true,
                ErrorType = (int)FileTransferCmds.FileTransfer_Close,
                PayloadSize = (ushort)Marshal.SizeOf<FileTransferClose>()
            }, p_Connection.SerializeObject(new FileTransferClose
            {
                Handle = p_Descriptor
            }));
        }

        public static byte[] ReadFile(this MiraConnection p_Connection, string p_Path)
        {
            if (!p_Connection.Connected)
                return null;

            int s_Descriptor = p_Connection.Open(p_Path, 0, 0);
            if (s_Descriptor < 0)
                return null;

            var s_HeaderData = p_Connection.SerializeObject(new RpcMessageHeader
            {
                Magic = MiraConnection.c_Magic,
                Category = (int)RPC_CATEGORY.RPCCAT_FILE,
                Request = true,
                ErrorType = (int)FileTransferCmds.FileTransfer_ReadFile,
                PayloadSize = (ushort)Marshal.SizeOf<FileTransferReadFile>()
            });

            var s_RequestData = p_Connection.SerializeObject(new FileTransferReadFile
            {
                Handle = s_Descriptor,
                Size = 0
            });

            using (var s_Writer = new BinaryWriter(p_Connection.m_Client.GetStream(), Encoding.ASCII, true))
            {
                s_Writer.Write(s_HeaderData);
                s_Writer.Write(s_RequestData);
            }

            var s_Response = p_Connection.ReceiveObject<FileTransferReadFile>();

            var s_BlockCount = s_Response.Size / 0x4000;
            var s_Leftover = s_Response.Size % 0x4000;
            var s_CurrentSize = 0;

            byte[] s_Data;
            using (var s_Reader = new BinaryReader(p_Connection.m_Client.GetStream(), Encoding.ASCII, true))
            {
                using (var s_Writer = new BinaryWriter(new MemoryStream()))
                {
                    for (ulong i = 0; i < s_BlockCount; ++i)
                    {
                        s_CurrentSize += 0x4000;
                        s_Writer.Write(s_Reader.ReadBytes(0x4000));
                    }


                    s_Writer.Write(s_Reader.ReadBytes((int)s_Leftover));

                    s_Data = ((MemoryStream)s_Writer.BaseStream).ToArray();
                }
            }

            // Close the handle
            p_Connection.Close(s_Descriptor);

            return s_Data;
        }

        public static void ReadFile(this MiraConnection p_Connection, string p_SourcePath, string p_LocalPath)
        {
            if (!p_Connection.Connected)
                return;

            int s_Descriptor = p_Connection.Open(p_SourcePath, 0, 0);
            if (s_Descriptor < 0)
                return;

            p_Connection.SendMessage(new RpcMessageHeader
            {
                Magic = MiraConnection.c_Magic,
                Category = (int)RPC_CATEGORY.RPCCAT_FILE,
                Request = true,
                ErrorType = (int)FileTransferCmds.FileTransfer_ReadFile,
                PayloadSize = (ushort)Marshal.SizeOf<FileTransferReadFile>()
            }, p_Connection.SerializeObject(new FileTransferReadFile
            {
                Handle = s_Descriptor,
                Size = 0
            }));

            var s_Result = new RpcMessageHeader(p_Connection.ReceiveObject<ulong>());
            if (s_Result.ErrorType != 0)
                return;

            var s_Response = p_Connection.ReceiveObject<FileTransferReadFile>();

            var s_BlockCount = s_Response.Size / 0x4000;
            var s_Leftover = s_Response.Size % 0x4000;

            using (var s_Reader = new BinaryReader(p_Connection.m_Client.GetStream(), Encoding.ASCII, true))
            {
                using (var s_Writer = new BinaryWriter(new FileStream(p_LocalPath, FileMode.Create, FileAccess.ReadWrite)))
                {
                    for (ulong i = 0; i < s_BlockCount; ++i)
                        s_Writer.Write(s_Reader.ReadBytes(0x4000));

                    s_Writer.Write(s_Reader.ReadBytes((int)s_Leftover));
                }
            }

            // Close the handle
            p_Connection.Close(s_Descriptor);
        }

        public static bool WriteFile(this MiraConnection p_Connection, string p_Path, byte[] p_Data)
        {
            if (!p_Connection.Connected)
                return false;

            var s_Descriptor = p_Connection.Open(p_Path, 0x0002 | 0x0200 /* O_RDWR | O_CREAT*/, 0777);
            if (s_Descriptor < 0)
                return false;

            p_Connection.SendMessage(new RpcMessageHeader
            {
                Magic = MiraConnection.c_Magic,
                Category = (int)RPC_CATEGORY.RPCCAT_FILE,
                Request = true,
                ErrorType = (int)FileTransferCmds.FileTransfer_WriteFile,
                PayloadSize = (ushort)Marshal.SizeOf<FileTransferWriteFile>()
            }, p_Connection.SerializeObject(new FileTransferWriteFile
            {
                Handle = s_Descriptor,
                Size = (ulong)p_Data.Length
            }));

            var s_BlockCount = p_Data.Length / 0x4000;
            var s_Leftover = p_Data.Length % 0x4000;

            using (var s_Reader = new BinaryReader(new MemoryStream(p_Data)))
            {
                using (var s_Writer = new BinaryWriter(p_Connection.m_Client.GetStream(), Encoding.ASCII, true))
                {
                    for (int i = 0; i < s_BlockCount; ++i)
                        s_Writer.Write(s_Reader.ReadBytes(0x4000));

                    s_Writer.Write(s_Reader.ReadBytes((int)s_Leftover));
                }
            }

            var s_Result = new RpcMessageHeader(p_Connection.ReceiveObject<ulong>());

            p_Connection.Close(s_Descriptor);

            return s_Result.ErrorType == 0;
        }

        public static byte[] Read(this MiraConnection p_Connection, int p_Descriptor, uint p_Offset, ulong p_Size)
        {
            if (!p_Connection.Connected)
                return null;

            if (p_Descriptor < 0)
                return null;

            p_Connection.SendMessage(new RpcMessageHeader
            {
                Magic = MiraConnection.c_Magic,
                Category = (int)RPC_CATEGORY.RPCCAT_FILE,
                Request = true,
                ErrorType = (int)FileTransferCmds.FileTransfer_Read,
                PayloadSize = (ushort)Marshal.SizeOf<FileTransferRead>(),
            }, p_Connection.SerializeObject(new FileTransferRead
            {
                Handle = p_Descriptor,
                Offset = p_Offset,
                Size = p_Size
            }));

            var s_Response = new RpcMessageHeader(p_Connection.ReceiveObject<ulong>());
            if (s_Response.ErrorType != 0)
                return null;

            var s_Payload = p_Connection.ReceiveObject<FileTransferRead>();

            var s_BlockCount = s_Payload.Size / 0x4000;
            var s_Leftover = s_Payload.Size % 0x4000;

            byte[] s_Data;
            using (var s_Reader = new BinaryReader(p_Connection.m_Client.GetStream(), Encoding.ASCII, true))
            {
                using (var s_Writer = new BinaryWriter(new MemoryStream()))
                {
                    for (ulong i = 0; i < s_BlockCount; ++i)
                        s_Writer.Write(s_Reader.ReadBytes(0x4000));

                    s_Writer.Write(s_Reader.ReadBytes((int)s_Leftover));
                    s_Data = ((MemoryStream)s_Writer.BaseStream).ToArray();
                }
            }

            return s_Data;
        }

        public static bool Delete(this MiraConnection p_Connection, string p_Path)
        {
            if (!p_Connection.Connected)
                return false;

            if (string.IsNullOrWhiteSpace(p_Path))
                return false;

            if (p_Path.Length > 255)
                return false;

            p_Connection.SendMessage(new RpcMessageHeader
            {
                Magic = MiraConnection.c_Magic,
                Category = (int)RPC_CATEGORY.RPCCAT_FILE,
                Request = true,
                ErrorType = unchecked((int)FileTransferCmds.FileTransfer_Delete),
                PayloadSize = (ushort)Marshal.SizeOf<FileTransferDelete>(),
            }, p_Connection.SerializeObject(new FileTransferDelete
            {
                Path = p_Path
            }));

            var s_Result = new RpcMessageHeader(p_Connection.ReceiveObject<ulong>());
            if (s_Result.ErrorType != 0)
                return false;

            return true;
        }

        public static IEnumerable<FileTransferGetdents> GetDents(this MiraConnection p_Connection, string p_Path)
        {
            if (!p_Connection.Connected)
                return null;

            // Send the request
            p_Connection.SendMessage(new RpcMessageHeader
            {
                Magic = MiraConnection.c_Magic,
                Category = (int)RPC_CATEGORY.RPCCAT_FILE,
                Request = true,
                ErrorType = (int)FileTransferCmds.FileTransfer_GetDents,
                PayloadSize = (ushort)Marshal.SizeOf<FileTransferGetdents>(),
            }, p_Connection.SerializeObject(new FileTransferGetdents
            {
                Path = p_Path,
                Handle = -1,
                Type = 0
            }));

            // Check the result
            var s_Result = new RpcMessageHeader(p_Connection.ReceiveObject<ulong>());
            if (s_Result.ErrorType != 0)
                return Enumerable.Empty<FileTransferGetdents>();

            // Read the amount of directory entries
            var s_List = new List<FileTransferGetdents>();
            ulong s_DentCount = 0;
            using (var s_Reader = new BinaryReader(p_Connection.m_Client.GetStream(), Encoding.ASCII, true))
                s_DentCount = s_Reader.ReadUInt64();

            Debug.WriteLine($"Dent Count: {s_DentCount}");

            for (ulong i = 0; i < s_DentCount; ++i)
                s_List.Add(p_Connection.ReceiveObject<FileTransferGetdents>());

            return s_List;
        }

        public static bool Stat(this MiraConnection p_Connection, string p_Path, out FileTransferStat p_Stat)
        {
            p_Stat = new FileTransferStat();

            if (!p_Connection.Connected)
                return false;

            if (p_Path.Length > 255)
                return false;

            p_Connection.SendMessage(new RpcMessageHeader
            {
                Magic = MiraConnection.c_Magic,
                Category = (int)RPC_CATEGORY.RPCCAT_FILE,
                Request = true,
                ErrorType = unchecked((int)FileTransferCmds.FileTransfer_Stat),
                PayloadSize = (ushort)Marshal.SizeOf<FileTransferStat>()
            }, p_Connection.SerializeObject(new FileTransferStat
            {
                Path = p_Path
            }));

            var s_Response = new RpcMessageHeader(p_Connection.ReceiveObject<ulong>());
            if (s_Response.ErrorType != 0)
                return false;

            p_Stat = p_Connection.ReceiveObject<FileTransferStat>();

            return true;
        }
    }
}
