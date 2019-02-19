using System.IO;

namespace MiraUtils.Client.FileExplorer
{
    public class FileExplorerStatRequest : MessageSerializable
    {
        public int Handle;
        public ushort PathLength;
        public char[] Path;

        public override void Deserialize(BinaryReader p_Reader)
        {
            Handle = p_Reader.ReadInt32();
            PathLength = p_Reader.ReadUInt16();
            Path = p_Reader.ReadChars(PathLength);
        }

        public override byte[] Serialize()
        {
            using (var s_Writer = new BinaryWriter(new MemoryStream()))
            {
                s_Writer.Write(Handle);
                s_Writer.Write(PathLength);
                s_Writer.Write(Path);

                return ((MemoryStream)s_Writer.BaseStream).ToArray();
            }
        }
    }

    public class FileExplorerStat : MessageSerializable
    {
        /// <summary>
        /// inode's device
        /// </summary>
        public uint Dev;

        /// <summary>
        /// inode's number
        /// </summary>
        public uint Ino;

        /// <summary>
        /// inode protection mode
        /// </summary>
        public ushort Mode;

        /// <summary>
        /// number of hard links
        /// </summary>
        public ushort Nlink;

        /// <summary>
        /// user ID of the file's owner 
        /// </summary>
        public uint Uid;

        /// <summary>
        /// group ID of the file's group
        /// </summary>
        public uint Gid;

        /// <summary>
        /// device type
        /// </summary>
        public uint Rdev;

        /// <summary>
        /// time of last access
        /// </summary>
        public TimeSpec Atim;

        /// <summary>
        /// time of last data modification 
        /// </summary>
        public TimeSpec Mtim;

        /// <summary>
        /// time of last file status change
        /// </summary>
        public TimeSpec Ctim;

        /// <summary>
        /// file size, in bytes
        /// </summary>
        public long Size;

        /// <summary>
        /// blocks allocated for file
        /// </summary>
        public long Blocks;

        /// <summary>
        /// optimal blocksize for I/O
        /// </summary>
        public uint Blksize;

        /// <summary>
        /// user defined flags for file
        /// </summary>
        public uint Flags;

        /// <summary>
        /// file generation number
        /// </summary>
        public uint Gen;

        /// <summary>
        /// ?
        /// </summary>
        public int Lspare;

        /// <summary>
        /// time of file creation
        /// </summary>
        public TimeSpec Birthtim;

        public override void Deserialize(BinaryReader p_Reader)
        {
            Dev = p_Reader.ReadUInt32();
            Ino = p_Reader.ReadUInt32();
            Mode = p_Reader.ReadUInt16();
            Nlink = p_Reader.ReadUInt16();
            Uid = p_Reader.ReadUInt32();
            Gid = p_Reader.ReadUInt32();
            Rdev = p_Reader.ReadUInt32();
            Atim = new TimeSpec(p_Reader);
            Mtim = new TimeSpec(p_Reader);
            Ctim = new TimeSpec(p_Reader);
            Size = p_Reader.ReadInt64();
            Blocks = p_Reader.ReadInt64();
            Blksize = p_Reader.ReadUInt32();
            Flags = p_Reader.ReadUInt32();
            Gen = p_Reader.ReadUInt32();
            Birthtim = new TimeSpec(p_Reader);
        }

        public override byte[] Serialize()
        {
            using (var s_Writer = new BinaryWriter(new MemoryStream()))
            {
                s_Writer.Write(Dev);
                s_Writer.Write(Ino);
                s_Writer.Write(Mode);
                s_Writer.Write(Nlink);
                s_Writer.Write(Uid);
                s_Writer.Write(Gid);
                s_Writer.Write(Rdev);
                s_Writer.Write(Atim.Serialize());
                s_Writer.Write(Mtim.Serialize());
                s_Writer.Write(Ctim.Serialize());
                s_Writer.Write(Size);
                s_Writer.Write(Blocks);
                s_Writer.Write(Blksize);
                s_Writer.Write(Flags);
                s_Writer.Write(Gen);
                s_Writer.Write(Lspare);
                s_Writer.Write(Birthtim.Serialize());

                return ((MemoryStream)s_Writer.BaseStream).ToArray();
            }
        }
    }
}
