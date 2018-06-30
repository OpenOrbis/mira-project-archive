namespace MiraToolkit.Controls.Debugger
{
    partial class ucConsole
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
            this.tcConsole = new System.Windows.Forms.TabControl();
            this.SuspendLayout();
            // 
            // tcConsole
            // 
            this.tcConsole.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tcConsole.Location = new System.Drawing.Point(0, 0);
            this.tcConsole.Name = "tcConsole";
            this.tcConsole.SelectedIndex = 0;
            this.tcConsole.Size = new System.Drawing.Size(150, 150);
            this.tcConsole.TabIndex = 0;
            // 
            // ucConsole
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.tcConsole);
            this.Name = "ucConsole";
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TabControl tcConsole;
    }
}
