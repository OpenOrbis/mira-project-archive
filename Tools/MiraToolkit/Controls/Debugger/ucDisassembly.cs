using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace MiraToolkit.Controls.Debugger
{
    public partial class ucDisassembly : WeifenLuo.WinFormsUI.Docking.DockContent
    {
        private const ulong c_BadAddr = 0xFFFFFFFFFFFFFFFF;
        private const string c_UnknownInstruction = "??";

        private ListViewItem[] m_Window;

        public int WindowSize = 50;
        
        public ucDisassembly()
        {
            InitializeComponent();

            m_Window = new ListViewItem[WindowSize];

            foreach (var l_Item in m_Window)
            {
                l_Item.Text = c_BadAddr.ToString("X");
                l_Item.SubItems.Add(new ListViewItem.ListViewSubItem(l_Item, c_UnknownInstruction));
            }
        }
    }
}
