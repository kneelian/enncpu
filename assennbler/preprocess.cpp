#include "utility.hpp"

#include <fstream>
#include <sstream>
#include <cctype>

// apologies to future readers, including future me
// for using bullshit names for temporaries

void PREPROCESS(std::vector<std::string>& raw_lines)
{
	std::vector<std::string> temp_vec;
	{
		std::vector<std::vector<std::string>> in_files;
		std::string temp_line;
		for(int j = 0; j < 6; j++)
		{
			for(auto & i : raw_lines)
			{
				if(i.size() > 2)
					while(i.size() >= 2 and (std::ispunct(i.back()) == 0 and std::isalnum(i.back()) == 0))
						i.pop_back();

				if(i.size() and i[0] == '$')
				{
					if(i.substr(0, 8) == "$include")
					{
						std::fstream f(i.substr(9, i.size()));
						if(f.is_open())
						{
							temp_vec.clear();
							std::string temp_line;
							while(std::getline(f, temp_line))
								temp_vec.push_back(temp_line);
							f.close();
							i = "=";
							in_files.push_back(temp_vec);
						}
					}
				}
			}
			do
			{
				int s = raw_lines.size();
				for(auto & k : in_files)
					s += k.size();
				raw_lines.reserve(s);
			} while(false);

			while(in_files.size())
			{
				for(auto j = raw_lines.rbegin(); j != raw_lines.rend(); j++)
				{
					if(*j != "=") continue;
					else
					{
						*j = "";
						raw_lines.insert(j.base(), in_files.back().begin(), in_files.back().end());
						break;
					}
				}
				in_files.pop_back();
			}
		}
	}
	{
		struct replacements { std::string from, to; };
		std::vector<replacements> reps;
		std::string a, b, c;
		for(auto& i : raw_lines)
		{
			std::istringstream iss(i);

			if(iss >> a)
			{
				if(a == "$undef")
					if(iss >> b)
					{
						for(auto& j : reps)
						{
							if(j.from == b) { j.from = ""; j.to = ""; }
							i = "";
							continue;
						}
					}

			 	if(a == "$def")
					if(iss >> b and iss >> c) 
						if(b.size() and c.size())
						{
							reps.push_back({b, c});
							i = "";
							continue;
						}
			}

			if(reps.size())
			{
				for(auto& j : reps)
				{
					size_t where = i.find(" " + j.from);
					if(where != std::string::npos)
					{
						std::printf("[%s]\n", i.c_str());
						std::string a = i.substr(0, where + 1);
						std::string b = i.substr(where + 1 + j.from.size(), i.size());
						std::printf("{%s}{%s}{%s}\n", a.c_str(), j.to.c_str(), b.c_str());
						i = a + j.to + b;
						std::printf("<%s>\n\n", i.c_str());
					}
				}
			}
		}
		reps.clear();
		a.clear();
		b.clear();
		c.clear();
	}

	{
		struct macro { std::string from; std::vector<std::string> to; int argc; };
		temp_vec.clear();
		std::vector<macro> macros;
		macro temp_macro;
		for(int i = 0; i < raw_lines.size(); i++)
		{
			std::istringstream iss(raw_lines[i]);
			std::string a, b, c;
			if(iss >> a)
			{
				if(a == "$macro")
				{
					if((iss >> b) and (iss >> c))
					{
						temp_macro.from = "_" + b;
						temp_macro.argc = (std::stoi(c) > 10)?10:std::stoi(c);
					}
					for(i = i + 1; i < raw_lines.size(); i++)
					{
						std::istringstream jss(raw_lines[i]);
						jss >> a;
						if(a != "$endm")
						{
							temp_vec.push_back(raw_lines[i]);
						}
						else
						{
							temp_macro.to = std::move(temp_vec);
							macros.push_back(temp_macro);
							goto bigbreak;
						}
					}
				}
			}
			bigbreak:;
		}
		{
			bool started_macro = false;
			for(auto& l : raw_lines)
			{
				if(l.size() and l.substr(0, 6) == "$macro")
				{
					l = "=";
					started_macro = true;
					continue;
				}
				else if(l.size() and l.substr(0, 5) == "$endm")
				{
					l = "=";
					started_macro = false;
					continue;
				}
				else if(started_macro)
				{ l = "="; continue; }
				else continue;
			}
		}

		if(macros.size())
		{
			for(auto & m : macros)
			{
				std::array<std::string, 10> macroargs;
				for(auto& s : macroargs) { s = ""; }

				for(int i = 0; i < raw_lines.size(); i++)
				{
					std::istringstream iss(raw_lines[i]);
					std::string a, b, c, d;
					// here is where we actually start macro substitution
					if((iss >> a) and (a == m.from)) 
					{
						for(int j = 0; j < m.argc and j < 10; j++)
							if(!(iss >> macroargs[j])) { macroargs[j] = ""; break; }
						for(int j = 0; j < i; j++)
							temp_vec.push_back(raw_lines[j]); // contains the code up to here
						
						for(int j = 0; j < m.to.size(); j++)
						{
							a = m.to[j];
							for(int k = 0; k < m.argc; k++)
								// disgusting gore to find mX for each numbered argument
								if(a.find( std::string("m") + char(k + '0') ) != std::string::npos)
									a.replace(a.find( std::string("m") + char(k + '0') ), 2, macroargs[k] );
							temp_vec.push_back(a);
						}
						for(int j = i + 1; j < raw_lines.size(); j++)
							temp_vec.push_back(raw_lines[j]); // contains the code beyond this point

						raw_lines = std::move(temp_vec);
						goto next_macro;
					}
				}
				next_macro:;
			}
		}
		macros.clear();
	}
	for(auto& j : raw_lines) { if((j == "=") or (j[0] == '$')) j = ""; }
}

//raw_lines.insert(j, temp_vec.begin(), temp_vec.end());	
