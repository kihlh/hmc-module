#include <iostream>
#include <hmc-module.h>
#include <any>

int main()
{

	std::any data = (unsigned long)184467440000076ul;

	auto dd = hmc_util::anyCast(&data);
	
	auto dd0 = dd.get_int64();
	auto dd1 = dd.get_int32();
	auto dd2 = dd.get_uint32();
	auto dd3 = dd.get_uint64();

	return 0;
}
