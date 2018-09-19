namespace MiraToolkit.Controls.Debugger
{
    partial class ucCallStack
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
            this.lstCallStack = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.SuspendLayout();
            // 
            // lstCallStack
            // 
            this.lstCallStack.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1});
            this.lstCallStack.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lstCallStack.GridLines = true;
            this.lstCallStack.Location = new System.Drawing.Point(0, 0);
            this.lstCallStack.Name = "lstCallStack";
            this.lstCallStack.Size = new System.Drawing.Size(519, 497);
            this.lstCallStack.TabIndex = 1;
            this.lstCallStack.UseCompatibleStateImageBehavior = false;
            this.lstCallStack.View = System.Windows.Forms.View.Details;
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Function Address";
            this.columnHeader1.Width = 193;
            // 
            // ucCallStack
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.lstCallStack);
            this.Name = "ucCallStack";
            this.Size = new System.Drawing.Size(519, 497);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ListView lstCallStack;
        private System.Windows.Forms.ColumnHeader columnHeader1;
    }
}
