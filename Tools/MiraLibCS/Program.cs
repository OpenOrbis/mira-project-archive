using Google.Protobuf;
using MiraLibCS.Client.Extensions;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MiraLibCS
{
    class Program
    {
        public string c_GetDirEnts = "-getDirEnts:";

        static void Main(string[] args)
        {
            var s_DebugArgs = "-getDirEnts:/user -address:192.168.1.2 -port:9999".Split(null);

            var s_Address = "192.168.1.2";

            var s_Connection = new PbConnection(s_Address);

            if (!s_Connection.Connect())
            {
                Console.WriteLine($"could not connect to {s_Address}");
                return;
            }

            var s_DentList = s_Connection.GetDirEnts("/user");
        }


    }
}
