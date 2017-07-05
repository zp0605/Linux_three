

struct l3_driver;
struct l3_client {
   // struct l3_adapter    *adapter;    /* the adapter we sit on    */
    struct l3_driver    *driver;    /* and our access routines    */
    void  *driver_data;    /* private driver data        */
    struct list_head    list;  
};

struct l3_ops {
	int (*open)(struct l3_client *clnt);
       int (*command)(struct l3_client *clnt, int cmd, void *arg);
	void (*close)(struct l3_client *clnt);
};

struct l3_driver{
	char *name;
	int (*attach_client)(struct l3_client *clnt);
	void (*detach_client)(struct l3_client *clnt);
	struct l3_ops *ops; 
	struct module *owner;
	struct list_head    list; 
};

extern int l3_attach_client(struct l3_client *clnt, const char *adap, const char *name);
extern void l3_detach_client(struct l3_client *clnt);
extern int  l3_add_driver(struct l3_driver *l3_drv);
extern void l3_del_driver(struct l3_driver *l3_drv);





