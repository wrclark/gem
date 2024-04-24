#ifndef REQUEST_H
#define REQUEST_H

struct request {
    char data[1024];
    unsigned int size;
};

struct resource {
    char data[1024];
    unsigned int size;
};

int req_valid(struct request *);
int req_resource(struct request *, struct resource *);

#endif
