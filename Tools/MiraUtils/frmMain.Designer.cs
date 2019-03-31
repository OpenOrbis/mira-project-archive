namespace MiraUtils
{
    partial class frmMain
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.dockPanel = new WeifenLuo.WinFormsUI.Docking.DockPanel();
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.barProgress = new System.Windows.Forms.ToolStripProgressBar();
            this.lblStatus = new System.Windows.Forms.ToolStripStatusLabel();
            this.mmuStrip = new System.Windows.Forms.MenuStrip();
            this.mmuFile = new System.Windows.Forms.ToolStripMenuItem();
            this.mmuConnect = new System.Windows.Forms.ToolStripMenuItem();
            this.mmuDisconnect = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
            this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.pluginsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fileExplorerToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.statusStrip1.SuspendLayout();
            this.mmuStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // dockPanel
            // 
            this.dockPanel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.dockPanel.Location = new System.Drawing.Point(0, 24);
            this.dockPanel.Name = "dockPanel";
            this.dockPanel.Size = new System.Drawing.Size(685, 367);
            this.dockPanel.TabIndex = 0;
            // 
            // statusStrip1
            // 
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.barProgress,
            this.lblStatus});
            this.statusStrip1.Location = new System.Drawing.Point(0, 391);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Size = new System.Drawing.Size(685, 22);
            this.statusStrip1.TabIndex = 2;
            this.statusStrip1.Text = "statusStrip1";
            // 
            // barProgress
            // 
            this.barProgress.Name = "barProgress";
            this.barProgress.Size = new System.Drawing.Size(100, 16);
            // 
            // lblStatus
            // 
            this.lblStatus.Name = "lblStatus";
            this.lblStatus.Size = new System.Drawing.Size(88, 17);
            this.lblStatus.Text = "Not Connected";
            // 
            // mmuStrip
            // 
            this.mmuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mmuFile,
            this.pluginsToolStripMenuItem});
            this.mmuStrip.Location = new System.Drawing.Point(0, 0);
            this.mmuStrip.Name = "mmuStrip";
            this.mmuStrip.Size = new System.Drawing.Size(685, 24);
            this.mmuStrip.TabIndex = 3;
            this.mmuStrip.Text = "menuStrip1";
            // 
            // mmuFile
            // 
            this.mmuFile.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mmuConnect,
            this.mmuDisconnect,
            this.toolStripMenuItem1,
            this.exitToolStripMenuItem});
            this.mmuFile.Name = "mmuFile";
            this.mmuFile.Size = new System.Drawing.Size(37, 20);
            this.mmuFile.Text = "File";
            // 
            // mmuConnect
            // 
            this.mmuConnect.Name = "mmuConnect";
            this.mmuConnect.Size = new System.Drawing.Size(133, 22);
            this.mmuConnect.Text = "Connect";
            this.mmuConnect.Click += new System.EventHandler(this.mmuConnect_Click);
            // 
            // mmuDisconnect
            // 
            this.mmuDisconnect.Name = "mmuDisconnect";
            this.mmuDisconnect.Size = new System.Drawing.Size(133, 22);
            this.mmuDisconnect.Text = "Disconnect";
            // 
            // toolStripMenuItem1
            // 
            this.toolStripMenuItem1.Name = "toolStripMenuItem1";
            this.toolStripMenuItem1.Size = new System.Drawing.Size(130, 6);
            // 
            // exitToolStripMenuItem
            // 
            this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
            this.exitToolStripMenuItem.Size = new System.Drawing.Size(133, 22);
            this.exitToolStripMenuItem.Text = "Exit";
            // 
            // pluginsToolStripMenuItem
            // 
            this.pluginsToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileExplorerToolStripMenuItem});
            this.pluginsToolStripMenuItem.Name = "pluginsToolStripMenuItem";
            this.pluginsToolStripMenuItem.Size = new System.Drawing.Size(58, 20);
            this.pluginsToolStripMenuItem.Text = "Plugins";
            // 
            // fileExplorerToolStripMenuItem
            // 
            this.fileExplorerToolStripMenuItem.Name = "fileExplorerToolStripMenuItem";
            this.fileExplorerToolStripMenuItem.Size = new System.Drawing.Size(137, 22);
            this.fileExplorerToolStripMenuItem.Text = "File Explorer";
            // 
            // frmMain
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(685, 413);
            this.Controls.Add(this.dockPanel);
            this.Controls.Add(this.mmuStrip);
            this.Controls.Add(this.statusStrip1);
            this.Name = "frmMain";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "MiraUtils - [ Not Connected ]";
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            this.mmuStrip.ResumeLayout(false);
            this.mmuStrip.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private WeifenLuo.WinFormsUI.Docking.DockPanel dockPanel;
        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.ToolStripProgressBar barProgress;
        private System.Windows.Forms.ToolStripStatusLabel lblStatus;
        private System.Windows.Forms.MenuStrip mmuStrip;
        private System.Windows.Forms.ToolStripMenuItem mmuFile;
        private System.Windows.Forms.ToolStripMenuItem mmuConnect;
        private System.Windows.Forms.ToolStripMenuItem mmuDisconnect;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem pluginsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem fileExplorerToolStripMenuItem;
    }
}

