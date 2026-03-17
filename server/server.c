#include "mongoose.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

static const char *s_listen_on = "ws://0.0.0.0:8890";

// Ordered sizes: 0.5K, 1K, 2K, 10K, 50K, 75K, 100K, 150K, 1K
static const int sizes[] = {512, 1024, 2048, 10240, 51200, 76800, 102400, 153600, 1024};
static const int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

static const char *base_start = "{\"type\":\"STATUS_UPDATE\",\"timestamp\":\"2026-03-13T18:00:05Z\",\"payload\":{\"signals\":[{\"id\":\"S10\",\"aspect\":\"PROCEED\",\"locked\":true},{\"id\":\"S20\",\"aspect\":\"STOP\",\"locked\":false}],\"points\":[{\"id\":\"PM_101\",\"position\":\"NORMAL\",\"status\":\"LOCKED\"}],\"tracks\":[{\"id\":\"T101\",\"occupied\":false},{\"id\":\"T102\",\"occupied\":true}]}, \"pad\":\"";
static const char *base_end = "\"}";

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

    // Send JSON responses of varying sizes
    printf("Sending %d JSON responses of varying sizes...\n", num_sizes);
    
    size_t start_len = strlen(base_start);
    size_t end_len = strlen(base_end);

    for(int i = 0; i < num_sizes; i++) {
        size_t target_size = sizes[i];
        if (target_size < start_len + end_len) {
            target_size = start_len + end_len; // enforce minimum size constraint
        }
        
        char *buf = malloc(target_size + 1);
        if (buf) {
            memcpy(buf, base_start, start_len);
            size_t pad_len = target_size - start_len - end_len;
            memset(buf + start_len, 'A', pad_len);
            memcpy(buf + start_len + pad_len, base_end, end_len);
            buf[target_size] = '\0';
            
            mg_ws_send(c, buf, target_size, WEBSOCKET_OP_TEXT);
            free(buf);
        }
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
