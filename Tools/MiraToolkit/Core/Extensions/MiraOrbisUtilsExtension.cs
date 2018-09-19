using MiraLib;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

namespace MiraToolkit.Core
{
    public static class MiraOrbisUtilsExtension
    {
        enum UtilityCmds : uint
        {
            UtilityCmd_DecryptExecutable = 0xD32F5831,
            OrbisUtils_DumpHddKeys = 0xA5020F62,
        };

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 0)]
        public struct DecryptExecutable
        {
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 255)]
            public string Path;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 0)]
        public struct DumpHddKeys
        {
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 0x70)]
            public byte[] Encrypted;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 0x20)]
            public byte[] Key;
        }

        public static bool GetHDDKeys(this MiraConnection p_Connection, out DumpHddKeys p_Output)
        {
            p_Output = new DumpHddKeys
            {
                Encrypted = new byte[0x60],
                Key = new byte[0x20]
            };

            if (!p_Connection.Connected)
                return false;

            p_Connection.SendMessage(new RpcMessageHeader
            {
                Category = (int)RPC_CATEGORY.RPCCAT_SYSTEM,
                Magic = MiraConnection.c_Magic,
                ErrorType = unchecked((int)UtilityCmds.OrbisUtils_DumpHddKeys),
                Request = true,
                PayloadSize = (ushort)Marshal.SizeOf<DumpHddKeys>()
            }, p_Connection.SerializeObject(p_Output));

            var s_Result = new RpcMessageHeader(p_Connection.ReceiveObject<ulong>());
            if (s_Result.ErrorType < 0)
                return false;

            new RpcMessageHeader(p_Connection.ReceiveObject<ulong>());
            p_Output = p_Connection.ReceiveObject<DumpHddKeys>();

            return true;
        }

        public static byte[] DecryptExecutableFile(this MiraConnection p_Connection, string p_Path)
        {
            if (p_Path.Length > 255)
                return null;

            p_Connection.SendMessage(new RpcMessageHeader
            {
                Category = (int)RPC_CATEGORY.RPCCAT_SYSTEM,
                PayloadSize = (ushort)Marshal.SizeOf<DecryptExecutable>(),
                Request = true,
                Magic = MiraConnection.c_Magic,
                ErrorType = unchecked((int)UtilityCmds.UtilityCmd_DecryptExecutable)
            }, new DecryptExecutable
            {
                Path = p_Path
            });

            byte[] s_Data;
            using (var s_Writer = new BinaryWriter(new MemoryStream()))
            {
                using (var s_Reader = new BinaryReader(p_Connection.GetStream(), Encoding.ASCII, true))
                {
                    var s_Offset = s_Reader.ReadUInt64();
                    var s_Length = s_Reader.ReadUInt64();
                    while (s_Length != 0)
                    {
                        s_Writer.Seek((int)s_Offset, SeekOrigin.Begin);
                        s_Writer.Write(s_Reader.ReadBytes((int)s_Length));
                        s_Offset = s_Reader.ReadUInt64();
                        s_Length = s_Reader.ReadUInt64();
                    }
                }

                s_Data = ((MemoryStream)s_Writer.BaseStream).ToArray();
            }

            return s_Data;
        }
    }
}
