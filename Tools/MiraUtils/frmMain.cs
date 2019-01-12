﻿using MiraUtils.Client;
using MiraUtils.Controls;
using MiraUtils.Controls.FileExplorer;
using System.Collections.Generic;
using System.Windows.Forms;
using WeifenLuo.WinFormsUI.Docking;

namespace MiraUtils
{
    public partial class frmMain : Form
    {
        public List<PbConnection> Connections { get; protected set; }

        private frmFileExplorer m_Explorer;

        public frmMain()
        {
            InitializeComponent();

            Connections = new List<PbConnection>();

            m_Explorer = new frmFileExplorer(this);

            m_Explorer.Show(dockPanel, DockState.DockRight);
        }

        private void mmuConnect_Click(object sender, System.EventArgs e)
        {
            var s_Dialog = new frmIpAddress();
            if (s_Dialog.ShowDialog() != DialogResult.OK)
                return;

            var s_Connection = new PbConnection(s_Dialog.Address);
            if (!s_Connection.Connect())
            {
                MessageBox.Show("could not connect");
                return;
            }

            if (!s_Connection.IsConnected)
            {
                MessageBox.Show("is not connected");
                return;
            }

            Connections.Add(s_Connection);

            m_Explorer.UpdateControls();
        }
    }
}
