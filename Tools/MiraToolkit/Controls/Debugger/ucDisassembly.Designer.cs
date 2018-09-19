namespace MiraToolkit.Controls.Debugger
{
    partial class ucDisassembly
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
            this.lstDisassembly = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.SuspendLayout();
            // 
            // lstDisassembly
            // 
            this.lstDisassembly.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2});
            this.lstDisassembly.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lstDisassembly.GridLines = true;
            this.lstDisassembly.Location = new System.Drawing.Point(0, 0);
            this.lstDisassembly.Name = "lstDisassembly";
            this.lstDisassembly.Size = new System.Drawing.Size(496, 551);
            this.lstDisassembly.TabIndex = 1;
            this.lstDisassembly.UseCompatibleStateImageBehavior = false;
            this.lstDisassembly.View = System.Windows.Forms.View.Details;
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Address";
            this.columnHeader1.Width = 69;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "Instruction";
            this.columnHeader2.Width = 342;
            // 
            // ucDisassembly
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.lstDisassembly);
            this.Name = "ucDisassembly";
            this.Size = new System.Drawing.Size(496, 551);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ListView lstDisassembly;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
    }
}
