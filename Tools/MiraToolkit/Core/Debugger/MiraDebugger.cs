using MiraLib;
using System;
using System.Collections.Generic;
using System.Linq;
using MiraToolkit.Controls.Debugger;
using static OniPC.Rpc.MiraDebuggerExtensions;

namespace MiraToolkit.Core.Debugger
{
    public class MiraDebugger
    {
        public enum DataType
        {
            Int8,
            UInt8,
            Int16,
            UInt16,
            Int32,
            UInt32,
            Int64,
            UInt64,
        }
        public enum BreakpointType : byte
        {
            Read = 1,
            Write = 2,
            Execute = 4
        }

        public class Breakpoint
        {
            public ulong Address;
            public byte Size;
            public bool Hardware;
            public bool Enabled;
            public BreakpointType BpType;
            public byte[] Backup;
        }

        public class TrackedValue
        {
            public ulong Address;
            public DataType Type;
            public byte[] Data;
            public byte[] PreviousData;
            public byte[] LockedData;
            public bool Locked;
        }

        public struct Reg
        {
            public ulong R15;
            public ulong R14;
            public ulong R13;
            public ulong R12;
            public ulong R11;
            public ulong R10;
            public ulong R9;
            public ulong R8;
            public ulong RDI;
            public ulong RSI;
            public ulong RBP;
            public ulong RBX;
            public ulong RDX;
            public ulong RCX;
            public ulong RAX;
            public uint TrapNo;
            public ushort FS;
            public ushort GS;
            public uint ERR;
            public ushort ES;
            public ushort DS;
            public ulong RIP;
            public ulong CS;
            public ulong RFlags;
            public ulong RSP;
            public ulong SS;
        }

        public enum DebuggerState
        {
            Paused,
            Running,
            Detach
        }

        public MiraDevice Device { get; protected set; }
        public MiraConnection Connection => Device?.Connection;
        public DebuggerState State { get; protected set; }
        public ProcInfo ProcessInfo { get; protected set; }

        public int ProcessId => ProcessInfo.ProcessId;

        private List<Breakpoint> m_Breakpoints;

        private ucBreakpoints m_BreakpointControl;
        private ucCallStack m_CallStackControl;
        private ucCommand m_CommandControl;
        private ucDisassembly m_DisassemblyControl;
        private ucProcess m_ProcessControl;
        private ucRegisters m_RegistersControl;
        private ucThreads m_ThreadsControl;
        private ucWatch m_WatchControl;
        

        public int HardwareBreakpointCount => m_Breakpoints.Count(p_Breakpoint => p_Breakpoint.Hardware);
        public int SoftwareBreakpointCount => m_Breakpoints.Count(p_Breakpoint => !p_Breakpoint.Hardware);

        public MiraDebugger(MiraDevice p_Device, ProcInfo p_ProcInfo)
        {
            // Create a new list to hold our breakpoints
            m_Breakpoints = new List<Breakpoint>();

            // Assign our information
            ProcessInfo = p_ProcInfo;
            Device = p_Device;

            // Set our debugger to the initial state
            State = DebuggerState.Detach;

            // Create the new UI elements
            m_BreakpointControl = new ucBreakpoints(this);
            m_CallStackControl = new ucCallStack(this);
        }

        public void Close()
        {
            if (State != DebuggerState.Detach)
                Connection.Ptrace((int)PtraceRequests.PT_DETACH, ProcessId, 0, 0);

            
        }

        public bool Attach()
        {
            if (State != DebuggerState.Detach)
                return false;

            if (!Connection.Connected)
                return false;

            var s_Result = Connection.Ptrace((int)PtraceRequests.PT_ATTACH, ProcessInfo.ProcessId, 0, 0);
            if (s_Result.Result < 0)
                return false;

            SetState(DebuggerState.Running);

            return true;
        }

        public bool Detach()
        {
            if (State == DebuggerState.Detach)
                return false;

            if (State == DebuggerState.Paused)
                Go();

            if (!Connection.Connected)
                return false;

            var s_Result = Connection.Ptrace((int)PtraceRequests.PT_DETACH, ProcessInfo.ProcessId, 0, 0);
            if (s_Result.Result < 0)
                return false;

            SetState(DebuggerState.Detach);

            return true;
        }

        /// <summary>
        /// Sends SIGSTOP to process
        /// </summary>
        /// <returns>True on success, false otherwise</returns>
        public bool PauseBreak()
        {
            if (!Connection.Connected)
                return false;

            var s_Result = Connection.Kill(ProcessInfo.ProcessId, 17/*SIGSTOP*/);
            if (s_Result < 0)
                return false;

            SetState(DebuggerState.Paused);

            return true;
        }

        public bool Go()
        {
            if (!Connection.Connected)
                return false;

            var s_Result = Connection.Ptrace((int)PtraceRequests.PT_CONTINUE, ProcessInfo.ProcessId, 1, 0);
            if (s_Result.Result < 0)
                return false;

            SetState(DebuggerState.Running);

            return true;
        }

        public void Update()
        {
            // Verify connection status
            if (!Connection.Connected)
                return;
        }

        public void SetState(DebuggerState p_State)
        {
            State = p_State;

            Update();

            Program.SetStatus($"Process {ProcessInfo.ProcessId}:{ProcessInfo.ProcessName} - {p_State.ToString()}.");
        }

        /// <summary>
        /// Single step execution of traced process
        /// </summary>
        public bool Step()
        {
            // Verify connection status
            if (!Connection.Connected)
                return false;

            if (State != DebuggerState.Paused)
                return false;

            // Call ptrace(PT_STEP
            var s_Result = Connection.Ptrace((int)PtraceRequests.PT_STEP, ProcessInfo.ProcessId, 1, 0);
            if (s_Result.Result < 0)
                return false;

            // Some debugging output
            Program.SetStatus($"Process {ProcessInfo.ProcessId}:{ProcessInfo.ProcessName} stepped.");

            SetState(DebuggerState.Paused);

            return true;
        }

        public Reg GetRegisters()
        {
            var s_Regs = new Reg();

            if (!Connection.Connected)
                return s_Regs;

            // We can only peek registers when we are paused
            if (State != DebuggerState.Paused)
                return s_Regs;

            var s_Result = Connection.Ptrace((int)PtraceRequests.PT_GETREGS, ProcessInfo.ProcessId, 0, 0, true);
            if (s_Result.Result < 0)
                return s_Regs;

            return Connection.DeserializeObject<Reg>(s_Result.Buffer);
        }

        /// <summary>
        /// Sets registers
        /// </summary>
        /// <param name="p_Registers">Registers to set, all must be filled out accurately</param>
        /// <returns>True on success, false otherwise</returns>
        public bool SetRegisters(Reg p_Registers)
        {
            if (!Connection.Connected)
                return false;

            if (State != DebuggerState.Paused)
                return false;

            var s_Result = Connection.Ptrace((int)PtraceRequests.PT_SETREGS, ProcessInfo.ProcessId, 0, 0, true, Connection.SerializeObject(p_Registers));
            if (s_Result.Result < 0)
                return false;

            return true;
        }

        public bool AddBreakpoint(ulong p_Address, bool p_Hardware, int p_Size, BreakpointType p_Type, bool p_Enabled = true)
        {
            if (!Connection.Connected)
                return false;

            if (p_Size != 1 || p_Size != 2 || p_Size != 4 || p_Size != 8)
                return false;

            byte[] s_Backup = null;

            // For software breakpoints we need a 0xCC 0x90
            if (!p_Hardware)
                s_Backup = Connection.ReadMemory(ProcessId, p_Address, 2);

            var s_Breakpoint = new Breakpoint
            {
                Address = p_Address,
                BpType = p_Type,
                Enabled = p_Enabled,
                Hardware = p_Hardware,
                Size = (byte)p_Size,
                Backup = s_Backup
            };

            if (!p_Hardware)
            {
                // Write software breakpoint
                Connection.WriteMemory(ProcessId, p_Address, new byte[] { 0xCC, 0x90 });
            }
            else
            {
                throw new Exception("Hardware breakpoints not supported currently");
            }

            m_Breakpoints.Add(s_Breakpoint);

            return true;
        }
    }
}
