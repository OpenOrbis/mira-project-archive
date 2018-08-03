namespace MiraToolkit.Controls.Console
{
    partial class ucOutputs
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
            this.tcOutputs = new System.Windows.Forms.TabControl();
            this.cmuConsole = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.cmuAddConsole = new System.Windows.Forms.ToolStripMenuItem();
            this.cmuRemoveConsole = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
            this.cmuSaveConsole = new System.Windows.Forms.ToolStripMenuItem();
            this.cmuConsole.SuspendLayout();
            this.SuspendLayout();
            // 
            // tcOutputs
            // 
            this.tcOutputs.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tcOutputs.Location = new System.Drawing.Point(0, 0);
            this.tcOutputs.Name = "tcOutputs";
            this.tcOutputs.SelectedIndex = 0;
            this.tcOutputs.Size = new System.Drawing.Size(435, 151);
            this.tcOutputs.TabIndex = 0;
            // 
            // cmuConsole
            // 
            this.cmuConsole.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.cmuAddConsole,
            this.cmuRemoveConsole,
            this.toolStripMenuItem1,
            this.cmuSaveConsole});
            this.cmuConsole.Name = "cmuConsole";
            this.cmuConsole.Size = new System.Drawing.Size(118, 76);
            // 
            // cmuAddConsole
            // 
            this.cmuAddConsole.Name = "cmuAddConsole";
            this.cmuAddConsole.Size = new System.Drawing.Size(117, 22);
            this.cmuAddConsole.Text = "Add";
            // 
            // cmuRemoveConsole
            // 
            this.cmuRemoveConsole.Name = "cmuRemoveConsole";
            this.cmuRemoveConsole.Size = new System.Drawing.Size(117, 22);
            this.cmuRemoveConsole.Text = "Remove";
            // 
            // toolStripMenuItem1
            // 
            this.toolStripMenuItem1.Name = "toolStripMenuItem1";
            this.toolStripMenuItem1.Size = new System.Drawing.Size(114, 6);
            // 
            // cmuSaveConsole
            // 
            this.cmuSaveConsole.Name = "cmuSaveConsole";
            this.cmuSaveConsole.Size = new System.Drawing.Size(117, 22);
            this.cmuSaveConsole.Text = "Save";
            // 
            // ucOutputs
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(435, 151);
            this.Controls.Add(this.tcOutputs);
            this.Name = "ucOutputs";
            this.cmuConsole.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TabControl tcOutputs;
        private System.Windows.Forms.ContextMenuStrip cmuConsole;
        private System.Windows.Forms.ToolStripMenuItem cmuAddConsole;
        private System.Windows.Forms.ToolStripMenuItem cmuRemoveConsole;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem cmuSaveConsole;
    }
}
