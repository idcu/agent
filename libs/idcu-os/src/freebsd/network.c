#include "idcu/os.h"

int idcu_sock_close(int sock) { (void)sock; return -1; }
int idcu_sock_bind(int sock, const char* addr, uint16_t port) { (void)sock; (void)addr; (void)port; return -1; }
int idcu_sock_listen(int sock, int backlog) { (void)sock; (void)backlog; return -1; }
int idcu_sock_accept(int sock, char* client_addr, size_t addr_len, uint16_t* client_port) { (void)sock; (void)client_addr; (void)addr_len; (void)client_port; return -1; }
int idcu_sock_connect(int sock, const char* addr, uint16_t port) { (void)sock; (void)addr; (void)port; return -1; }
int idcu_sock_send(int sock, const void* data, size_t len) { (void)sock; (void)data; (void)len; return -1; }
int idcu_sock_recv(int sock, void* data, size_t len) { (void)sock; (void)data; (void)len; return -1; }
int idcu_sock_set_nonblocking(int sock, bool nonblocking) { (void)sock; (void)nonblocking; return -1; }
