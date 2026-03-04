#include <stdint.h>
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

// --- Protobuf Encoder Helpers ---
int encode_varint(uint8_t *buffer, uint64_t value) {
  int len = 0;
  while (value > 0x7F) {
    buffer[len++] = (uint8_t)((value & 0x7F) | 0x80);
    value >>= 7;
  }
  buffer[len++] = (uint8_t)(value & 0x7F);
  return len;
}

int encode_tag(uint8_t *buffer, uint32_t field_num, uint8_t wire_type) {
  return encode_varint(buffer, (field_num << 3) | wire_type);
}

int encode_uint32_field(uint8_t *buffer, uint32_t field_num, uint32_t value) {
  if (value == 0)
    return 0; // omit default value in proto3
  int len = 0;
  len += encode_tag(buffer + len, field_num, 0); // wire type 0: varint
  len += encode_varint(buffer + len, value);
  return len;
}

int encode_string_field(uint8_t *buffer, uint32_t field_num, const char *str,
                        int str_len) {
  if (str_len == 0)
    return 0; // omit default value in proto3
  int len = 0;
  len +=
      encode_tag(buffer + len, field_num, 2); // wire type 2: length delimited
  len += encode_varint(buffer + len, str_len);
  memcpy(buffer + len, str, str_len);
  len += str_len;
  return len;
}
// ---------------------------------

int main() {
  WSADATA wsaData;
  SOCKET listenSocket = INVALID_SOCKET, clientSocket = INVALID_SOCKET;
  struct sockaddr_in serverAddr, clientAddr;
  int clientAddrLen = sizeof(clientAddr);
  char recvbuf[4];
  int res;
  uint8_t payload_buffer[1024];

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

    // Use inet_ntop to avoid deprecation warnings if desired, or keep as is for
    // PoC
    char ipstr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
    printf("Client connected: %s\n", ipstr);

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

      int payload_len = 0;

      switch (req_type) {
      case 1: {
        struct type1 t1;
        memset(&t1, 0, sizeof(t1));
        memcpy(t1.time, "12:34:5", 7);
        t1.num = 45;
        strncpy_s(t1.param, sizeof(t1.param),
                  "This is dummy parameter for type1 protobuf.", _TRUNCATE);

        int param_len = (int)strnlen(t1.param, sizeof(t1.param));
        int time_len = 7;

        payload_len += encode_string_field(payload_buffer + payload_len, 1,
                                           t1.time, time_len);
        payload_len +=
            encode_uint32_field(payload_buffer + payload_len, 2, t1.num);
        payload_len += encode_string_field(payload_buffer + payload_len, 3,
                                           t1.param, param_len);
        break;
      }
      case 2: {
        struct type2 t2;
        t2.pm_num = 1002;
        t2.status = 1;

        payload_len +=
            encode_uint32_field(payload_buffer + payload_len, 1, t2.pm_num);
        payload_len +=
            encode_uint32_field(payload_buffer + payload_len, 2, t2.status);
        break;
      }
      case 3: {
        struct type3 t3;
        t3.signal_num = 3003;
        t3.status = 2;

        payload_len +=
            encode_uint32_field(payload_buffer + payload_len, 1, t3.signal_num);
        payload_len +=
            encode_uint32_field(payload_buffer + payload_len, 2, t3.status);
        break;
      }
      case 4: {
        struct type4 t4;
        memset(&t4, 0, sizeof(t4));
        t4.route_num = 4004;
        t4.status1 = 1;
        t4.status2 = 10;
        t4.status3 = 5;

        payload_len +=
            encode_uint32_field(payload_buffer + payload_len, 1, t4.route_num);
        payload_len +=
            encode_uint32_field(payload_buffer + payload_len, 2, t4.status1);
        payload_len +=
            encode_uint32_field(payload_buffer + payload_len, 3, t4.status2);
        payload_len +=
            encode_uint32_field(payload_buffer + payload_len, 4, t4.status3);
        break;
      }
      default:
        printf("Unknown condition %d\n", req_type);
        break;
      }

      int net_len = htonl(payload_len);

      // Send length prefix
      send(clientSocket, (char *)&net_len, 4, 0);

      // Send payload if length > 0
      if (payload_len > 0) {
        send(clientSocket, (char *)payload_buffer, payload_len, 0);
      }

      printf("Sent %d bytes of Protobuf data.\n", payload_len);
    }
    closesocket(clientSocket);
  }

  closesocket(listenSocket);
  WSACleanup();
  return 0;
}
