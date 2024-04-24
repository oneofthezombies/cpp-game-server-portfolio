#include <cstring>
#include <iostream>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "core/utils.h"

auto main(int argc, char **argv) noexcept -> int {
  const auto args = core::ParseArgcArgv(argc, argv);
  const char *server_ip = "127.0.0.1";
  const int server_port = 8000;

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    std::cerr << "Failed to create socket." << std::endl;
    return 1;
  }

  sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(server_port);
  inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

  if (connect(sock, (sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    std::cerr << "Failed to connect to the server." << std::endl;
    close(sock);
    return 1;
  }

  const char *message = "Hello, server!";
  if (send(sock, message, strlen(message), 0) == -1) {
    std::cerr << "Failed to send data." << std::endl;
    close(sock);
    return 1;
  }

  char buffer[1024] = {0};
  ssize_t bytes_received = recv(sock, buffer, sizeof(buffer), 0);
  if (bytes_received == -1) {
    const char *error_message = strerror(errno);

    std::cerr << error_message << std::endl;
    std::cerr << "Failed to receive data." << std::endl;
    close(sock);
    return 1;
  }

  std::cout << "Received message: " << buffer << std::endl;

  close(sock);
  return 0;
}
