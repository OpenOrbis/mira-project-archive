using System;
using System.Windows.Forms;
using WeifenLuo.WinFormsUI.Docking;

namespace MiraToolkit
{
    public class StatusChangedEventArgs : EventArgs
    {
        public string Message;

        public int? Percent;
    }

    static class Program
    {
        public static DockPanel DockPanel;
        public static event StatusChangedEventHandler StatusChanged;
        public delegate void StatusChangedEventHandler(object sender, StatusChangedEventArgs e);

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new frmMain());
        }

        public static void SetStatus(string p_Message, int? p_Percent = null)
        {
            StatusChanged?.Invoke(null, new StatusChangedEventArgs
            {
                Message = p_Message,
                Percent = p_Percent
            });
        }
    }
}
