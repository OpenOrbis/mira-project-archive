using MiraRpcGenerator.Messages;
using System;
using System.Text;

namespace MiraRpcGenerator
{
    class Generator
    {
        static void Main(string[] args)
        {
            Console.WriteLine("Hello World!");

            var s_Crc32 = new Crc32();

            var s_Hash1 = s_Crc32.Get("File");
            var s_Hash2 = s_Crc32.Get(MessageHeader.MessageCategory.File.ToString());

            var s_Header1 = new MessageHeader
            {
                Category = MessageHeader.MessageCategory.File,
                Type = s_Hash1,
                IsRequest = true,
                Magic = MessageHeader.c_Magic,
                PayloadLength = 0
            };

            var s_Header2 = new MessageHeader();
            s_Header2.Category = MessageHeader.MessageCategory.Command;
            s_Header2.Type = s_Hash2;
            s_Header2.IsRequest = true;
            s_Header2.Magic = MessageHeader.c_Magic;
            s_Header2.PayloadLength = 0;
        }
    }
}
