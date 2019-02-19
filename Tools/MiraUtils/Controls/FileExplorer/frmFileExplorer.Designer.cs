//namespace MiraUtils.Controls.FileExplorer
//{
//    partial class frmFileExplorer
//    {
//        /// <summary>
//        /// Required designer variable.
//        /// </summary>
//        private System.ComponentModel.IContainer components = null;

//        /// <summary>
//        /// Clean up any resources being used.
//        /// </summary>
//        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
//        protected override void Dispose(bool disposing)
//        {
//            if (disposing && (components != null))
//            {
//                components.Dispose();
//            }
//            base.Dispose(disposing);
//        }

//        #region Windows Form Designer generated code

//        /// <summary>
//        /// Required method for Designer support - do not modify
//        /// the contents of this method with the code editor.
//        /// </summary>
//        private void InitializeComponent()
//        {
//            this.components = new System.ComponentModel.Container();
//            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(frmFileExplorer));
//            this.tvExplorer = new System.Windows.Forms.TreeView();
//            this.imgList = new System.Windows.Forms.ImageList(this.components);
//            this.cmuExplorer = new System.Windows.Forms.ContextMenuStrip(this.components);
//            this.cmuDownload = new System.Windows.Forms.ToolStripMenuItem();
//            this.cmuUpload = new System.Windows.Forms.ToolStripMenuItem();
//            this.cmuProperties = new System.Windows.Forms.ToolStripMenuItem();
//            this.cmuExplorer.SuspendLayout();
//            this.SuspendLayout();
//            // 
//            // tvExplorer
//            // 
//            this.tvExplorer.ContextMenuStrip = this.cmuExplorer;
//            this.tvExplorer.Dock = System.Windows.Forms.DockStyle.Fill;
//            this.tvExplorer.Location = new System.Drawing.Point(0, 0);
//            this.tvExplorer.Name = "tvExplorer";
//            this.tvExplorer.Size = new System.Drawing.Size(330, 450);
//            this.tvExplorer.TabIndex = 0;
//            this.tvExplorer.BeforeExpand += new System.Windows.Forms.TreeViewCancelEventHandler(this.tvExplorer_BeforeExpand);
//            // 
//            // imgList
//            // 
//            this.imgList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("imgList.ImageStream")));
//            this.imgList.TransparentColor = System.Drawing.Color.Transparent;
//            this.imgList.Images.SetKeyName(0, "ComputerSystem");
//            this.imgList.Images.SetKeyName(1, "Folder");
//            this.imgList.Images.SetKeyName(2, "FolderOpen");
//            this.imgList.Images.SetKeyName(3, "BinaryFile");
//            this.imgList.Images.SetKeyName(4, "TextFile");
//            this.imgList.Images.SetKeyName(5, "Key");
//            // 
//            // cmuExplorer
//            // 
//            this.cmuExplorer.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
//            this.cmuDownload,
//            this.cmuUpload,
//            this.cmuProperties});
//            this.cmuExplorer.Name = "cmuExplorer";
//            this.cmuExplorer.Size = new System.Drawing.Size(129, 70);
//            // 
//            // cmuDownload
//            // 
//            this.cmuDownload.Name = "cmuDownload";
//            this.cmuDownload.Size = new System.Drawing.Size(128, 22);
//            this.cmuDownload.Text = "Download";
//            this.cmuDownload.Click += new System.EventHandler(this.cmuDownload_Click);
//            // 
//            // cmuUpload
//            // 
//            this.cmuUpload.Name = "cmuUpload";
//            this.cmuUpload.Size = new System.Drawing.Size(128, 22);
//            this.cmuUpload.Text = "Upload";
//            // 
//            // cmuProperties
//            // 
//            this.cmuProperties.Name = "cmuProperties";
//            this.cmuProperties.Size = new System.Drawing.Size(128, 22);
//            this.cmuProperties.Text = "Properties";
//            this.cmuProperties.Click += new System.EventHandler(this.cmuProperties_Click);
//            // 
//            // frmFileExplorer
//            // 
//            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
//            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
//            this.ClientSize = new System.Drawing.Size(330, 450);
//            this.Controls.Add(this.tvExplorer);
//            this.Name = "frmFileExplorer";
//            this.Text = "File Explorer";
//            this.cmuExplorer.ResumeLayout(false);
//            this.ResumeLayout(false);

//        }

//        #endregion

//        private System.Windows.Forms.TreeView tvExplorer;
//        private System.Windows.Forms.ImageList imgList;
//        private System.Windows.Forms.ContextMenuStrip cmuExplorer;
//        private System.Windows.Forms.ToolStripMenuItem cmuDownload;
//        private System.Windows.Forms.ToolStripMenuItem cmuUpload;
//        private System.Windows.Forms.ToolStripMenuItem cmuProperties;
//    }
//}