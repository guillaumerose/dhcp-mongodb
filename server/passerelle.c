#include "dhcpd.h"
#include "mongo.h"

#define MONGO_HOST		"127.0.0.1"
#define MONGO_PORT		27017
#define MONGO_BASE		"production.users"

#define MONGO_MAC_FIELD		"mac"
#define	MONGO_DHCP_FIELD	"dhcp"

mongo_connection conn[1];
mongo_connection_options opts;

char *mongo_host = MONGO_HOST,
     *mongo_base = MONGO_BASE,
     *mongo_mac_field = MONGO_MAC_FIELD,
     *mongo_dhcp_field = MONGO_DHCP_FIELD;

int mongo_port = MONGO_PORT;

int
mongo_start() 
{	
	strncpy(opts.host, mongo_host, 255);
	opts.host[254] = '\0';
	opts.port = mongo_port;

	if (mongo_connect(conn, &opts)){
		printf("Failed to connect\n");
		return 0;
	}

	return 1;
}

int
find_haddr_in_mongo(struct host_decl **hp, int htype, unsigned hlen, 
		const unsigned char *haddr, const char *file, int line)
{
	struct host_decl * host;
	isc_result_t status;
	char *type_str;

	switch (htype) 
	{
		case HTYPE_ETHER:
			type_str = "ethernet";
			break;
		case HTYPE_IEEE802:
			type_str = "token-ring";
			break;
		case HTYPE_FDDI:
			type_str = "fddi";
			break;
		default:
			type_str = "something else";
	}

	printf("MAC = %s %s\n", type_str, print_hw_addr(htype, hlen, haddr));

	// Build request
	mongo_cursor *cursor = NULL;

	bson_buffer bb;
	bson query;
	bson result;

	bson_buffer_init(&bb);
	bson_append_string(&bb, mongo_mac_field, print_hw_addr(htype, hlen, haddr));
	bson_append_finish_object(&bb);

	bson_from_buffer(&query, &bb);

	bson_buffer_init(&bb);
	bson_append_string(&bb, mongo_dhcp_field, "");
	bson_append_finish_object(&bb);

	bson_from_buffer(&result, &bb);

	MONGO_TRY{
		cursor = mongo_find(conn, mongo_base, &query, &result, 0, 0, 0);
	}MONGO_CATCH{
		mongo_start();
		cursor = mongo_find(conn, mongo_base, &query, &result, 0, 0, 0);
		return 0;
	}
	
	char option_buffer[8196];

	if (mongo_cursor_next(cursor)) {
		bson_iterator it;
		bson_iterator_init(&it, cursor->current.data);

		while(bson_iterator_next(&it)) {
			if (bson_iterator_type(&it) == bson_string) {
				strncpy(option_buffer, bson_iterator_string(&it), bson_iterator_string_len(&it));
			}
		}

		printf("MAC found\n");
		mongo_cursor_destroy(cursor);
	}
	else {
		printf("MAC not found\n");
		mongo_cursor_destroy(cursor);
		return 0;
	}

	host = (struct host_decl *)0;

	status = host_allocate (&host, MDL);
	if (status != ISC_R_SUCCESS)
	{
		log_fatal ("can't allocate host decl struct: %s", 
				isc_result_totext (status)); 
		return (0);
	}

	host->name = "Testing";      
	if (host->name == NULL)
	{
		host_dereference (&host, MDL);
		return (0);
	}

	if (!clone_group (&host->group, root_group, MDL))
	{
		log_fatal ("can't clone group for host %s", host->name);
		host_dereference (&host, MDL);
		return (0);
	}

	int declaration, lease_limit;
	enum dhcp_token token;
	struct parse *cfile;
	isc_result_t res;
	const char *val;

	lease_limit = 1000;

	cfile = (struct parse *) NULL;
	res = new_parse (&cfile, -1, option_buffer, strlen(option_buffer), 
			"Testing", 0);

	if (res != ISC_R_SUCCESS)
		return (lease_limit);

	printf("Sending the following options: '%s'\n", option_buffer);

	declaration = 0;
	do
	{
		token = peek_token (&val, NULL, cfile);
		if (token == END_OF_FILE)
			break;
		declaration = parse_statement (cfile, host->group, HOST_DECL, host, declaration);
	} while (1);

	end_parse (&cfile);

	*hp = host;

	return (lease_limit);
}
