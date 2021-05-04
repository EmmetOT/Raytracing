#ifndef BEAUTIFY_DURATION_H
#define BEAUTIFY_DURATION_H

#include <chrono>
#include <sstream>
#include <iomanip>

// source: https://stackoverflow.com/a/46134506/4731782
std::string beautify_duration(std::chrono::milliseconds input_milliseconds)
{
	using namespace std::chrono;
	typedef duration<int, std::ratio<86400>> days;
	auto d = duration_cast<days>(input_milliseconds);
	input_milliseconds -= d;
	auto h = duration_cast<hours>(input_milliseconds);
	input_milliseconds -= h;
	auto m = duration_cast<minutes>(input_milliseconds);
	input_milliseconds -= m;
	auto s = duration_cast<seconds>(input_milliseconds);

	auto dc = d.count();
	auto hc = h.count();
	auto mc = m.count();
	auto sc = s.count();

	std::stringstream ss;
	ss.fill('0');
	if (dc)
	{
		ss << d.count() << "d";
	}
	if (dc || hc)
	{
		if (dc) { ss << std::setw(2); } //pad if second set of numbers
		ss << h.count() << "h";
	}
	if (dc || hc || mc)
	{
		if (dc || hc) { ss << std::setw(2); }
		ss << m.count() << "m";
	}
	if (dc || hc || mc || sc)
	{
		if (dc || hc || mc) { ss << std::setw(2); }
		ss << s.count() << 's';
	}

	return ss.str();
}

std::string beautify_duration(int milliseconds)
{
	return beautify_duration(std::chrono::milliseconds(milliseconds));
}

#endif