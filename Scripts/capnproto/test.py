import capnp
import sys
import os
import getopt
import mirabuiltin_capnp
import fileexplorer_capnp
import socket
import argparse
import struct
import binascii

g_Version = "0.1-beta1"
g_Title = "mira.py (" + g_Version + ") - Python Mira connection library"

def OnLs():
    print("OnLs")

def OnExit():
    print("OnExit")
    sys.exit(0)

def splash():
    os.system("clear")
    print(g_Title)
    print("")

g_MenuOptions = [
    { "ls" : OnLs },
    { "exit" : OnExit }
]

class MiraConnection:
    m_Host = '127.0.0.1'
    m_Port = 9999
    m_Socket = None
    m_SendTimeout = 10 * 1000

    def __init__(self, p_Host, p_Port):
        self.m_Host = p_Host
        self.m_Port = p_Port
        self.m_Socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    def Connect(self, p_Timeout):
        if (self.m_Socket == None):
            return False
        
        self.m_Socket.connect((self.m_Host, self.m_Port))
        return True

    def Test(self, p_Text):
        s_Request = fileexplorer_capnp.EchoRequest.new_message()
        s_Request.message = p_Text

        s_Message = mirabuiltin_capnp.Message.new_message()
        s_Message.category = mirabuiltin_capnp.MessageCategory.file
        s_Message.type = 0xEBDB1342
        s_Message.containedMessage = s_Request.to_bytes()

        s_Data = s_Message.to_bytes()
        s_DataLen = struct.pack("<i", len(s_Data))
        f = open("out.bin", "wb")
        f.write(s_DataLen)
        f.write(s_Data)
        f.close()
        
        #print binascii.hexlify(s_DataLen)
        s_BytesSent = self.m_Socket.send(s_DataLen)
        print("bytesSent: " + str(s_BytesSent))
        s_BytesSent = self.m_Socket.send(s_Data)
        print("bytesSent: " + str(s_BytesSent))

def main(p_Argc, p_Args):
    # Check if we have the right number of arguments
    
    s_Parser = argparse.ArgumentParser()
    s_Parser.add_argument("ip", help="ip address of console")
    s_Parser.add_argument("port", help="the port of the daemon")

    s_Namespace, s_Args = s_Parser.parse_known_args()
    
    print(s_Namespace)
    print(s_Args)

    # Hold our default connection information
    s_IpAddress = s_Namespace.ip
    s_Port = int(s_Namespace.port)
    
    print("ip: " + s_IpAddress)
    print("port: " + str(s_Port))
    s_Connection = MiraConnection(s_IpAddress, s_Port)

    if s_Connection.Connect(500) == False:
        print("error connecting to console")
        return
    
    s_Connection.Test("capnproto echo test")

    # Loop our menu until we exit
    while True:
        #splash()

        # print menu
        for l_Item in g_MenuOptions:
            print(" [ " + str(g_MenuOptions.index(l_Item)) + " ] " + l_Item.keys()[0])
        
        # get user selection
        s_Selection = raw_input(">> ")
        try:
            s_SelectionIndex = int(s_Selection)
            if s_SelectionIndex < 0 : 
                raise ValueError
            g_MenuOptions[s_SelectionIndex].values()[0]()
        except (ValueError, IndexError):
            pass
    
    print("Done")


if __name__ == "__main__":
    main(len(sys.argv), sys.argv)
