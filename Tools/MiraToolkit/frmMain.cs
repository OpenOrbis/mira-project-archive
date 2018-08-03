using MiraToolkit.Controls.Generic;
using MiraToolkit.Core;
using System;
using System.Collections.Generic;
using System.Windows.Forms;
using WeifenLuo.WinFormsUI.Docking;

namespace MiraToolkit
{
    public partial class frmMain : Form
    {
        // List of the current devices
        List<MiraDevice> m_Devices;

        public frmMain()
        {
            InitializeComponent();

            m_Devices = new List<MiraDevice>();


            Program.DockPanel = dockPanel;
            Program.StatusChanged += Mira_OnStatusChanged;

            new Controls.Console.ucOutputs(null).Show(Program.DockPanel, DockState.DockBottom);
        }

        private void Mira_OnStatusChanged(object sender, StatusChangedEventArgs e)
        {
            this.Invoke((MethodInvoker)delegate
            {
                lblStatus.Text = e.Message;
                barProgress.Value = e.Percent ?? 0;
            });
        }

        private void mmuConnect_Click(object sender, EventArgs e)
        {
            var s_Dialog = new frmIpAddress();

            if (s_Dialog.ShowDialog() != DialogResult.OK)
                return;

            var s_Device = new MiraDevice(s_Dialog.IPAddress, 9999);

            var s_Result = s_Device.Connection.Connect();

            if (!s_Result)
            {
                MessageBox.Show($"Could not connect to: {s_Device.Hostname}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                Program.SetStatus($"Could not connect to: {s_Device.Hostname}");
                return;
            }


            MessageBox.Show($"Connected to: {s_Device.Hostname}");
            Program.SetStatus($"Connected to: {s_Device.Hostname}");

            LoadUIForDevice(s_Device);
            
            m_Devices.Add(s_Device);
        }

        private void LoadUIForDevice(MiraDevice p_Device)
        {
            //var s_DevConsoleLog = new MiraConsole(p_Device, 9998, "dev_console_log.txt");

            //p_Device.AddConsole(9998);

            //new Controls.FileTransfer.ucFileTransfer(p_Device).Show(Program.DockPanel, DockState.DockRight);

            new Controls.Console.ucOutputs(null);
        }
    }
}
