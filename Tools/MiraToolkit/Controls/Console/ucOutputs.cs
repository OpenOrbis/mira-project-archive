using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using MiraToolkit.Core;

namespace MiraToolkit.Controls.Console
{
    public partial class ucOutputs : WeifenLuo.WinFormsUI.Docking.DockContent
    {
        private MiraDevice m_Device;

        private TabPage m_MiraPage;
        private TextBox m_MiraOutput;

        public ucOutputs(MiraDevice p_Device)
        {
            InitializeComponent();

            m_Device = p_Device;
             
            // Create the main Mira logging page
            m_MiraPage = new TabPage
            {
                Text = "Mira Output",
            };

            m_MiraOutput = new TextBox
            {
                ReadOnly = true,
                Dock = DockStyle.Fill,
                Multiline = true
            };

            m_MiraPage.Controls.Add(m_MiraOutput);

            Program.StatusChanged += OnMiraStatuschanged;

            tcOutputs.TabPages.Add(m_MiraPage);

            PopulateOutputs();
        }

        private void OnMiraStatuschanged(object sender, StatusChangedEventArgs e)
        {
            m_MiraOutput.Invoke((MethodInvoker)delegate
            {
                if (e.Percent == null)
                    m_MiraOutput.AppendText($"[{DateTime.Now}] {e.Message} {Environment.NewLine}");
                else
                    m_MiraOutput.AppendText($"[{DateTime.Now}] [{e.Percent}/100] {e.Message} {Environment.NewLine}");
            });
        }

        private void PopulateOutputs()
        {
            var s_Pages = new List<TabPage>();

            foreach (var l_Console in m_Device.Consoles)
            {
                var l_Page = new TabPage
                {
                    Text = l_Console.ToString(),
                    Tag = l_Console,
                };

                var l_TextBox = new TextBox
                {
                    ReadOnly = true,
                    Dock = DockStyle.Fill,
                    Multiline = true
                };

                l_Page.Controls.Add(l_TextBox);

                s_Pages.Add(l_Page);
            }


        }
    }
}
