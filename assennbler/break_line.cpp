#include "utility.hpp"

void BREAK_LINES(std::vector<std::vector<std::string>>& lines,  std::string& in)
{
	std::istringstream iss(in);
	std::string part;

	bool found_comment = false;

	lines.push_back({});

	while(iss >> part)
	{
		if(!part.empty() && part.back() == ',')
			part.pop_back();
		if(!part.empty() && part.find(';') != std::string::npos)
		{
			part = part.substr(0, part.find(';'));
			found_comment = true;
		}
		if(found_comment) { break; }
		lines.back().push_back(part);
	}
	for(auto i = lines.begin(); i != lines.end();)
	{
		if( (*i).size() == 0 ) { i = lines.erase(i); }
		else ++i;
	}
}
