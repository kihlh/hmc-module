#include <iostream>
#include <hmc-module.h>
#include <any>

int main()
{

	bool _1 = hmc_path_util::isNtPath(L"\\Device\\HarddiskVolume2");
	bool _2 = hmc_path_util::isNtPath(L"\\\\?\\\\Device\\HarddiskVolume2");
	bool _22 = hmc_path_util::isNtPath(L"\\\\?\\F:\\sss\\");

	bool _3 = hmc_path_util::isDosPath(L"\\\\?\\F:\\sss\\");
	bool _4 = hmc_path_util::isDosPath(L"F:\\sss\\");

	bool _5 = hmc_path_util::isUncPath(L"\\\\?\\F:\\sss\\");
	bool _6 = hmc_path_util::isUncPath(L"F:\\sss\\");

	bool _7 = hmc_path_util::isVolumePath(L"\\\\?\\F:\\sss\\123456789\\\\123456");
	bool _8 = hmc_path_util::isVolumePath(L"F:\\sss\\\\123456789\\\\123456");
	bool _9 = hmc_path_util::isVolumePath(L"\\Device\\HarddiskVolume2\\123456789\\\\123456");
	bool _10 = hmc_path_util::isVolumePath(L"\\\\?\\Volume{68de5ae6-0000-0000-0000-100000000000}\\123456789\\\\123456");
	bool _11 = hmc_path_util::isVolumePath(L"Volume{68de5ae6-0000-0000-0000-100000000000}\\123456789\\\\123456");

	auto dd = hmc_path_util::parseVolume(L"\\\\?\\Volume{68de5ae6-0000-0000-0000-100000000000}\\123456789\\\\123456");
	auto ddcs = hmc_path_util::parseVolume(L"\\\\?\\Volume{68de5ae6-0000-0000-0000-100000000000}\\123456789\\\\123456\\");

	auto dc = hmc_path_util::parseNt(L"\\\\?\\\\Device\\HarddiskVolume2");

	auto ddc = hmc_path_util::parseVolume(L"\\\\?\\Volume{68de5ae6-0000-0000-0000-100000000000}\\123456789\\\\123456\\fssdhg.cc");

	auto dcc = hmc_path_util::parseNt(L"\\\\?\\\\Device\\HarddiskVolume2\\123456789\\\\123456\\fssdhg.cc");


	auto dccc = hmc_path_util::parse(L"F:\\sss\\\\123456789\\\\123456.mp4");
	auto trr = hmc_path_util::formatSeparator(L"\\\\?\\\\Device\\HarddiskVolume2\\123456789\\\\123456\\fssdhg.cc");
	auto trrcb = hmc_path_util::formatSeparator(L"\\\\?\\\\Device\\HarddiskVolume2");
	auto trrc = hmc_path_util::formatSeparator(L"////\\\\?\\\\Device\\HarddiskVolume2\\123456789\\\\123456\\fssdhg.cc");
	auto trrcv = hmc_path_util::formatSeparator(L"D:\\/HarddiskVolume2\\123456789\\\\123456\\fssdhg.cc");
	auto trrv = hmc_path_util::formatSeparator(L"F:\\\\\\\\\\Device\\HarddiskVolume2\\//////123456789\\\\123456\\fssdhg.cc");
	auto trrcbc = hmc_path_util::formatSeparator(L"\\Device\\HarddiskVolume2");

	auto dcccc = hmc_path_util::parse(L"\\\\?\\F:\\sss\\\\123456789\\\\123456.mp4");
	return 0;
}
