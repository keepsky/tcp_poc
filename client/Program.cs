using System;
using System.Net.WebSockets;
using System.Text;
using System.Text.Json;
using System.Threading;
using System.Threading.Tasks;

namespace tcp_poc_client
{
    public class RequestHeader {
        public string msgType { get; set; }
        public string msgId { get; set; }
        public string timestamp { get; set; }
    }

    public class RequestOptions {
        public bool force { get; set; }
    }

    public class RequestBody {
        public string command { get; set; }
        public string routeId { get; set; }
        public string startSignal { get; set; }
        public string endSignal { get; set; }
        public RequestOptions options { get; set; }
    }

    public class ControlRequest {
        public RequestHeader header { get; set; }
        public RequestBody body { get; set; }
    }

    class Program
    {
        static async Task Main(string[] args)
        {
            var req = new ControlRequest {
                header = new RequestHeader {
                    msgType = "CONTROL_REQ",
                    msgId = "REQ_20260313_001",
                    timestamp = "2026-03-13T18:00:01Z"
                },
                body = new RequestBody {
                    command = "SET_ROUTE",
                    routeId = "R201",
                    startSignal = "S10",
                    endSignal = "S20",
                    options = new RequestOptions { force = false }
                }
            };

            var jsonOptions = new JsonSerializerOptions { WriteIndented = false };
            string reqJson = JsonSerializer.Serialize(req, jsonOptions);
            
            Console.WriteLine("Client started. Press Ctrl+C to terminate.");
            
            while (true)
            {
                try
                {
                    using var ws = new ClientWebSocket();
                    Uri serverUri = new Uri("ws://127.0.0.1:8890");
                    Console.WriteLine($"\n[INFO] Connecting to {serverUri}...");
                    
                    using var cts = new CancellationTokenSource(TimeSpan.FromSeconds(5));
                    await ws.ConnectAsync(serverUri, cts.Token);
                    Console.WriteLine("[INFO] Connected.");

                    while (ws.State == WebSocketState.Open)
                    {
                        byte[] reqBytes = Encoding.UTF8.GetBytes(reqJson);
                        await ws.SendAsync(new ArraySegment<byte>(reqBytes), WebSocketMessageType.Text, true, CancellationToken.None);
                        Console.WriteLine($"[SEND] {reqJson}");

                        byte[] recvBuffer = new byte[200 * 1024]; // 200K buffer to hold up to 150K payload sizes
                        
                        for (int i = 0; i < 9; i++) // 9 variable sizes from server
                        {
                            WebSocketReceiveResult result;
                            int offset = 0;
                            do {
                                result = await ws.ReceiveAsync(new ArraySegment<byte>(recvBuffer, offset, recvBuffer.Length - offset), CancellationToken.None);
                                offset += result.Count;
                            } while (!result.EndOfMessage);

                            if (result.MessageType == WebSocketMessageType.Close)
                            {
                                Console.WriteLine("[INFO] Server closed connection.");
                                break;
                            }

                            // Do not format/print the full JSON, just report its size and sequence
                            Console.WriteLine($"[RECV {i + 1}/9] 데이터 크기: {offset} bytes");
                        }

                        if (ws.State == WebSocketState.Open)
                        {
                            Console.WriteLine("[INFO] Wait 2 seconds before next loop...");
                            await Task.Delay(2000);
                        }
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"[ERROR] Connection Error: {ex.Message}");
                    Console.WriteLine("[INFO] Retrying in 5 seconds...");
                    await Task.Delay(5000);
                }
            }
        }
    }
}
