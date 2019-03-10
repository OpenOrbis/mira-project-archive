using System;
using System.Windows.Forms;

namespace MiraUtils.Controls
{
    public partial class frmIpAddress : Form
    {
        public string Address = string.Empty;

        public frmIpAddress()
        {
            InitializeComponent();

            DialogResult = DialogResult.Cancel;
        }

        private void btnOk_Click(object sender, EventArgs e)
        {
            Address = txtAddress.Text;

            DialogResult = DialogResult.OK;
            Close();
        }
    }
}
