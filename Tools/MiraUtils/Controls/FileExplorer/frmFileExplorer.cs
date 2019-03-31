﻿using MiraUtils.Client;
using MiraUtils.Client.FileExplorer;
using System;
using System.Collections.Generic;
using System.IO;
using System.Windows.Forms;
using WeifenLuo.WinFormsUI.Docking;

namespace MiraUtils.Controls.FileExplorer
{
    public partial class frmFileExplorer : DockContent, IMiraControl
    {
        private MiraConnection m_Connection;

        private const string c_ComputerSystem = "ComputerSystem";
        private const string c_Folder = "Folder";
        private const string c_FolderOpen = "FolderOpen";
        private const string c_File = "File";

        public frmFileExplorer(MiraConnection p_Connection)
        {
            InitializeComponent();

            m_Connection = p_Connection ?? throw new ArgumentNullException("invalid connection");

            Text = $"File Explorer - [{m_Connection.Nickname}]";
        }

        public void UpdateControls()
        {
            tvExplorer.Nodes.Clear();

            var l_ConnectionNode = new TreeNode
            {
                ImageKey = c_ComputerSystem,
                Text = m_Connection.Address,
                Tag = m_Connection
            };

            l_ConnectionNode.Nodes.Add(new TreeNode
            {
                Text = "loading...",
            });
            tvExplorer.Nodes.Add(l_ConnectionNode);
        }

        private void PopulateNode(MiraConnection p_Connection, TreeNode p_Parent, string p_Path)
        {
            p_Parent.Nodes.Clear();

            var s_Nodes = new List<TreeNode>();

            var s_Dents = p_Connection.GetDents(p_Path);

            foreach (var l_Dent in s_Dents)
            {
                if (l_Dent.NameString == ".." || l_Dent.NameString == ".")
                    continue;

                var l_Node = new TreeNode
                {
                    Text = l_Dent.NameString,
                    Tag = l_Dent
                };

                switch ((FileTypes)l_Dent.Type)
                {
                    case FileTypes.DT_DIR:
                        l_Node.ImageKey = c_Folder;
                        l_Node.Nodes.Add(new TreeNode
                        {
                            Text = "loading...",
                        });
                        break;
                    case FileTypes.DT_REG:
                        l_Node.ImageKey = c_File;
                        break;
                    default:
                        break;
                }

                s_Nodes.Add(l_Node);
            }

            p_Parent.Nodes.AddRange(s_Nodes.ToArray());
        }

        private void tvExplorer_BeforeExpand(object p_Sender, TreeViewCancelEventArgs p_Args)
        {
            var s_Node = p_Args.Node;
            if (s_Node == null)
                return;

            if (s_Node.Tag is MiraConnection)
            {
                PopulateNode(s_Node.Tag as MiraConnection, s_Node, "/");
            }
            else if (s_Node.Tag is FileExplorerDent)
            {
                var s_Path = GetFullPath(s_Node);
                PopulateNode(GetNodeConnection(s_Node), s_Node, s_Path);
            }
        }

        private string GetFullPath(TreeNode p_Node)
        {
            var s_Paths = new Stack<string>();

            var s_Current = p_Node;
            while (s_Current != null)
            {
                var s_Dent = s_Current.Tag as FileExplorerDent;
                if (s_Dent == null)
                {
                    s_Current = s_Current.Parent;
                    continue;
                }

                s_Paths.Push(s_Dent.NameString);
                s_Current = s_Current.Parent;
            }

            var s_Path = string.Empty;

            while (s_Paths.Count > 0)
                s_Path += $"/{s_Paths.Pop()}";

            return s_Path;
        }

        private MiraConnection GetNodeConnection(TreeNode p_Node)
        {
            var s_Current = p_Node;
            while (s_Current != null)
            {
                var s_Dent = s_Current.Tag as MiraConnection;
                if (s_Dent == null)
                {
                    s_Current = s_Current.Parent;
                    continue;
                }

                return s_Current.Tag as MiraConnection;
            }

            return null;
        }

        private void cmuDownload_Click(object sender, System.EventArgs e)
        {
            var s_Node = tvExplorer.SelectedNode;
            if (s_Node == null)
                return;

            var s_Entry = s_Node.Tag as FileExplorerDent;
            if (s_Entry == null)
                return;

            var s_Path = GetFullPath(s_Node);
            if (string.IsNullOrWhiteSpace(s_Path))
                return;

            var s_Connection = GetNodeConnection(s_Node);
            if (s_Connection == null)
                return;

            if ((FileTypes)s_Entry.Type == FileTypes.DT_DIR)
            {
                var s_FolderBrowserDialog = new FolderBrowserDialog
                {
                    Description = "Select folder to download to...",
                    ShowNewFolderButton = true,
                };

                if (s_FolderBrowserDialog.ShowDialog() != DialogResult.OK)
                    return;

                var s_SavePath = s_FolderBrowserDialog.SelectedPath;
                RecursiveDownload(s_Connection, s_SavePath, s_Path, true);
            }
            else if ((FileTypes)s_Entry.Type == FileTypes.DT_REG)
            {
                var s_Data = s_Connection.DownloadFile(s_Path, (int p_Percent, bool p_Error) =>
                {
                    Console.WriteLine($"Download {p_Percent}% Error: {p_Error}");
                });

                if (s_Data == null)
                {
                    MessageBox.Show("could not download file");
                    return;
                }

                var s_SafeFileDialog = new SaveFileDialog
                {
                    Title = "Save as...",
                    FileName = s_Entry.NameString,
                    Filter = "All Files (*.*)|*.*",
                };

                if (s_SafeFileDialog.ShowDialog() == DialogResult.OK)
                    File.WriteAllBytes(s_SafeFileDialog.FileName, s_Data);
            }
        }

        private void RecursiveDownload(MiraConnection p_Connection, string p_LocalDir, string p_RemoteDir, bool p_Overwrite = true)
        {
            var s_DirectoryEntries = p_Connection.GetDents(p_RemoteDir);

            foreach (var l_Entry in s_DirectoryEntries)
            {
                if (l_Entry.NameString == "." || l_Entry.NameString == "..")
                    continue;

                var l_LocalPath = $"{p_LocalDir}/{new string(l_Entry.Name)}";
                var l_RemotePath = $"{p_RemoteDir}/{new string(l_Entry.Name)}";

                if ((FileTypes)l_Entry.Type == FileTypes.DT_DIR)
                {
                    if (!Directory.Exists(l_LocalPath))
                        Directory.CreateDirectory(l_LocalPath);

                    RecursiveDownload(p_Connection, l_LocalPath, l_RemotePath);
                }
                else if ((FileTypes)l_Entry.Type == FileTypes.DT_REG)
                {
                    var s_Data = p_Connection.DownloadFile(l_RemotePath);
                    if (s_Data == null)
                        continue;

                    File.WriteAllBytes(l_LocalPath, s_Data);
                }
            }
        }

        private void cmuProperties_Click(object sender, System.EventArgs e)
        {
            var s_Node = tvExplorer.SelectedNode;
            if (s_Node == null)
                return;

            var s_Entry = s_Node.Tag as FileExplorerDent;
            if (s_Entry == null)
                return;
        }
    }
}