#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#include "e4_func.h"

#define ROUTER_CONF_FILE "C:\\Users\\27400\\Desktop\\RouterConfig.TXT"
//=======================================================================================================

// 函数：添加IP地址和掩码到路由器信息结构体
void add_ip_and_mask( RouterInfo *router, uint32_t interfaceid, const char *ip_str, const char *mask_str) {
	if (router->ip_and_masks_count < MAX_IPS) {
		router->ip_and_masks[router->ip_and_masks_count].interface_id = interfaceid;
		router->ip_and_masks[router->ip_and_masks_count].ip_address = ip_string_to_uint32(ip_str);
		router->ip_and_masks[router->ip_and_masks_count].subnet_mask = ip_string_to_uint32(mask_str);
		router->ip_and_masks_count++;
	} else {
		printf("Error: Maximum number of IP addresses and masks reached.\n");
	}
}

// 函数：添加路由条目到路由表
void add_route( RouterInfo *router, const char dest_net_str[], const char dest_mask_str[], const char next_hop_str[], int metric) {
	if (router->route_table_count < MAX_ROUTES) {
		router->route_table[router->route_table_count].destination_network = ip_string_to_uint32(dest_net_str);
		router->route_table[router->route_table_count].destination_mask = ip_string_to_uint32(dest_mask_str);
		router->route_table[router->route_table_count].next_hop_ip = ip_string_to_uint32(next_hop_str);
		router->route_table[router->route_table_count].metric = metric;
		router->route_table_count++;
	} else {
		printf("Error: Maximum number of routes reached.\n");
	}
}

// 函数：打印路由器IP地址和掩码
void print_router_ips( RouterInfo *router) {
	int i;
	printf("%-16s%-16s%-16s\n", "Interface","Address", "Mask");
	for ( i = 0; i < router->ip_and_masks_count; i++) {
		char ip_str[16];
		char mask_str[16];
		uint32_to_ip_string(router->ip_and_masks[i].ip_address, ip_str);
		uint32_to_ip_string(router->ip_and_masks[i].subnet_mask, mask_str);
		printf("%-16u%-16s%-16s\n",router->ip_and_masks[i].interface_id,ip_str, mask_str);
	}
}

// 函数：打印路由表
void print_route_table( RouterInfo *router) {
	int i;
	printf("%-16s%-16s%-16s%-16s%-16s\n","","DestNet","NetMask","Gateway","Matric");
	for ( i = 0; i < router->route_table_count; i++) {
		char dest_net_str[16];
		char dest_mask_str[16];
		char next_hop_str[16];
		uint32_to_ip_string(router->route_table[i].destination_network, dest_net_str);
		uint32_to_ip_string(router->route_table[i].destination_mask, dest_mask_str);
		uint32_to_ip_string(router->route_table[i].next_hop_ip, next_hop_str);
		printf("%-16s%-16s%-16s%-16s%3d\n","",
		       dest_net_str, dest_mask_str, next_hop_str, router->route_table[i].metric);
	}
}

// 读取路由器配置文件的函数
int read_router_config(const char *filename, RouterInfo *router_list, int *router_count, int max_routers) {
	int router_index = 0, metric;
	uint32_t ip, mask, interfaceid;
	char ip_str[16], mask_str[16], next_hop_str[16], line[256];
	RouterInfo *current_router = NULL;

	FILE *file = fopen(filename, "r");
	if (!file) {
		printf("Failed to open file: %s\n",filename);
		return -1;
	}

	while (fgets(line, sizeof(line), file)) {
		// 跳过空行
		if (line[0] == '\0') {
			continue;
		}
		// 解析路由器名称
		if (strncmp(line, "ROUTER", 6) == 0) {
			if (router_index >= max_routers) {
				fprintf(stderr, "Exceeded maximum number of routers\n");
				fclose(file);
				return router_index;
			}
			sscanf(line + 7, "%s", router_list[router_index].router_name);
			current_router = &router_list[router_index++];
			current_router->ip_and_masks_count = 0;
			current_router->route_table_count = 0;
		}
		// 解析IP地址和掩码
		else if (strncmp(line, "ADDRESS", 7) == 0) {
			if (!current_router) {
				fprintf(stderr, "ADDRESS found outside of router context\n");
				continue;
			}
			sscanf(line + 8, "%u %s %s", &interfaceid,ip_str, mask_str);

			add_ip_and_mask(current_router,interfaceid, ip_str, mask_str);
		}
		// 解析路由表项
		else if (strncmp(line, "ROUTE", 5) == 0) {
			if (!current_router) {
				fprintf(stderr, "ROUTE found outside of router context\n");
				continue;
			}
			sscanf(line + 6, "%s %s %s %d", ip_str, mask_str, next_hop_str, &metric);
			add_route(current_router, ip_str, mask_str, next_hop_str, metric);
		}
	}

	*router_count = router_index;
	fclose(file);
	return router_index;
}
// 初始化网络路由器并打印路由器信息
int init_routers(const char * filename) {
	return read_router_config(filename, ROUTER_LIST, &ROUTER_COUNT, MAX_ROUTERS);
}
// 初始化网络路由器并打印路由器信息
int print_routers() {
	int i;
	for ( i = 0; i < ROUTER_COUNT; i++) {
		printf("Router %d: %s\n",i+1,ROUTER_LIST[i].router_name);
		print_router_ips(&ROUTER_LIST[i]);
		printf("\n");
		print_route_table(&ROUTER_LIST[i]);
		printf("----------------------------------------------------------------------\n");
	}
}

void print_ip_uint(const uint32_t ip) {
	char ip_str[16];
	uint32_to_ip_string(ip,ip_str);
	printf("%s\n",ip_str);
}

//============================================================================
int main() {
	uint32_t ip;
	uint32_t mask;
	uint32_t nexthop;
	uint32_t localip;
	uint32_t routepath[MAX_HOP_NUM] = {0};  // 途经的下一跳ip保存在route_path数组中，返回途经路由跳数。
	int hop_num;
	int i;
	int l;

	char ip_input[16] = {0};
	char ip_string[16] = {0};
	
	
	if (init_routers(ROUTER_CONF_FILE) < 0) {
		printf("Environment not prepared, exit.\n");
		return -1;
	};
	//print_routers();
	while (1){
		memset(ip_input, 0, 16);
		printf("----------------------------------------------------------------------\n");
		printf("Functions write by %s\n",AUTHOR);
		printf("Input \"exit\" to exit\n");
		printf("Input \"show\" or \"print\" to print routers\n");
		printf("Input a ip address to trace route\n");
		printf("Input:");
		scanf("%16s",ip_input);
		if(strncmp(ip_input,"exit",4)==0) {
			break;
		} else if ((strncmp(ip_input,"show",4)==0) || (strncmp(ip_input,"print",5)==0) ) {
			print_routers();
		} else {
			//ip = string_to_ip(ip_input);
			//ip_to_string(ip,ip_string);
			ip = ip_string_to_uint32(ip_input);
			uint32_to_ip_string(ip,ip_string);
			nexthop = find_best_route(&ROUTER_LIST[0],ip,&localip);
			printf("To IP:%s, Next hop: ",ip_string);
			print_ip_uint(nexthop);

			hop_num = find_route_path(&ROUTER_LIST[0], ip, routepath);
			printf("To IP:%s, Hops: %d\n",ip_string,hop_num);
			for (i=0; i<hop_num; i++) {
				//printf("%3d: ",i+1);
				print_ip_uint(routepath[i]);
			}
			if (hop_num == MAX_HOP_NUM) {
				printf("Max HOP(%d)...\n",MAX_HOP_NUM);
			}
			
		}
	}
	//getchar();
	return 0;


	print_ip_uint(net_mask(16));
	print_ip_uint(net_mask(22));
	print_ip_uint(net_mask(23));
	print_ip_uint(net_mask(24));
	printf("%lu ", ip_string_to_uint32("192.168.1.1"));
	print_ip_uint(ip_string_to_uint32("192.168.1.1"));
	printf("%lu ", ip_string_to_uint32("255.255.255.0"));
	print_ip_uint(ip_string_to_uint32("255.255.255.0"));
	printf("%lu ", ip_string_to_uint32("192.168.1.0"));
	print_ip_uint(ip_string_to_uint32("192.168.1.0"));

	mask = ip_string_to_uint32("255.0.0.0");
	l= mask_len(mask);
	printf("%lu, %d",mask,l);
	getchar();
	return 0;
}
