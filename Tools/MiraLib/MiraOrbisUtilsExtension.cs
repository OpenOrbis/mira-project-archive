using System.Runtime.InteropServices;


namespace MiraLib
{
    public static class MiraOrbisUtilsExtension
    {
        enum UtilityCmds : uint
        {
            OrbisUtils_DumpHddKeys = 0xA5020F62,
        };

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
                Encrypted = new byte[0x70],
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

    }
}
