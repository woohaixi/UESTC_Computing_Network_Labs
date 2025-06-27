#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define MAX_SEQ 1024
#define WINDOW_SIZE 5
#define MAX_DATA_SIZE 100
#define TIMEOUT 3
#define LOOP_DELAY 1

typedef struct {
    int seq_num;
    int ack_num;
    char data[MAX_DATA_SIZE];
    int data_size;
    bool is_ack;
    int send_time;
} Packet;

typedef struct {
    Packet packets[WINDOW_SIZE];
    int base;
    int next_seq_num;
} SendWindow;

typedef struct {
    Packet packets[WINDOW_SIZE];
    int expected_seq;
} ReceiveWindow;

void init_send_window(SendWindow* window) {
    window->base = 0;
    window->next_seq_num = 0;
    memset(window->packets, 0, sizeof(window->packets));
}

void init_receive_window(ReceiveWindow* window) {
    window->expected_seq = 0;
    memset(window->packets, 0, sizeof(window->packets));
}

Packet create_packet(int seq_num, int ack_num, const char* data, int data_size, bool is_ack) {
    Packet packet;
    packet.seq_num = seq_num;
    packet.ack_num = ack_num;
    packet.data_size = data_size;
    packet.is_ack = is_ack;
    packet.send_time = 0;

    if (data && data_size > 0) {
        // ȷ�����ݲ������������
        int copy_size = data_size < MAX_DATA_SIZE ? data_size : MAX_DATA_SIZE - 1;
        memcpy(packet.data, data, copy_size);
        packet.data[copy_size] = '\0'; // ȷ���ַ�����ֹ
        packet.data_size = copy_size;
    }
    else {
        packet.data[0] = '\0';
    }

    return packet;
}

bool send_packet(Packet packet) {
    if (rand() % 10 == 0) {
        if (packet.is_ack) {
            printf("ģ�ⶪ��: ���к�=%d, ȷ�Ϻ�=%d, ����=%s\n",
                packet.seq_num, packet.ack_num, packet.data);
        }
        else {
            printf("ģ�ⶪ��: ���к�=%d, ȷ�Ϻ�=%d, ����=%s\n",
                packet.seq_num, packet.ack_num, packet.data);
        }
        return false;
    }

    if (packet.is_ack) {
        printf("����ACK: ȷ�Ϻ�=%d\n", packet.ack_num);
    }
    else {
        printf("�������ݰ�: ���к�=%d, ����=%s\n",
            packet.seq_num, packet.data);
    }

    return true;
}

bool is_window_full(SendWindow* window) {
    return (window->next_seq_num - window->base) >= WINDOW_SIZE;
}

void process_ack(SendWindow* window, Packet ack_packet, int current_time) {
    int ack_num = ack_packet.ack_num;

    if (ack_num > window->base && ack_num <= window->next_seq_num) {
        printf("�յ���ЧACK: ȷ�Ϻ�=%d, ���ڻ���Ŵ�%d���µ�%d\n",
            ack_num, window->base, ack_num);

        window->base = ack_num;

        for (int i = 0; i < WINDOW_SIZE; i++) {
            if (window->base + i < window->next_seq_num) {
                window->packets[i].send_time = current_time;
            }
        }
    }
    else {
        printf("�յ���ʱACK: ȷ�Ϻ�=%d, ��ǰ���ڻ����=%d\n", ack_num, window->base);
    }
}

Packet process_data(ReceiveWindow* window, Packet data_packet) {
    Packet ack_packet;

    if (data_packet.seq_num == window->expected_seq) {
        printf("�յ��������ݰ�: ���к�=%d, ����=%s\n",
            data_packet.seq_num, data_packet.data);

        window->expected_seq++;
        ack_packet = create_packet(0, window->expected_seq, NULL, 0, true);
    }
    else if (data_packet.seq_num > window->expected_seq) {
        printf("�յ��������ݰ�: ���к�=%d, �������к�=%d\n",
            data_packet.seq_num, window->expected_seq);
        ack_packet = create_packet(0, window->expected_seq, NULL, 0, true);
    }
    else {
        printf("�յ��ظ����ݰ�: ���к�=%d, �������к�=%d\n",
            data_packet.seq_num, window->expected_seq);
        ack_packet = create_packet(0, window->expected_seq, NULL, 0, true);
    }

    return ack_packet;
}

void check_and_retransmit(SendWindow* window, int current_time) {
    for (int i = 0; i < WINDOW_SIZE; i++) {
        if (window->base + i < window->next_seq_num) {
            Packet* pkt = &window->packets[i];
            if (current_time - pkt->send_time > TIMEOUT) {
                printf("��⵽��ʱ: ���к�=%d, �����ʱ��=%d, ��ǰʱ��=%d\n",
                    pkt->seq_num, pkt->send_time, current_time);

                if (send_packet(*pkt)) {
                    pkt->send_time = current_time;
                }
            }
        }
    }
}

int main() {
    SendWindow send_window;
    ReceiveWindow receive_window;
    Packet packets_to_send[20];
    int total_packets = 0;
    int sent_packets = 0;
    int received_acks = 0;
    int current_time = 0;

    srand(time(NULL));

    init_send_window(&send_window);
    init_receive_window(&receive_window);

    for (int i = 0; i < 20; i++) {
        char data[50];
        sprintf(data, "Data-%d", i);
        packets_to_send[i] = create_packet(i, 0, data, strlen(data) + 1, false);
        total_packets++;
    }

    printf("==== TCP�Զ������ģ�⿪ʼ ====\n");

    while (received_acks < total_packets) {
        printf("\n==== ʱ��� %d ====\n", current_time);

        while (!is_window_full(&send_window) && sent_packets < total_packets) {
            Packet packet = packets_to_send[sent_packets];
            packet.send_time = current_time;

            int window_index = sent_packets % WINDOW_SIZE;
            send_window.packets[window_index] = packet;

            send_packet(packet);

            sent_packets++;
            send_window.next_seq_num++;
        }

        if (sent_packets > receive_window.expected_seq) {
            int random_index = receive_window.expected_seq + (rand() % 3);
            if (random_index < sent_packets) {
                Packet data_packet = packets_to_send[random_index];

                if (send_packet(data_packet)) {
                    Packet ack_packet = process_data(&receive_window, data_packet);
                    send_packet(ack_packet);

                    if (rand() % 10 > 1) {
                        process_ack(&send_window, ack_packet, current_time);
                        received_acks = send_window.base;
                    }
                }
            }
        }

        check_and_retransmit(&send_window, current_time);

        printf("��ǰ״̬: ���ʹ���=[%d-%d), �ѷ���=%d, ��ȷ��=%d, ��������=%d\n",
            send_window.base, send_window.next_seq_num, sent_packets,
            received_acks, receive_window.expected_seq);

        current_time += LOOP_DELAY;

        if (current_time > 100) {
            printf("����: ģ��ʱ����������ܽ�����ѭ����ǿ���˳�\n");
            break;
        }
    }

    printf("\n==== ģ����� ====\n");
    printf("�������ݰ��ѳɹ����Ͳ�ȷ�ϣ�\n");

    return 0;
}