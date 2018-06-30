namespace MiraToolkit.Controls.Debugger
{
    partial class ucWatch
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
            this.lstValues = new System.Windows.Forms.ListView();
            this.chExpression = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.chExpressionValue = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.SuspendLayout();
            // 
            // lstValues
            // 
            this.lstValues.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.chExpression,
            this.chExpressionValue});
            this.lstValues.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lstValues.GridLines = true;
            this.lstValues.Location = new System.Drawing.Point(0, 0);
            this.lstValues.Name = "lstValues";
            this.lstValues.Size = new System.Drawing.Size(257, 433);
            this.lstValues.TabIndex = 0;
            this.lstValues.UseCompatibleStateImageBehavior = false;
            this.lstValues.View = System.Windows.Forms.View.Details;
            // 
            // chExpression
            // 
            this.chExpression.Text = "Expression";
            this.chExpression.Width = 120;
            // 
            // chExpressionValue
            // 
            this.chExpressionValue.Text = "Value";
            this.chExpressionValue.Width = 120;
            // 
            // ucWatch
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.lstValues);
            this.Name = "ucWatch";
            this.Size = new System.Drawing.Size(257, 433);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ListView lstValues;
        private System.Windows.Forms.ColumnHeader chExpression;
        private System.Windows.Forms.ColumnHeader chExpressionValue;
    }
}
