namespace MiraUtils.Controls.FileExplorer
{
    partial class frmFileProperties
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
            this.gbOwnerPermissions = new System.Windows.Forms.GroupBox();
            this.cbOwnerRead = new System.Windows.Forms.CheckBox();
            this.cbOwnerWrite = new System.Windows.Forms.CheckBox();
            this.cbOwnerExecute = new System.Windows.Forms.CheckBox();
            this.gbGroupPermissions = new System.Windows.Forms.GroupBox();
            this.cbGroupExecute = new System.Windows.Forms.CheckBox();
            this.cbGroupWrite = new System.Windows.Forms.CheckBox();
            this.cbGroupRead = new System.Windows.Forms.CheckBox();
            this.gbPublicPermissions = new System.Windows.Forms.GroupBox();
            this.checkBox4 = new System.Windows.Forms.CheckBox();
            this.checkBox5 = new System.Windows.Forms.CheckBox();
            this.checkBox6 = new System.Windows.Forms.CheckBox();
            this.lblNumericValue = new System.Windows.Forms.Label();
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.btnOK = new System.Windows.Forms.Button();
            this.btnCancel = new System.Windows.Forms.Button();
            this.gbOwnerPermissions.SuspendLayout();
            this.gbGroupPermissions.SuspendLayout();
            this.gbPublicPermissions.SuspendLayout();
            this.SuspendLayout();
            // 
            // gbOwnerPermissions
            // 
            this.gbOwnerPermissions.Controls.Add(this.cbOwnerExecute);
            this.gbOwnerPermissions.Controls.Add(this.cbOwnerWrite);
            this.gbOwnerPermissions.Controls.Add(this.cbOwnerRead);
            this.gbOwnerPermissions.Location = new System.Drawing.Point(12, 12);
            this.gbOwnerPermissions.Name = "gbOwnerPermissions";
            this.gbOwnerPermissions.Size = new System.Drawing.Size(195, 45);
            this.gbOwnerPermissions.TabIndex = 0;
            this.gbOwnerPermissions.TabStop = false;
            this.gbOwnerPermissions.Text = "Owner permissions";
            // 
            // cbOwnerRead
            // 
            this.cbOwnerRead.AutoSize = true;
            this.cbOwnerRead.Location = new System.Drawing.Point(6, 19);
            this.cbOwnerRead.Name = "cbOwnerRead";
            this.cbOwnerRead.Size = new System.Drawing.Size(52, 17);
            this.cbOwnerRead.TabIndex = 0;
            this.cbOwnerRead.Text = "Read";
            this.cbOwnerRead.UseVisualStyleBackColor = true;
            // 
            // cbOwnerWrite
            // 
            this.cbOwnerWrite.AutoSize = true;
            this.cbOwnerWrite.Location = new System.Drawing.Point(64, 19);
            this.cbOwnerWrite.Name = "cbOwnerWrite";
            this.cbOwnerWrite.Size = new System.Drawing.Size(51, 17);
            this.cbOwnerWrite.TabIndex = 1;
            this.cbOwnerWrite.Text = "Write";
            this.cbOwnerWrite.UseVisualStyleBackColor = true;
            // 
            // cbOwnerExecute
            // 
            this.cbOwnerExecute.AutoSize = true;
            this.cbOwnerExecute.Location = new System.Drawing.Point(121, 19);
            this.cbOwnerExecute.Name = "cbOwnerExecute";
            this.cbOwnerExecute.Size = new System.Drawing.Size(65, 17);
            this.cbOwnerExecute.TabIndex = 2;
            this.cbOwnerExecute.Text = "Execute";
            this.cbOwnerExecute.UseVisualStyleBackColor = true;
            // 
            // gbGroupPermissions
            // 
            this.gbGroupPermissions.Controls.Add(this.cbGroupExecute);
            this.gbGroupPermissions.Controls.Add(this.cbGroupWrite);
            this.gbGroupPermissions.Controls.Add(this.cbGroupRead);
            this.gbGroupPermissions.Location = new System.Drawing.Point(12, 63);
            this.gbGroupPermissions.Name = "gbGroupPermissions";
            this.gbGroupPermissions.Size = new System.Drawing.Size(195, 45);
            this.gbGroupPermissions.TabIndex = 1;
            this.gbGroupPermissions.TabStop = false;
            this.gbGroupPermissions.Text = "Group permissions";
            // 
            // cbGroupExecute
            // 
            this.cbGroupExecute.AutoSize = true;
            this.cbGroupExecute.Location = new System.Drawing.Point(121, 19);
            this.cbGroupExecute.Name = "cbGroupExecute";
            this.cbGroupExecute.Size = new System.Drawing.Size(65, 17);
            this.cbGroupExecute.TabIndex = 2;
            this.cbGroupExecute.Text = "Execute";
            this.cbGroupExecute.UseVisualStyleBackColor = true;
            // 
            // cbGroupWrite
            // 
            this.cbGroupWrite.AutoSize = true;
            this.cbGroupWrite.Location = new System.Drawing.Point(64, 19);
            this.cbGroupWrite.Name = "cbGroupWrite";
            this.cbGroupWrite.Size = new System.Drawing.Size(51, 17);
            this.cbGroupWrite.TabIndex = 1;
            this.cbGroupWrite.Text = "Write";
            this.cbGroupWrite.UseVisualStyleBackColor = true;
            // 
            // cbGroupRead
            // 
            this.cbGroupRead.AutoSize = true;
            this.cbGroupRead.Location = new System.Drawing.Point(6, 19);
            this.cbGroupRead.Name = "cbGroupRead";
            this.cbGroupRead.Size = new System.Drawing.Size(52, 17);
            this.cbGroupRead.TabIndex = 0;
            this.cbGroupRead.Text = "Read";
            this.cbGroupRead.UseVisualStyleBackColor = true;
            // 
            // gbPublicPermissions
            // 
            this.gbPublicPermissions.Controls.Add(this.checkBox4);
            this.gbPublicPermissions.Controls.Add(this.checkBox5);
            this.gbPublicPermissions.Controls.Add(this.checkBox6);
            this.gbPublicPermissions.Location = new System.Drawing.Point(12, 114);
            this.gbPublicPermissions.Name = "gbPublicPermissions";
            this.gbPublicPermissions.Size = new System.Drawing.Size(195, 45);
            this.gbPublicPermissions.TabIndex = 3;
            this.gbPublicPermissions.TabStop = false;
            this.gbPublicPermissions.Text = "Public permissions";
            // 
            // checkBox4
            // 
            this.checkBox4.AutoSize = true;
            this.checkBox4.Location = new System.Drawing.Point(121, 19);
            this.checkBox4.Name = "checkBox4";
            this.checkBox4.Size = new System.Drawing.Size(65, 17);
            this.checkBox4.TabIndex = 2;
            this.checkBox4.Text = "Execute";
            this.checkBox4.UseVisualStyleBackColor = true;
            // 
            // checkBox5
            // 
            this.checkBox5.AutoSize = true;
            this.checkBox5.Location = new System.Drawing.Point(64, 19);
            this.checkBox5.Name = "checkBox5";
            this.checkBox5.Size = new System.Drawing.Size(51, 17);
            this.checkBox5.TabIndex = 1;
            this.checkBox5.Text = "Write";
            this.checkBox5.UseVisualStyleBackColor = true;
            // 
            // checkBox6
            // 
            this.checkBox6.AutoSize = true;
            this.checkBox6.Location = new System.Drawing.Point(6, 19);
            this.checkBox6.Name = "checkBox6";
            this.checkBox6.Size = new System.Drawing.Size(52, 17);
            this.checkBox6.TabIndex = 0;
            this.checkBox6.Text = "Read";
            this.checkBox6.UseVisualStyleBackColor = true;
            // 
            // lblNumericValue
            // 
            this.lblNumericValue.AutoSize = true;
            this.lblNumericValue.Location = new System.Drawing.Point(15, 168);
            this.lblNumericValue.Name = "lblNumericValue";
            this.lblNumericValue.Size = new System.Drawing.Size(78, 13);
            this.lblNumericValue.TabIndex = 4;
            this.lblNumericValue.Text = "Numeric value:";
            // 
            // textBox1
            // 
            this.textBox1.Location = new System.Drawing.Point(107, 165);
            this.textBox1.Name = "textBox1";
            this.textBox1.Size = new System.Drawing.Size(100, 20);
            this.textBox1.TabIndex = 5;
            // 
            // btnOK
            // 
            this.btnOK.Location = new System.Drawing.Point(12, 194);
            this.btnOK.Name = "btnOK";
            this.btnOK.Size = new System.Drawing.Size(75, 23);
            this.btnOK.TabIndex = 6;
            this.btnOK.Text = "OK";
            this.btnOK.UseVisualStyleBackColor = true;
            // 
            // btnCancel
            // 
            this.btnCancel.Location = new System.Drawing.Point(133, 194);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(75, 23);
            this.btnCancel.TabIndex = 7;
            this.btnCancel.Text = "Cancel";
            this.btnCancel.UseVisualStyleBackColor = true;
            // 
            // frmFileProperties
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(218, 225);
            this.Controls.Add(this.btnCancel);
            this.Controls.Add(this.btnOK);
            this.Controls.Add(this.textBox1);
            this.Controls.Add(this.lblNumericValue);
            this.Controls.Add(this.gbPublicPermissions);
            this.Controls.Add(this.gbGroupPermissions);
            this.Controls.Add(this.gbOwnerPermissions);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.Name = "frmFileProperties";
            this.Text = "Properties";
            this.gbOwnerPermissions.ResumeLayout(false);
            this.gbOwnerPermissions.PerformLayout();
            this.gbGroupPermissions.ResumeLayout(false);
            this.gbGroupPermissions.PerformLayout();
            this.gbPublicPermissions.ResumeLayout(false);
            this.gbPublicPermissions.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.GroupBox gbOwnerPermissions;
        private System.Windows.Forms.CheckBox cbOwnerExecute;
        private System.Windows.Forms.CheckBox cbOwnerWrite;
        private System.Windows.Forms.CheckBox cbOwnerRead;
        private System.Windows.Forms.GroupBox gbGroupPermissions;
        private System.Windows.Forms.CheckBox cbGroupExecute;
        private System.Windows.Forms.CheckBox cbGroupWrite;
        private System.Windows.Forms.CheckBox cbGroupRead;
        private System.Windows.Forms.GroupBox gbPublicPermissions;
        private System.Windows.Forms.CheckBox checkBox4;
        private System.Windows.Forms.CheckBox checkBox5;
        private System.Windows.Forms.CheckBox checkBox6;
        private System.Windows.Forms.Label lblNumericValue;
        private System.Windows.Forms.TextBox textBox1;
        private System.Windows.Forms.Button btnOK;
        private System.Windows.Forms.Button btnCancel;
    }
}