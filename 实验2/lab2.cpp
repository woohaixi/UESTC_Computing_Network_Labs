#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// 1. 定义MAC地址结构体
typedef struct {
    unsigned char bytes[6]; // MAC地址由6个字节组成
} MacAddress;

// 2. 定义MAC表项结构体
typedef struct {
    MacAddress mac;
    int port;
} MacTableEntry;

// 生成随机MAC地址
MacAddress generateRandomMac() {
    MacAddress mac;
    for (int i = 0; i < 6; i++) {
        mac.bytes[i] = rand() % 256; // 生成0-255的随机数
    }

    // 确保MAC地址符合规范(第1字节最低位为0表示单播，第2位为0表示全局唯一)
    mac.bytes[0] &= 0xFE; // 确保最低位为0
    mac.bytes[0] |= 0x02; // 设置第二位为1表示本地管理地址

    return mac;
}

// 将MAC表写入文件
void writeMacAddressAndPortToFile(const char* filename, MacTableEntry* table, int size) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("无法打开文件 %s\n", filename);
        return;
    }

    fprintf(file, "MAC地址表\n");
    fprintf(file, "%-20s %s\n", "MAC地址", "端口号");
    fprintf(file, "--------------------- ------\n");

    for (int i = 0; i < size; i++) {
        // 格式化MAC地址为XX:XX:XX:XX:XX:XX
        fprintf(file, "%02X:%02X:%02X:%02X:%02X:%02X %6d\n",
            table[i].mac.bytes[0], table[i].mac.bytes[1],
            table[i].mac.bytes[2], table[i].mac.bytes[3],
            table[i].mac.bytes[4], table[i].mac.bytes[5],
            table[i].port);
    }

    fclose(file);
}

// 打印MAC表
void printMacTable(MacTableEntry* table, int size) {
    printf("\nMAC地址表内容:\n");
    printf("%-20s %s\n", "MAC地址", "端口号");
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
    // 初始化随机数种子
    srand(time(NULL));

    int tableSize;
    printf("请输入要生成的MAC表项数量: ");
    scanf_s("%d", &tableSize);

    // 创建并填充MAC表
    MacTableEntry* macTable = (MacTableEntry*)malloc(tableSize * sizeof(MacTableEntry));
    if (macTable == NULL) {
        printf("内存分配失败\n");
        return 1;
    }

    // 填充表项
    for (int i = 0; i < tableSize; i++) {
        macTable[i].mac = generateRandomMac();
        macTable[i].port = i + 1; // 端口号从1开始顺序排列
    }

    // 打印生成的MAC表
    printMacTable(macTable, tableSize);

    // 将MAC表写入文件
    writeMacAddressAndPortToFile("mac_table.txt", macTable, tableSize);
    printf("\nMAC地址表已写入文件 mac_table.txt\n");

    // 释放内存
    free(macTable);

    return 0;
}