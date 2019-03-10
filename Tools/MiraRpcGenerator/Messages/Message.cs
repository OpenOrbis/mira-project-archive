using System;
using System.Collections.Generic;
using System.Text;

namespace MiraRpcGenerator.Messages
{
    public class Message : MessageHeader
    {
        public List<byte> Payload;

        public Message()
        {
            Payload = new List<byte>();
        }
    }
}
