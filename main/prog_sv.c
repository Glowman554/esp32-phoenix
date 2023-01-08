#include <prog_sv.h>

#include <phoenix/cpu_core.h>

#include <netinet/in.h> //structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> //for socket APIs
#include <sys/types.h>
#include <stdint.h>

#include "esp_log.h"

static const char* TAG = "prog_sv";

#define CMD_READ 0x1
#define CMD_WRITE 0x2
#define CMD_SAVE 0x3
#define CMD_HALT 0x4
#define CMD_RST 0x5

const char* cmd_str[] =
{
    "?????",
    "read ",
    "write",
    "save ",
    "halt ",
    "rst  "
};

typedef struct command
{
    uint8_t cmd;
    uint16_t addr;
    uint8_t data;
} __attribute__((packed)) command_t;

extern bool pxhalt;
void px_save();

void sock_ack(int socket)
{
    uint8_t res = 1;
    send(socket, &res, 1, 0);
}

void init_prog_sv()
{
    int serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in serv_addr;
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(9001);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr));

    listen(serv_sock, 1);

    while (true)
    {
        int socket = accept(serv_sock, NULL, NULL);
        ESP_LOGI(TAG, "New client %d", socket);

        command_t cmd;
        while (recv(socket, &cmd, sizeof(command_t), 0) != 0)
        {
            ESP_LOGI(TAG, "command %s: 0x%x %d", cmd_str[cmd.cmd], cmd.addr, cmd.data);
            
            switch (cmd.cmd)
            {
                case CMD_READ:
                    uint8_t res = cpu_fetch_byte(cmd.addr);
                    send(socket, &res, sizeof(res), 0);
                    break;
                case CMD_WRITE:
                    cpu_write_byte(cmd.addr, cmd.data);
                    sock_ack(socket);
                    break;
                case CMD_SAVE:
                    px_save();
                    sock_ack(socket);
                    break;
                case CMD_HALT:
                    pxhalt = true;
                    sock_ack(socket);
                    break;
                case CMD_RST:
                    sock_ack(socket);
                    esp_restart();
                    break;

                default:
                    ESP_LOGE(TAG, "Unknown command %d", cmd.cmd);
                    close(socket);
                    break;
            }

        }

        close(socket);
    }
}
