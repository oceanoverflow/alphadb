#include "rpc.h"

rpc::~rpc() {}

template<typename T, typename R1, typename R2>
void rpc::set(const char* str, T* obj, std::function<void(T*, const R1&, R2&)> func)
{
	service_ = [this, obj, func]() {
		rpc_id_t rid;
		codec_ >> rid;
		R1 request;
		codec_.unmarshal(rid, request);
		R2 reply;
		func(obj, request, reply);
		codec_.marshal(rid, reply);
	};
}

void rpc::operator()()
{
    service_();
}

void rpc::set_codec(const codec& codec)
{
	codec_ = codec;
}

int rpc::hash(const char* str) const
{
    int hashcode = 0;
	char* p = const_cast<char *>(str);
	while (*p)
		hashcode ^= int(*p);
        p++;
	return hashcode;
}