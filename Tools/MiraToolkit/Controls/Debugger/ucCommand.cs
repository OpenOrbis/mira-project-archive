using System.Windows.Forms;
using MiraToolkit.Core.Debugger;

namespace MiraToolkit.Controls.Debugger
{
    public partial class ucCommand : UserControl
    { 
        private MiraDebugger m_Debugger;

        public ucCommand(MiraDebugger p_Debugger)
        {
            InitializeComponent();

            m_Debugger = p_Debugger;
        }
    }
}
