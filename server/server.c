#include "mongoose.h"
#include <stdio.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

static const char *s_listen_on = "ws://0.0.0.0:8890";

static const char *json_response = 
"{\n"
"  \"type\": \"STATUS_UPDATE\",\n"
"  \"timestamp\": \"2026-03-13T18:00:05Z\",\n"
"  \"payload\": {\n"
"    \"signals\": [\n"
"      { \"id\": \"S10\", \"aspect\": \"PROCEED\", \"locked\": true },\n"
"      { \"id\": \"S20\", \"aspect\": \"STOP\", \"locked\": false }\n"
"    ],\n"
"    \"points\": [\n"
"      { \"id\": \"PM_101\", \"position\": \"NORMAL\", \"status\": \"LOCKED\" }\n"
"    ],\n"
"    \"tracks\": [\n"
"      { \"id\": \"T101\", \"occupied\": false },\n"
"      { \"id\": \"T102\", \"occupied\": true }\n"
"    ]\n"
"  }\n"
"}";

// Event handler function
static void fn(struct mg_connection *c, int ev, void *ev_data) {
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    if (mg_match(hm->uri, mg_str("/"), NULL)) {
      // Upgrade to WebSocket
      mg_ws_upgrade(c, hm, NULL);
    } else {
      mg_http_reply(c, 404, "", "Not Found\n");
    }
  } else if (ev == MG_EV_WS_MSG) {
    struct mg_ws_message *wm = (struct mg_ws_message *) ev_data;
    printf("Received Request: %.*s\n", (int) wm->data.len, wm->data.buf);

    // Send 10 JSON responses
    printf("Sending 10 JSON responses...\n");
    for(int i = 0; i < 10; i++) {
        mg_ws_send(c, json_response, strlen(json_response), WEBSOCKET_OP_TEXT);
    }
  }
}

int main(void) {
  struct mg_mgr mgr;
  mg_mgr_init(&mgr);
  printf("Starting WS Server on %s\n", s_listen_on);
  mg_http_listen(&mgr, s_listen_on, fn, NULL);
  for (;;) mg_mgr_poll(&mgr, 100);
  mg_mgr_free(&mgr);
  return 0;
}
