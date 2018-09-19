using System;
using System.Windows.Forms;

namespace MiraToolkit.Controls.Generic
{
    public partial class frmIpAddress : Form
    {
        public string IPAddress { get; set; }
        public frmIpAddress()
        {
            InitializeComponent();
        }

        private void btnOK_Click(object sender, EventArgs e)
        {
            IPAddress = txtAddress.Text;
            DialogResult = DialogResult.OK;
            Close();
        }

        private void txtAddress_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (e.KeyChar != Convert.ToChar(Keys.Enter))
                return;

            IPAddress = txtAddress.Text;
            DialogResult = DialogResult.OK;
            Close();
        }
    }
}
