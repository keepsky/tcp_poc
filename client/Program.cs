using System;
using System.Buffers.Binary;
using System.Net.Sockets;
using System.Text;
using System.Text.Json;
using System.Threading;

namespace tcp_poc_client
{
    class Type1 {
        public string time { get; set; }
        public byte num { get; set; }
        public string param { get; set; }
    }
    class Type2 {
        public ushort pm_num { get; set; }
        public byte status { get; set; }
    }
    class Type3 {
        public ushort signal_num { get; set; }
        public byte status { get; set; }
    }
    class Type4 {
        public ushort route_num { get; set; }
        public byte status { get; set; }
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
                byte[] lengthBuffer = new byte[4];

                while (true)
                {
                    // Send request message (1~4)
                    Console.WriteLine($"\nSending request for message type {type}...");
                    byte[] reqBuffer = new byte[4];
                    BinaryPrimitives.WriteInt32BigEndian(reqBuffer, type);
                    stream.Write(reqBuffer, 0, reqBuffer.Length);

                    // Receive length
                    int bytesRead = 0;
                    while (bytesRead < 4)
                    {
                        int read = stream.Read(lengthBuffer, bytesRead, 4 - bytesRead);
                        if (read == 0) throw new Exception("Server disconnected.");
                        bytesRead += read;
                    }

                    int jsonLength = BinaryPrimitives.ReadInt32BigEndian(lengthBuffer);
                    if (jsonLength < 0 || jsonLength > 1024 * 1024) throw new Exception("Invalid JSON length");

                    // Receive JSON string
                    byte[] jsonBuffer = new byte[jsonLength];
                    bytesRead = 0;
                    while (bytesRead < jsonLength)
                    {
                        int read = stream.Read(jsonBuffer, bytesRead, jsonLength - bytesRead);
                        if (read == 0) throw new Exception("Server disconnected.");
                        bytesRead += read;
                    }

                    string jsonString = Encoding.UTF8.GetString(jsonBuffer);
                    Console.WriteLine($"Received JSON:\n  {jsonString}");

                    // Deserialize to object and print
                    try
                    {
                        switch (type)
                        {
                            case 1:
                                var t1 = JsonSerializer.Deserialize<Type1>(jsonString);
                                Console.WriteLine($"Deserialized Object [Type1] -> time: {t1.time}, num: {t1.num}, param: {t1.param}");
                                break;
                            case 2:
                                var t2 = JsonSerializer.Deserialize<Type2>(jsonString);
                                Console.WriteLine($"Deserialized Object [Type2] -> pm_num: {t2.pm_num}, status: {t2.status}");
                                break;
                            case 3:
                                var t3 = JsonSerializer.Deserialize<Type3>(jsonString);
                                Console.WriteLine($"Deserialized Object [Type3] -> signal_num: {t3.signal_num}, status: {t3.status}");
                                break;
                            case 4:
                                var t4 = JsonSerializer.Deserialize<Type4>(jsonString);
                                Console.WriteLine($"Deserialized Object [Type4] -> route_num: {t4.route_num}, status: {t4.status}");
                                break;
                        }
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"JSON deserialization error: {ex.Message}");
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
