#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8890

// Define structs
#pragma pack(push, 1)
struct type1 {
  char time[7];
  unsigned char num;
  char param[49];
};

struct type2 {
  unsigned short pm_num;
  unsigned char status;
};

struct type3 {
  unsigned short signal_num;
  unsigned char status;
};

struct type4 {
  unsigned short route_num;
  unsigned char status1 : 1;
  unsigned char status2 : 4;
  unsigned char status3 : 3;
};
#pragma pack(pop)

int main() {
  WSADATA wsaData;
  SOCKET listenSocket = INVALID_SOCKET, clientSocket = INVALID_SOCKET;
  struct sockaddr_in serverAddr, clientAddr;
  int clientAddrLen = sizeof(clientAddr);
  char recvbuf[4];
  int res;
  char json_payload[1024];

  // Initialize Winsock
  res = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (res != 0) {
    printf("WSAStartup failed with error: %d\n", res);
    return 1;
  }

  listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (listenSocket == INVALID_SOCKET) {
    printf("socket failed with error: %ld\n", WSAGetLastError());
    WSACleanup();
    return 1;
  }

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(PORT);

  if (bind(listenSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) ==
      SOCKET_ERROR) {
    printf("bind failed with error: %d\n", WSAGetLastError());
    closesocket(listenSocket);
    WSACleanup();
    return 1;
  }

  if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
    printf("listen failed with error: %d\n", WSAGetLastError());
    closesocket(listenSocket);
    WSACleanup();
    return 1;
  }

  printf("Server listening on port %d...\n", PORT);

  while (1) {
    clientSocket =
        accept(listenSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (clientSocket == INVALID_SOCKET) {
      printf("accept failed with error: %d\n", WSAGetLastError());
      closesocket(listenSocket);
      WSACleanup();
      return 1;
    }

    printf("Client connected: %s\n", inet_ntoa(clientAddr.sin_addr));

    while (1) {
      int bytesReceived = 0;
      while (bytesReceived < 4) {
        res = recv(clientSocket, recvbuf + bytesReceived, 4 - bytesReceived, 0);
        if (res > 0) {
          bytesReceived += res;
        } else if (res == 0) {
          printf("Client disconnected.\n");
          break;
        } else {
          printf("recv failed with error: %d\n", WSAGetLastError());
          break;
        }
      }
      if (res <= 0)
        break;

      int req_type = ntohl(*(int *)recvbuf);
      printf("Received request type: %d\n", req_type);

      json_payload[0] = '\0';

      switch (req_type) {
      case 1: {
        struct type1 t1;
        memcpy(t1.time, "1234567", 7);
        t1.num = 45;
        strncpy(t1.param, "This is dummy parameter for type1.", 49);

        snprintf(json_payload, sizeof(json_payload),
                 "{\"time\":\"%.7s\", \"num\":%u, \"param\":\"%.49s\"}",
                 t1.time, t1.num, t1.param);
        break;
      }
      case 2: {
        struct type2 t2;
        t2.pm_num = 1002;
        t2.status = 1;

        snprintf(json_payload, sizeof(json_payload),
                 "{\"pm_num\":%hu, \"status\":%u}", t2.pm_num, t2.status);
        break;
      }
      case 3: {
        struct type3 t3;
        t3.signal_num = 3003;
        t3.status = 2;

        snprintf(json_payload, sizeof(json_payload),
                 "{\"signal_num\":%hu, \"status\":%u}", t3.signal_num,
                 t3.status);
        break;
      }
      case 4: {
        struct type4 t4;
        t4.route_num = 4004;
        t4.status1 = 1;
        t4.status2 = 10;
        t4.status3 = 5;

        snprintf(json_payload, sizeof(json_payload),
                 "{\"route_num\":%hu, \"status1\":%u, \"status2\":%u, "
                 "\"status3\":%u}",
                 t4.route_num, t4.status1, t4.status2, t4.status3);
        break;
      }
      default:
        printf("Unknown condition %d\n", req_type);
        snprintf(json_payload, sizeof(json_payload), "{}");
        break;
      }

      int payload_len = (int)strlen(json_payload);
      int net_len = htonl(payload_len);

      send(clientSocket, (char *)&net_len, 4, 0);
      send(clientSocket, json_payload, payload_len, 0);

      printf("Sent %d bytes of JSON: %s\n", payload_len, json_payload);
    }
    closesocket(clientSocket);
  }

  closesocket(listenSocket);
  WSACleanup();
  return 0;
}
