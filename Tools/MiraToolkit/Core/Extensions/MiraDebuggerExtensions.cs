using MiraLib;
using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

namespace OniPC.Rpc
{
    public static class MiraDebuggerExtensions
    {
        enum DebuggerCmds : uint
        {
            Debugger_GetProcs = 0x2644A6F6,
            Debugger_ReadMem = 0x42338928,
            Debugger_WriteMem = 0x29DDCA51,
            Debugger_SetBp = 0x768A6CC0,
            Debugger_RemoveBp = 0x82D4B365,
            Debugger_Ptrace = 0xD15318ED,
            Debugger_Kill = 0x9B3D3394,
            Debugger_COUNT
        };

        public enum PtraceRequests
        {
            PT_TRACE_ME = 0,
            PT_READ_I = 1,
            PT_READ_D = 2,
            PT_WRITE_I = 4,
            PT_WRITE_D = 5,
            PT_CONTINUE = 7,
            PT_KILL = 8,
            PT_STEP = 9,
            PT_ATTACH = 10,
            PT_DETACH = 11,
            PT_IO = 12,
            PT_LWPINFO = 13,
            PT_GETNUMLWPS = 14,
            PT_GETLWPLIST = 15,
            PT_CLEARSTEP = 16,
            PT_SETSTEP = 17,
            PT_SUSPEND = 18,
            PT_RESUME = 19,
            PT_TO_SCE = 20,
            PT_TO_SCX = 21,
            PT_SYSCALL = 22,
            PT_FOLLOW_FORK = 23,
            PT_GETREGS = 33,
            PT_SETREGS = 34,
            PT_GETFPREGS = 35,
            PT_SETFPREGS = 36,
            PT_GETDBREGS = 37,
            PT_SETDBREGS = 38,
            PT_VM_TIMESTAMP = 40,
            PT_VM_ENTRY = 41,
            PT_FIRSTMACH = 64
        }

        public enum SystemSignals
        {
            SIGHUP = 1,
            SIGINT = 2,
            SIGQUIT = 3,
            SIGILL = 4,
            // TODO: Complete this
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 0)]
        public struct ProcInfo
        {
            public int ProcessId;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 1024)]
            public string Path;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
            public string ProcessName;
            public ulong TextAddress;
            public ulong TextSize;
            public ulong DataAddress;
            public ulong DataSize;
            public ulong VirtualSize;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 0)]
        public struct ReadWriteMem
        {
            public int ProcessId;
            public ulong Address;
            public ulong DataLength;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 0x1000)]
            public byte[] Data;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 0)]
        public struct DebuggerPtrace
        {
            public int Result;
            public int Request;
            public int ProcessId;
            public ulong Address;
            public int Data;
            public int SetAddressToBuffer;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 0x800)]
            public byte[] Buffer;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 0)]
        public struct DebuggerKill
        {
            public int Pid;
            public int Signal;
        }

        public static int Kill(this MiraConnection p_Connection, int p_Pid, int p_Signal)
        {
            if (!p_Connection.Connected)
                return -1;

            p_Connection.SendMessage(new RpcMessageHeader
            {
                Category = (int)RPC_CATEGORY.RPCCAT_DBG,
                Magic = MiraConnection.c_Magic,
                ErrorType = unchecked((int)DebuggerCmds.Debugger_Kill),
                Request = true,
                PayloadSize = (ushort)Marshal.SizeOf<DebuggerKill>()
            }, p_Connection.SerializeObject(new DebuggerKill
            {
                Pid = p_Pid,
                Signal = p_Signal
            }));

            var s_Result = new RpcMessageHeader(p_Connection.ReceiveObject<ulong>());

            return (int)s_Result.ErrorType;
        }

        public static DebuggerPtrace Ptrace(this MiraConnection p_Connection, int p_Request, int p_ProcessId, ulong p_Address, int p_Data, bool p_UseInternalBuffer = false, byte[] p_BufferData = null)
        {
            var s_Data = new byte[0x800];

            if (p_BufferData != null && p_BufferData.Length <= s_Data.Length)
                Buffer.BlockCopy(p_BufferData, 0, s_Data, 0, p_BufferData.Length);

            p_Connection.SendMessage(new RpcMessageHeader
            {
                Category = (int)RPC_CATEGORY.RPCCAT_DBG,
                Magic = MiraConnection.c_Magic,
                ErrorType = unchecked((int)DebuggerCmds.Debugger_Ptrace),
                Request = true,
                PayloadSize = (ushort)Marshal.SizeOf<DebuggerPtrace>()
            }, p_Connection.SerializeObject(new DebuggerPtrace
            {
                Address = p_Address,
                Buffer = s_Data,
                Data = p_Data,
                ProcessId = p_ProcessId,
                Request = p_Request,
                SetAddressToBuffer = p_UseInternalBuffer ? 1 : 0
            }));

            var s_Response = p_Connection.ReceiveObject<DebuggerPtrace>();

            return s_Response;
        }

        public static List<ProcInfo> GetProcessList(this MiraConnection p_Connection)
        {
            p_Connection.SendMessage(new RpcMessageHeader
            {
                Category = (int)RPC_CATEGORY.RPCCAT_DBG,
                Magic = MiraConnection.c_Magic,
                ErrorType = (int)DebuggerCmds.Debugger_GetProcs,
                Request = true,
                PayloadSize = 0
            }.ToUInt64());

            var s_Procs = new List<ProcInfo>();
            using (var s_Reader = new BinaryReader(p_Connection.GetStream(), Encoding.ASCII, true))
            {
                while (true)
                {
                    var s_Proc = p_Connection.ReceiveObject<ProcInfo>();
                    // We can check any of the variables for the end finalizer
                    if ((uint)s_Proc.ProcessId == 0xDDDDDDDD)
                        break;

                    s_Procs.Add(s_Proc);
                }
            }

            return s_Procs;
        }

        public static void WriteMemory(this MiraConnection p_Connection, int p_ProcessId, ulong p_Address, byte[] p_Data)
        {
            var s_HeaderData = p_Connection.SerializeObject(new RpcMessageHeader
            {
                Category = (int)RPC_CATEGORY.RPCCAT_DBG,
                Magic = MiraConnection.c_Magic,
                ErrorType = (int)DebuggerCmds.Debugger_WriteMem,
                Request = true,
                PayloadSize = (ushort)Marshal.SizeOf<ReadWriteMem>()
            }.ToUInt64());

            var s_Payload = new ReadWriteMem
            {
                ProcessId = p_ProcessId,
                Address = p_Address,
                DataLength = (ulong)p_Data.LongLength,
                Data = new byte[0x1000]
            };

            Array.Copy(p_Data, 0, s_Payload.Data, 0, p_Data.Length);

            var s_PayloadData = p_Connection.SerializeObject(s_Payload);

            using (var s_Writer = new BinaryWriter(p_Connection.GetStream(), Encoding.ASCII, true))
            {
                s_Writer.Write(s_HeaderData);
                s_Writer.Write(s_PayloadData);
            }
        }

        public static byte[] ReadMemory(this MiraConnection p_Connection, int p_ProcessId, ulong p_Address,
            ushort p_Size)
        {
            if (p_Size > 0x1000)
                return null;

            p_Connection.SendMessage(new RpcMessageHeader
            {
                Category = (int)RPC_CATEGORY.RPCCAT_DBG,
                Magic = MiraConnection.c_Magic,
                ErrorType = (int)DebuggerCmds.Debugger_ReadMem,
                Request = true,
                PayloadSize = (ushort)Marshal.SizeOf<ReadWriteMem>()
            }, new ReadWriteMem
            {
                ProcessId = p_ProcessId,
                Address = p_Address,
                DataLength = p_Size,
                Data = new byte[0x1000]
            });

            var s_Response = p_Connection.ReceiveObject<ReadWriteMem>();

            return s_Response.Data;
        }
    }
}
