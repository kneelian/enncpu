#include "utility.hpp"

void RESOLVE_LABELS(
	std::vector<LINE>& lines,
	std::unordered_map<std::string, u32>& label_addresses)
{
	for(auto& i : lines)
	{
		if(i.OPERATION == LABEL)
			continue;
		if(i.LABELNAME.size() > 1)
		{
			if(label_addresses.contains(i.LABELNAME))
			{
				i.LABEL_POS = label_addresses[i.LABELNAME];
				i.LABEL_OFF = label_addresses[i.LABELNAME] - i.POSITION;
			}
			else
			{
				char REG_NAMES[] = "ABCDEFGH";
				std::printf("No such label %s in line @ 0x%06x: %s%s %c %c %s\n", 
					i.LABELNAME.c_str(), 
					i.POSITION,
					unmappings[i.OPERATION].c_str(),
					(i.PREDICATED)?".P":"",
					(i.REG_A != -1)? REG_NAMES[i.REG_A]:'\0',
					(i.REG_B != -1)? REG_NAMES[i.REG_B]:'\0',
					(i.IMMEDIATE != -1024)? std::to_string(i.IMMEDIATE).c_str(): " "
				);
				throw std::invalid_argument(" NO SUCH LABEL ");
			}
		}
	}
	for(auto & i : lines)
	{
		if(i.LABELNAME.size() < 2)
		{ continue; }
		switch(i.OPERATION)
		{
			case MVLA:
			case MVLB:
			case MVLC:
			case MVLD:
			case MVLE:
			case MVLF:
			case MVLG:
			case MVLH:
				i.IMMEDIATE = (i.LABEL_POS >>  0) & 0xff; break;

			case MVMA:
			case MVMB:
			case MVMC:
			case MVMD:
			case MVME:
			case MVMF:
			case MVMG:
			case MVMH:
				i.IMMEDIATE = (i.LABEL_POS >>  8) & 0xff; break;

			case MVHA:
			case MVHB:
			case MVHC:
			case MVHD:
			case MVHE:
			case MVHF:
			case MVHG:
			case MVHH:
				i.IMMEDIATE = (i.LABEL_POS >> 16) & 0xff; break;

			case JMFAR:
			case JLFAR:
				if((i.LABEL_OFF >= -8192) and (i.LABEL_OFF < 8192))
					i.IMMEDIATE = i.LABEL_OFF / 2;
				else
					{	
						std::printf("Label %s too far in line @ 0x%06x: %s%s %s\n", 
							i.LABELNAME.c_str(), 
							i.POSITION,
							unmappings[i.OPERATION].c_str(),
							(i.PREDICATED)?".P":"",
							i.LABELNAME.c_str()
						);
						throw std::invalid_argument(" LABEL TOO FAR ");
					}
				break;
			case JMA:
			case JLA:
				if(i.LABEL_POS < 512)
					i.IMMEDIATE = i.LABEL_POS / 2;
				else
				{	
					std::printf("Label %s too far in line @ 0x%06x: %s%s %s\n", 
						i.LABELNAME.c_str(), 
						i.POSITION,
						unmappings[i.OPERATION].c_str(),
						(i.PREDICATED)?".P":"",
						i.LABELNAME.c_str()
					);
					throw std::invalid_argument(" LABEL TOO FAR ");
				}
				break;
			case JMO:
			case JLO:
			case JMZO:
			case JLZO:
			case JMNZO:
			case JLNZO:
				if((i.LABEL_OFF >= -128) and (i.LABEL_OFF < 128))
					i.IMMEDIATE = i.LABEL_OFF;
				else
				{	
					std::printf("Label %s (0x%06x) too far in line @ 0x%06x: %s%s %s\n", 
						i.LABELNAME.c_str(),
						i.LABEL_POS,
						i.POSITION,
						unmappings[i.OPERATION].c_str(),
						(i.PREDICATED)?".P":"",
						i.LABELNAME.c_str()
					);
					throw std::invalid_argument(" LABEL TOO FAR ");
				}
				break;
			default:
				break;
		}
	}
}
