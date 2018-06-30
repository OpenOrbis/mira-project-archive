namespace MiraToolkit
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
            this.sStrip = new System.Windows.Forms.StatusStrip();
            this.barProgress = new System.Windows.Forms.ToolStripProgressBar();
            this.lblStatus = new System.Windows.Forms.ToolStripStatusLabel();
            this.mmuMenu = new System.Windows.Forms.MenuStrip();
            this.mmuFile = new System.Windows.Forms.ToolStripMenuItem();
            this.mmuConnect = new System.Windows.Forms.ToolStripMenuItem();
            this.dockPanel = new WeifenLuo.WinFormsUI.Docking.DockPanel();
            this.sStrip.SuspendLayout();
            this.mmuMenu.SuspendLayout();
            this.SuspendLayout();
            // 
            // sStrip
            // 
            this.sStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.barProgress,
            this.lblStatus});
            this.sStrip.Location = new System.Drawing.Point(0, 428);
            this.sStrip.Name = "sStrip";
            this.sStrip.Size = new System.Drawing.Size(800, 22);
            this.sStrip.TabIndex = 0;
            this.sStrip.Text = "Status";
            // 
            // barProgress
            // 
            this.barProgress.Name = "barProgress";
            this.barProgress.Size = new System.Drawing.Size(100, 16);
            // 
            // lblStatus
            // 
            this.lblStatus.Name = "lblStatus";
            this.lblStatus.Size = new System.Drawing.Size(137, 17);
            this.lblStatus.Text = "Welcome to Mira Toolkit";
            // 
            // mmuMenu
            // 
            this.mmuMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mmuFile});
            this.mmuMenu.Location = new System.Drawing.Point(0, 0);
            this.mmuMenu.Name = "mmuMenu";
            this.mmuMenu.Size = new System.Drawing.Size(800, 24);
            this.mmuMenu.TabIndex = 1;
            this.mmuMenu.Text = "menuStrip1";
            // 
            // mmuFile
            // 
            this.mmuFile.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mmuConnect});
            this.mmuFile.Name = "mmuFile";
            this.mmuFile.Size = new System.Drawing.Size(37, 20);
            this.mmuFile.Text = "File";
            // 
            // mmuConnect
            // 
            this.mmuConnect.Name = "mmuConnect";
            this.mmuConnect.Size = new System.Drawing.Size(119, 22);
            this.mmuConnect.Text = "Connect";
            this.mmuConnect.Click += new System.EventHandler(this.mmuConnect_Click);
            // 
            // dockPanel
            // 
            this.dockPanel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.dockPanel.Location = new System.Drawing.Point(0, 24);
            this.dockPanel.Name = "dockPanel";
            this.dockPanel.Size = new System.Drawing.Size(800, 404);
            this.dockPanel.TabIndex = 6;
            // 
            // frmMain
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(800, 450);
            this.Controls.Add(this.dockPanel);
            this.Controls.Add(this.sStrip);
            this.Controls.Add(this.mmuMenu);
            this.MainMenuStrip = this.mmuMenu;
            this.Name = "frmMain";
            this.Text = "Mira Toolkit";
            this.sStrip.ResumeLayout(false);
            this.sStrip.PerformLayout();
            this.mmuMenu.ResumeLayout(false);
            this.mmuMenu.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.StatusStrip sStrip;
        private System.Windows.Forms.ToolStripProgressBar barProgress;
        private System.Windows.Forms.ToolStripStatusLabel lblStatus;
        private System.Windows.Forms.MenuStrip mmuMenu;
        private System.Windows.Forms.ToolStripMenuItem mmuFile;
        private System.Windows.Forms.ToolStripMenuItem mmuConnect;
        private WeifenLuo.WinFormsUI.Docking.DockPanel dockPanel;
    }
}

