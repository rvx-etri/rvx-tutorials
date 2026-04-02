#include "ervp_printf.h"

#include "ervp_assert.h"
#include "ervp_platform_controller_memorymap_offset.h"
#include "ervp_platform_controller_api.h"
#include "ervp_mmio_util.h"
#include "ervp_multicore_synch.h"

#include "munoc_api.h"

#include <string.h>

#define NAME_STRING_SIZE 16
#define TYPE_STRING_SIZE 8

struct NodeInfo
{
	int node_id;
	char node_name[NAME_STRING_SIZE];
	char type[TYPE_STRING_SIZE];
};

static int initilized = 0;
static int num_master = -1;
static int num_slave = -1;
struct NodeInfo ip_info[2][32];

char temp_string[32];

////////////////////////////////////////////////////////////////////////////////////

static void __initialize_info();
//static void check_ip_status(int type, int index);
//static void change_monitor_state_(int enable);

////////////////////////////////////////////////////////////////////////////////////

int get_node_id()
{
	return REG32(NOC_MMAP(MMAP_OFFSET_INFO_SELF_ID));
}

void print_noc_version()
{
	mmio_read_string(NOC_MMAP(MMAP_OFFSET_INFO_VERSION0),temp_string,NAME_STRING_SIZE);
	printf("\n[NOC:INFO] %s", temp_string);
}

void print_noc_config()
{
	mmio_read_string(NOC_MMAP(MMAP_OFFSET_INFO_CONFIG0),temp_string,NAME_STRING_SIZE);
	printf("\n[NOC:INFO] Config: %s", temp_string);
}

void print_noc_ip_number()
{
	int num_master, num_slave;
	num_master = mmio_read_data(NOC_MMAP(MMAP_OFFSET_INFO_NUM_MASTER));
	num_slave = mmio_read_data(NOC_MMAP(MMAP_OFFSET_INFO_NUM_SLAVE));
	printf("\n[NOC:INFO] master: %d, slave: %d", num_master, num_slave);
}

void print_noc_info()
{
	print_noc_version();
	print_noc_config();
	print_noc_ip_number();
}

const char* get_type_name(int type)
{
	static const char master_name[7] = "master";
	static const char slave_name[6] = "slave";
	const char* result = NULL;
	if(type==MASTER_IP)
		result = master_name;
	else if(type==SLAVE_IP)
		result = slave_name;
	return result;
}

void __initialize_ip_info(int type, int index)
{
	ip_info[type][index].node_id = mmio_read_data(NI_MMAP(type,index,MMAP_OFFSET_CONTROLLER_NODEID));
	if(ip_info[type][index].node_id!=index)
	{
		printf_must("\n[RVX/NOC/ERROR] %s %d has MISMATCHED node id %d", get_type_name(type), index, ip_info[type][index].node_id);
		exit(ERROR_SYSTEM_NOC);
	}
	mmio_read_string(NI_MMAP(type,index,MMAP_OFFSET_CONTROLLER_NAME0),ip_info[type][index].node_name,NAME_STRING_SIZE);
	mmio_read_string(NI_MMAP(type,index,MMAP_OFFSET_CONTROLLER_TYPE0),ip_info[type][index].type,TYPE_STRING_SIZE);
}

void __initialize_info()
{
	int i;
	if(initilized==0)
	{
		num_master = mmio_read_data(NOC_MMAP(MMAP_OFFSET_INFO_NUM_MASTER));
		num_slave = mmio_read_data(NOC_MMAP(MMAP_OFFSET_INFO_NUM_SLAVE));
		for(i=0; i<num_master; i++)
			__initialize_ip_info(MASTER_IP,i);
		for(i=0; i<num_slave; i++)
			__initialize_ip_info(SLAVE_IP,i);
		initilized = 1;
	}
}

void print_ip_name_(int type, int index)
{
	printf("\n[NOC:INFO] %s %d: %s", get_type_name(type), ip_info[type][index].node_id, ip_info[type][index].node_name);
}

void print_ip_list()
{
	int i;
	__initialize_info();
	for(i=0; i<num_master; i++)
		print_ip_name_(MASTER_IP,i);
	for(i=0; i<num_slave; i++)
		print_ip_name_(SLAVE_IP,i);
}

void check_ip_status(int type, int index)
{
	__initialize_info();
	printf("\nMonitor Enabled: %d", mmio_read_data(NI_MMAP(type,index,MMAP_OFFSET_CONTROLLER_MONITOR_ENABLE)));
	printf("\nExclude: %d", mmio_read_data(NI_MMAP(type,index,MMAP_OFFSET_CONTROLLER_EXCLUDE)));
	printf("\nSend State: %d", mmio_read_data(NI_MMAP(type,index,MMAP_OFFSET_CONTROLLER_FNI_STATE)));
	printf("\nReceive State: %d", mmio_read_data(NI_MMAP(type,index,MMAP_OFFSET_CONTROLLER_BNI_STATE)));
	printf("\nIP Timeout: %d", mmio_read_data(NI_MMAP(type,index,MMAP_OFFSET_CONTROLLER_IP_TIMEOUT)));
	printf("\nProtocol Violation: %d", mmio_read_data(NI_MMAP(type,index,MMAP_OFFSET_CONTROLLER_PROTOCOL_VIOLATION)));
	printf("\nRouting Error: %d", mmio_read_data(NI_MMAP(type,index,MMAP_OFFSET_CONTROLLER_ROUTING_ERROR)));
	printf("\nNoC Timeout: %d", mmio_read_data(NI_MMAP(type,index,MMAP_OFFSET_CONTROLLER_NOC_TIMEOUT)));
	printf("\n");
}

/*
int get_number_of_master()
{
	__initialize_info();
	return num_master;
}

int get_number_of_slave()
{
	__initialize_info();
	return num_slave;
}

void print_monitor_status()
{
	int i;
	__initialize_info();
	for(i=0; i<num_master; i++)
	{
		printf("\n[NOC:INFO] %s Monitor: ", master_info[i].node_name);
		if(mmio_read_data(MNI_MMAP(i,MMAP_OFFSET_CONTROLLER_MONITOR_ENABLE))==1)
			printf("Enabled");
		else
			printf("Disabled");
	}
	for(i=0; i<num_slave; i++)
	{
		printf("\n[NOC:INFO] %s Monitor: ", slave_info[i].node_name);
		if(mmio_read_data(SNI_MMAP(i,MMAP_OFFSET_CONTROLLER_MONITOR_ENABLE))==1)
			printf("Enabled");
		else
			printf("Disabled");
	}
}

void change_monitor_state_(int enable)
{
	int i;
	__initialize_info();
	for(i=0; i<num_master; i++)
		mmio_write_data(MNI_MMAP(i,MMAP_OFFSET_CONTROLLER_MONITOR_ENABLE),enable);
	for(i=0; i<num_slave; i++)
		mmio_write_data(SNI_MMAP(i,MMAP_OFFSET_CONTROLLER_MONITOR_ENABLE),enable);
	print_monitor_status();
}

void disable_monitor(){ change_monitor_state_(0); }

void disable_ip(char* name)
{
	int i;
	int done = 0;
	__initialize_info();
	for(i=0; i<num_master; i++)
	{
		if(done)
			break;
		if(strcmp(name,master_info[i].node_name)==0)
		{
			mmio_write_data(MNI_MMAP(i,MMAP_OFFSET_CONTROLLER_EXCLUDE),1);
			done = 1;
		}
	}
	for(i=0; i<num_slave; i++)
	{
		if(done)
			break;
		if(strcmp(name,slave_info[i].node_name)==0)
		{
			mmio_write_data(SNI_MMAP(i,MMAP_OFFSET_CONTROLLER_EXCLUDE),1);
			done = 1;
		}
	}
	if(done)
		printf("\n[NOC:INFO] IP \"%s\" is disabled", name);
	else
		printf("\n[NOC:INFO] No IP has a name \"%s\"", name);
}

// This does NOT work due to hardware limitation
//void enable_monitor(){ change_monitor_state_(1); } 

void check_undefined_access()
{
	int i;
	int num = mmio_read_data(NOC_MMAP(MMAP_OFFSET_ELOG_COUNT));
	int node_id;
	__initialize_info();
	printf("\n[NOC:INFO] Number of Undefined Region Access: %d\n", num);
	for(i=0; i<num; i++)
	{
		node_id = mmio_read_data(ELOG_MMAP(i,MMAP_OFFSET_ELOG_MASTER));
		printf("\n[NOC:INFO] Master %s", master_info[node_id].node_name);
		if(mmio_read_data(ELOG_MMAP(i,MMAP_OFFSET_ELOG_VALID))==1)
		{
			if(ELOG_MMAP(i,MMAP_OFFSET_ELOG_RW))
				printf(", Write");
			else
				printf(", Read");
			printf(", Addr: 0x%x", mmio_read_data(ELOG_MMAP(i,MMAP_OFFSET_ELOG_ADDR)));
		}
		else
			printf_must("\n[RVX/NOC/ERROR] Invalid info, which may be a bug");
	}
}
*/

void check_noc_status()
{
	int i;
	__initialize_info();
	//check_undefined_access();
	for(i=0; i<num_master; i++)
	{
		print_ip_name_(MASTER_IP,i);
		check_ip_status(MASTER_IP,i);
	}
	for(i=0; i<num_slave; i++)
	{
		print_ip_name_(SLAVE_IP,i);
		check_ip_status(SLAVE_IP,i);
	}
}
