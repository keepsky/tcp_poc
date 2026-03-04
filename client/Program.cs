using System;
using System.Buffers.Binary;
using System.IO;
using System.Net.Sockets;
using System.Threading;
using ProtoBuf;

namespace tcp_poc_client
{
    [ProtoContract]
    class Type1 {
        [ProtoMember(1)]
        public string time { get; set; }
        [ProtoMember(2)]
        public uint num { get; set; }
        [ProtoMember(3)]
        public string param { get; set; }
    }
    
    [ProtoContract]
    class Type2 {
        [ProtoMember(1)]
        public uint pm_num { get; set; }
        [ProtoMember(2)]
        public uint status { get; set; }
    }
    
    [ProtoContract]
    class Type3 {
        [ProtoMember(1)]
        public uint signal_num { get; set; }
        [ProtoMember(2)]
        public uint status { get; set; }
    }
    
    [ProtoContract]
    class Type4 {
        [ProtoMember(1)]
        public uint route_num { get; set; }
        [ProtoMember(2)]
        public uint status1 { get; set; }
        [ProtoMember(3)]
        public uint status2 { get; set; }
        [ProtoMember(4)]
        public uint status3 { get; set; }
    }

    class Program
    {
        static void Main(string[] args)
        {
            try
            {
                using TcpClient client = new TcpClient("127.0.0.1", 8890);
                using NetworkStream stream = client.GetStream();
                Console.WriteLine("Connected to server 127.0.0.1:8890.");

                int type = 1;

                while (true)
                {
                    // Send request message (1~4)
                    Console.WriteLine($"\nSending request for message type {type}...");
                    byte[] reqBuffer = new byte[4];
                    BinaryPrimitives.WriteInt32BigEndian(reqBuffer, type);
                    stream.Write(reqBuffer, 0, reqBuffer.Length);

                    // Receive length
                    byte[] lengthBuffer = new byte[4];
                    int bytesRead = 0;
                    while (bytesRead < 4)
                    {
                        int read = stream.Read(lengthBuffer, bytesRead, 4 - bytesRead);
                        if (read == 0) throw new Exception("Server disconnected.");
                        bytesRead += read;
                    }

                    int protoLength = BinaryPrimitives.ReadInt32BigEndian(lengthBuffer);
                    if (protoLength < 0 || protoLength > 1024 * 1024) throw new Exception("Invalid payload length");

                    // Receive Protobuf binary payload
                    byte[] payloadBuffer = new byte[protoLength];
                    bytesRead = 0;
                    while (bytesRead < protoLength)
                    {
                        int read = stream.Read(payloadBuffer, bytesRead, protoLength - bytesRead);
                        if (read == 0) throw new Exception("Server disconnected.");
                        bytesRead += read;
                    }

                    Console.WriteLine($"Received {protoLength} bytes of Protobuf data.");

                    using var ms = new MemoryStream(payloadBuffer);

                    // Deserialize to object and print
                    try
                    {
                        switch (type)
                        {
                            case 1:
                                var t1 = Serializer.Deserialize<Type1>(ms);
                                // For proto3, omitted values resolve to empty or 0. Make sure to handle null strings.
                                Console.WriteLine($"Deserialized Object [Type1] -> time: {t1?.time}, num: {t1?.num}, param: {t1?.param}");
                                break;
                            case 2:
                                var t2 = Serializer.Deserialize<Type2>(ms);
                                Console.WriteLine($"Deserialized Object [Type2] -> pm_num: {t2?.pm_num}, status: {t2?.status}");
                                break;
                            case 3:
                                var t3 = Serializer.Deserialize<Type3>(ms);
                                Console.WriteLine($"Deserialized Object [Type3] -> signal_num: {t3?.signal_num}, status: {t3?.status}");
                                break;
                            case 4:
                                var t4 = Serializer.Deserialize<Type4>(ms);
                                Console.WriteLine($"Deserialized Object [Type4] -> route_num: {t4?.route_num}, status1: {t4?.status1}, status2: {t4?.status2}, status3: {t4?.status3}");
                                break;
                        }
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"Protobuf deserialization error: {ex.Message}");
                    }

                    // Loop type 1->4 and delay
                    type = (type % 4) + 1;
                    Thread.Sleep(2000);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Connection Error: {ex.Message}");
            }
        }
    }
}
