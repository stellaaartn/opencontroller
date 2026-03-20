#pragma once

// Connect to WiFi in station mode.
// Blocks until connected or times out.
// Returns 1 on success, 0 on failure.
int  wifi_connect(const char *ssid, const char *password);
void wifi_get_ip(char *buf, int buf_len); // fills buf with "192.168.x.x"
