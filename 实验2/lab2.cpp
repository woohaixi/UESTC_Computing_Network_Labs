#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// 1. ����MAC��ַ�ṹ��
typedef struct {
    unsigned char bytes[6]; // MAC��ַ��6���ֽ����
} MacAddress;

// 2. ����MAC����ṹ��
typedef struct {
    MacAddress mac;
    int port;
} MacTableEntry;

// �������MAC��ַ
MacAddress generateRandomMac() {
    MacAddress mac;
    for (int i = 0; i < 6; i++) {
        mac.bytes[i] = rand() % 256; // ����0-255�������
    }

    // ȷ��MAC��ַ���Ϲ淶(��1�ֽ����λΪ0��ʾ��������2λΪ0��ʾȫ��Ψһ)
    mac.bytes[0] &= 0xFE; // ȷ�����λΪ0
    mac.bytes[0] |= 0x02; // ���õڶ�λΪ1��ʾ���ع����ַ

    return mac;
}

// ��MAC��д���ļ�
void writeMacAddressAndPortToFile(const char* filename, MacTableEntry* table, int size) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("�޷����ļ� %s\n", filename);
        return;
    }

    fprintf(file, "MAC��ַ��\n");
    fprintf(file, "%-20s %s\n", "MAC��ַ", "�˿ں�");
    fprintf(file, "--------------------- ------\n");

    for (int i = 0; i < size; i++) {
        // ��ʽ��MAC��ַΪXX:XX:XX:XX:XX:XX
        fprintf(file, "%02X:%02X:%02X:%02X:%02X:%02X %6d\n",
            table[i].mac.bytes[0], table[i].mac.bytes[1],
            table[i].mac.bytes[2], table[i].mac.bytes[3],
            table[i].mac.bytes[4], table[i].mac.bytes[5],
            table[i].port);
    }

    fclose(file);
}

// ��ӡMAC��
void printMacTable(MacTableEntry* table, int size) {
    printf("\nMAC��ַ������:\n");
    printf("%-20s %s\n", "MAC��ַ", "�˿ں�");
    printf("--------------------- ------\n");

    for (int i = 0; i < size; i++) {
        printf("%02X:%02X:%02X:%02X:%02X:%02X %6d\n",
            table[i].mac.bytes[0], table[i].mac.bytes[1],
            table[i].mac.bytes[2], table[i].mac.bytes[3],
            table[i].mac.bytes[4], table[i].mac.bytes[5],
            table[i].port);
    }
}

int main() {
    // ��ʼ�����������
    srand(time(NULL));

    int tableSize;
    printf("������Ҫ���ɵ�MAC��������: ");
    scanf_s("%d", &tableSize);

    // ���������MAC��
    MacTableEntry* macTable = (MacTableEntry*)malloc(tableSize * sizeof(MacTableEntry));
    if (macTable == NULL) {
        printf("�ڴ����ʧ��\n");
        return 1;
    }

    // ������
    for (int i = 0; i < tableSize; i++) {
        macTable[i].mac = generateRandomMac();
        macTable[i].port = i + 1; // �˿ںŴ�1��ʼ˳������
    }

    // ��ӡ���ɵ�MAC��
    printMacTable(macTable, tableSize);

    // ��MAC��д���ļ�
    writeMacAddressAndPortToFile("mac_table.txt", macTable, tableSize);
    printf("\nMAC��ַ����д���ļ� mac_table.txt\n");

    // �ͷ��ڴ�
    free(macTable);

    return 0;
}