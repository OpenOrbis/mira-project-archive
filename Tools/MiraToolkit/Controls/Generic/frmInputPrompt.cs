using System;
using System.Windows.Forms;

namespace MiraToolkit.Controls.Generic
{
    public partial class frmInputPrompt : Form
    {
        /// <summary>
        /// Input string
        /// </summary>
        public string Input { get; set; }
        public frmInputPrompt()
        {
            InitializeComponent();
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.Cancel;
            Close();
        }
    }
}
