namespace MiraToolkit.Controls.FileTransfer
{
    partial class ucFileTransfer
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

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(ucFileTransfer));
            this.imgList = new System.Windows.Forms.ImageList(this.components);
            this.tvDirectories = new System.Windows.Forms.TreeView();
            this.cmuStrip = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.cmuRefresh = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
            this.cmuUpload = new System.Windows.Forms.ToolStripMenuItem();
            this.cmuDownload = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
            this.cmuDecrypt = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem3 = new System.Windows.Forms.ToolStripSeparator();
            this.cmuDelete = new System.Windows.Forms.ToolStripMenuItem();
            this.cmuCreateNew = new System.Windows.Forms.ToolStripMenuItem();
            this.cmuCreateDirectory = new System.Windows.Forms.ToolStripMenuItem();
            this.cmuStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // imgList
            // 
            this.imgList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("imgList.ImageStream")));
            this.imgList.TransparentColor = System.Drawing.Color.Transparent;
            this.imgList.Images.SetKeyName(0, "imgFolder");
            this.imgList.Images.SetKeyName(1, "imgFolderGray");
            this.imgList.Images.SetKeyName(2, "imgHtml");
            this.imgList.Images.SetKeyName(3, "imgKey");
            this.imgList.Images.SetKeyName(4, "imgLink");
            this.imgList.Images.SetKeyName(5, "imgThread");
            this.imgList.Images.SetKeyName(6, "imgThreadStop");
            this.imgList.Images.SetKeyName(7, "imgRegistersWindow");
            this.imgList.Images.SetKeyName(8, "imgRefresh");
            this.imgList.Images.SetKeyName(9, "imgRemoteFolder");
            this.imgList.Images.SetKeyName(10, "imgFileExclude");
            this.imgList.Images.SetKeyName(11, "imgFileWarning");
            // 
            // tvDirectories
            // 
            this.tvDirectories.ContextMenuStrip = this.cmuStrip;
            this.tvDirectories.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tvDirectories.Location = new System.Drawing.Point(0, 0);
            this.tvDirectories.Name = "tvDirectories";
            this.tvDirectories.Size = new System.Drawing.Size(348, 458);
            this.tvDirectories.TabIndex = 0;
            this.tvDirectories.BeforeExpand += new System.Windows.Forms.TreeViewCancelEventHandler(this.tvDirectories_BeforeExpand);
            // 
            // cmuStrip
            // 
            this.cmuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.cmuRefresh,
            this.toolStripMenuItem1,
            this.cmuUpload,
            this.cmuDownload,
            this.toolStripMenuItem2,
            this.cmuDecrypt,
            this.toolStripMenuItem3,
            this.cmuDelete,
            this.cmuCreateNew});
            this.cmuStrip.Name = "cmuStrip";
            this.cmuStrip.Size = new System.Drawing.Size(181, 176);
            // 
            // cmuRefresh
            // 
            this.cmuRefresh.Name = "cmuRefresh";
            this.cmuRefresh.Size = new System.Drawing.Size(180, 22);
            this.cmuRefresh.Text = "Refresh";
            // 
            // toolStripMenuItem1
            // 
            this.toolStripMenuItem1.Name = "toolStripMenuItem1";
            this.toolStripMenuItem1.Size = new System.Drawing.Size(177, 6);
            // 
            // cmuUpload
            // 
            this.cmuUpload.Name = "cmuUpload";
            this.cmuUpload.Size = new System.Drawing.Size(180, 22);
            this.cmuUpload.Text = "Upload";
            this.cmuUpload.Click += new System.EventHandler(this.cmuUpload_Click);
            // 
            // cmuDownload
            // 
            this.cmuDownload.Name = "cmuDownload";
            this.cmuDownload.Size = new System.Drawing.Size(180, 22);
            this.cmuDownload.Text = "Download";
            this.cmuDownload.Click += new System.EventHandler(this.cmuDownload_Click);
            // 
            // toolStripMenuItem2
            // 
            this.toolStripMenuItem2.Name = "toolStripMenuItem2";
            this.toolStripMenuItem2.Size = new System.Drawing.Size(177, 6);
            // 
            // cmuDecrypt
            // 
            this.cmuDecrypt.Name = "cmuDecrypt";
            this.cmuDecrypt.Size = new System.Drawing.Size(180, 22);
            this.cmuDecrypt.Text = "Decrypt Executable";
            this.cmuDecrypt.Click += new System.EventHandler(this.cmuDecrypt_Click);
            // 
            // toolStripMenuItem3
            // 
            this.toolStripMenuItem3.Name = "toolStripMenuItem3";
            this.toolStripMenuItem3.Size = new System.Drawing.Size(177, 6);
            // 
            // cmuDelete
            // 
            this.cmuDelete.Name = "cmuDelete";
            this.cmuDelete.Size = new System.Drawing.Size(180, 22);
            this.cmuDelete.Text = "Delete";
            this.cmuDelete.Click += new System.EventHandler(this.cmuDelete_Click);
            // 
            // cmuCreateNew
            // 
            this.cmuCreateNew.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.cmuCreateDirectory});
            this.cmuCreateNew.Name = "cmuCreateNew";
            this.cmuCreateNew.Size = new System.Drawing.Size(180, 22);
            this.cmuCreateNew.Text = "Create New";
            // 
            // cmuCreateDirectory
            // 
            this.cmuCreateDirectory.Name = "cmuCreateDirectory";
            this.cmuCreateDirectory.Size = new System.Drawing.Size(122, 22);
            this.cmuCreateDirectory.Text = "Directory";
            // 
            // ucFileTransfer
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(348, 458);
            this.Controls.Add(this.tvDirectories);
            this.Name = "ucFileTransfer";
            this.cmuStrip.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.ImageList imgList;
        private System.Windows.Forms.TreeView tvDirectories;
        private System.Windows.Forms.ContextMenuStrip cmuStrip;
        private System.Windows.Forms.ToolStripMenuItem cmuRefresh;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem cmuUpload;
        private System.Windows.Forms.ToolStripMenuItem cmuDownload;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem2;
        private System.Windows.Forms.ToolStripMenuItem cmuDecrypt;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem3;
        private System.Windows.Forms.ToolStripMenuItem cmuDelete;
        private System.Windows.Forms.ToolStripMenuItem cmuCreateNew;
        private System.Windows.Forms.ToolStripMenuItem cmuCreateDirectory;
    }
}
