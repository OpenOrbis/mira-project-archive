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
            this.tvDirectories.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tvDirectories.Location = new System.Drawing.Point(0, 0);
            this.tvDirectories.Name = "tvDirectories";
            this.tvDirectories.Size = new System.Drawing.Size(364, 497);
            this.tvDirectories.TabIndex = 0;
            this.tvDirectories.BeforeExpand += new System.Windows.Forms.TreeViewCancelEventHandler(this.tvDirectories_BeforeExpand);
            // 
            // ucFileTransfer
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.tvDirectories);
            this.Name = "ucFileTransfer";
            this.Size = new System.Drawing.Size(364, 497);
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.ImageList imgList;
        private System.Windows.Forms.TreeView tvDirectories;
    }
}
