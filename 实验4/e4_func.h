#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#define UINT32_BYTE0(x) ((x >>  0) & 0x000000ff) /* 获取第0个字节 */
#define UINT32_BYTE1(x) ((x >>  8) & 0x000000ff) /* 获取第1个字节 */
#define UINT32_BYTE2(x) ((x >> 16) & 0x000000ff) /* 获取第2个字节 */
#define UINT32_BYTE3(x) ((x >> 24) & 0x000000ff) /* 获取第3个字节 */

#define MAX_IPS 10        		//路由器地址的最大数量
#define MAX_ROUTES 100    		//路由器中路由项的最大数量
#define MAX_ROUTERS 100 		//网络中路由器最大数量
#define MAX_HOP_NUM 15 			//路由的最大跳数
#define AUTHOR "your name"   //修改为你的学号姓名

// 定义用于存储单个IP地址和掩码的结构体
typedef struct {
	uint32_t interface_id; 		//网络接口ID
	uint32_t ip_address;   		//接口IP
	uint32_t subnet_mask;  		//掩码
} IPAndMask;

// 定义用于存储单个路由条目的结构体
typedef struct {
	uint32_t destination_network;  	//目标网络地址
	uint32_t destination_mask;     	//目标网络掩码
	uint32_t next_hop_ip;          	//下一跳转发地址
	uint32_t metric;               	//度量值
} RouteEntry;

// 定义用于存储路由器信息的结构体
typedef struct {
	IPAndMask ip_and_masks[MAX_IPS];	//ip地址列表
	int ip_and_masks_count;				//ip地址数量
	RouteEntry route_table[MAX_ROUTES];	//路由表
	int route_table_count;				//路由项数量
	char router_name[32]; 				// 路由器名称
} RouterInfo;

RouterInfo ROUTER_LIST[MAX_ROUTERS]; 	//全局变量，模拟路由器网络
int ROUTER_COUNT;  						//全局变量，记录路由器数量

// 函数：根据掩码长度计算掩码
uint32_t net_mask(int mask_len) {
	uint32_t mask = 0;
	int i;
	for (i = 0; i < mask_len; i++) {
		mask = (mask >> 1) | 0x80000000;
	}
	return mask;
}

// 函数：将32位无符号数转换为点分十进制字符串
int uint32_to_ip_string(uint32_t ip, char ipstr[]) {
	sprintf(ipstr, "%d.%d.%d.%d", UINT32_BYTE3(ip), UINT32_BYTE2(ip), UINT32_BYTE1(ip), UINT32_BYTE0(ip));
	return 0;
}

// 函数：将点分十进制字符串表示的IP地址转换为32位无符号整数
uint32_t ip_string_to_uint32(const char ipstr[]) {
	uint32_t result = 0;
	uint32_t ip[4] = { 0 };
	sscanf(ipstr, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
	if ((ip[0] < 256) && (ip[1] < 256) && (ip[2] < 256) && (ip[3] < 256)) {
		result = (ip[0] << 24) + (ip[1] << 16) + (ip[2] << 8) + ip[3];
	}
	else {
		printf("*Error! Not valid IP address:%s\n", ipstr);
		result = 0;
	}
	return result;
}

// ip无符号数转为字符串
void ip_to_string(uint32_t ipaddr, char ipstr[]) {
	union IPAddress {
		struct {
			uint8_t o1;
			uint8_t o2;
			uint8_t o3;
			uint8_t o4;
		} o;
		uint32_t ipv4;
	};
	union IPAddress ip;
	ip.ipv4 = ipaddr;
	sprintf(ipstr, "%u.%u.%u.%u", ip.o.o4, ip.o.o3, ip.o.o2, ip.o.o1);
}

// ip字符串转为无符号数
uint32_t string_to_ip(char ipstr[]) {
	union IPAddress {
		struct {
			uint8_t o1;
			uint8_t o2;
			uint8_t o3;
			uint8_t o4;
		} o;
		uint32_t ipv4;
	};
	union IPAddress ip;
	uint32_t n1, n2, n3, n4;
	sscanf(ipstr, "%u.%u.%u.%u\n", &n4, &n3, &n2, &n1);
	ip.o.o1 = (uint8_t)(n1 & 255);
	ip.o.o2 = (uint8_t)(n2 & 255);
	ip.o.o3 = (uint8_t)(n3 & 255);
	ip.o.o4 = (uint8_t)(n4 & 255);
	return ip.ipv4;
}

//函数：转换32位无符号数的字节序
uint32_t sw_byte_order(uint32_t ipaddr) {
	union IPAddress {
		struct {
			uint8_t o1;
			uint8_t o2;
			uint8_t o3;
			uint8_t o4;
		} o;
		uint32_t ipv4;
	};
	union IPAddress ip;
	uint8_t b;
	ip.ipv4 = ipaddr;
	b = ip.o.o1;
	ip.o.o1 = ip.o.o4;
	ip.o.o4 = b;
	b = ip.o.o2;
	ip.o.o2 = ip.o.o3;
	ip.o.o3 = b;
	return ip.ipv4;
}

// 函数：将32位无符号整数表示的IP地址转换为点分十进制字符串
int mask_len(uint32_t mask) {
	int i = 0;
	for (; i < 32 && ((mask >> i) & 1) == 0; i++);
	return 32 - i;
}

// 判断两个IP地址是否在同一网络内
int are_same_net(uint32_t ip1, uint32_t ip2, uint32_t mask) {
	return (ip1 & mask) == (ip2 & mask);
}

// 查找是否目标地址是否在路由器路由器的邻接网络内
uint32_t match_local_network(RouterInfo* router, const uint32_t dest_ip) {
	int i;
	for (i = 0; i < router->ip_and_masks_count; i++) {
		if (are_same_net(dest_ip, router->ip_and_masks[i].ip_address, router->ip_and_masks[i].subnet_mask)) {
			return router->ip_and_masks[i].ip_address;
		}
	}
	return 0;
}

// 查找最佳路由项并返回下一跳地址
uint32_t match_route(RouterInfo* router, const uint32_t dest_ip) {
	uint32_t best_match_network = 0; // 用于存储最佳匹配的目标网络
	uint32_t best_match_mask = 0; // 用于存储最佳匹配的目标网络掩码
	uint32_t best_next_hop = 0; // 用于存储最佳匹配的下一跳IP地址
	uint32_t best_metric = INT_MAX; // 初始化最佳度量值为最大整数
	int i;

	for (i = 0; i < router->route_table_count; i++) {
		RouteEntry* entry = &router->route_table[i];

		// 检查目标IP是否匹配该路由条目
		if ((dest_ip & entry->destination_mask) == entry->destination_network) {
			// 优先选择最长前缀匹配
			if (entry->destination_mask > best_match_mask) {
				best_match_mask = entry->destination_mask;
				best_next_hop = entry->next_hop_ip;
				best_metric = entry->metric;
			}
			// 如果前缀长度相同，选择度量值更小的
			else if (entry->destination_mask == best_match_mask) {
				if (entry->metric < best_metric) {
					best_next_hop = entry->next_hop_ip;
					best_metric = entry->metric;
				}
			}
		}
	}

	return best_next_hop;
}

// 查找路由器router中到达目标地址的最佳路由
uint32_t find_best_route(RouterInfo* router, const uint32_t dest_ip, uint32_t* localaddr) {
	*localaddr = match_local_network(router, dest_ip);
	if (*localaddr > 0) {
		return *localaddr;
	}
	else {
		return match_route(router, dest_ip);
	}
}

// 根据IP地址检索routerlist中的路由器
RouterInfo* trans_to_nexthop(uint32_t ip) {
	int i, j;
	for (i = 0; i < ROUTER_COUNT; i++) {
		for (j = 0; j < ROUTER_LIST[i].ip_and_masks_count; j++) {
			if (ip == ROUTER_LIST[i].ip_and_masks[j].ip_address) {
				return &ROUTER_LIST[i];
			}
		}
	}
	return NULL;
}

// 检索传输路径函数
int find_route_path(RouterInfo* router, const uint32_t dest_ip, uint32_t route_path[]) {
	int hopnum = 0;
	uint32_t current_ip = route_path[0]; // 起始IP
	RouterInfo* current_router = router;

	while (hopnum < MAX_HOP_NUM) {
		uint32_t localaddr;
		uint32_t next_hop = find_best_route(current_router, dest_ip, &localaddr);

		// 检查是否到达目标网络
		if (localaddr != 0) {
			route_path[hopnum++] = localaddr;
			break;
		}

		// 没有找到路由
		if (next_hop == 0) {
			break;
		}

		// 检查路由环路
		int i;
		for (i = 0; i < hopnum; i++) {
			if (route_path[i] == next_hop) {
				printf("Warning: Routing loop detected\n");
				return hopnum;
			}
		}

		route_path[hopnum++] = next_hop;
		current_router = trans_to_nexthop(next_hop);
		if (current_router == NULL) {
			break;
		}
	}

	return hopnum;
}
