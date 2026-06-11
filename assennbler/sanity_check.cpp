#include "utility.hpp"

void SANITY_CHECK(std::vector<LINE>& lines)
{
	int error = 0;
	int linenr = -1;
	for(auto& i : lines)
	{
		linenr++;
		if(i.OPERATION > 0x8000) continue;
		
		if(i.OPERATION == FMADD_48 and i.REG_C == -127)
		{
			std::printf("Too few arguments in line @ 0x%06x: %s%s %c, %c, %c\n",
				i.POSITION,
				unmappings[i.OPERATION].c_str(),
				(i.PREDICATED)?".P":"",
				(i.REG_A != -127)? i.REG_A + 'A':'x',
				(i.REG_B != -127)? i.REG_B + 'A':'x',
				(i.REG_C != -127)? i.REG_C + 'A':'x'
			);
			error = 1;
		}
		if(i.REG_A != -127 and i.REG_B != -127 and i.IMMEDIATE != -1024)
		{
			std::printf("SANITY: Too many arguments in line @ 0x%06x: %s%s %c, %c, #%d\n",
				i.POSITION,
				unmappings[i.OPERATION].c_str(),
				(i.PREDICATED)?".P":"",
				(i.REG_A != -127)? i.REG_A + 'A':'\0',
				(i.REG_B != -127)? i.REG_B + 'A':'\0',
				i.IMMEDIATE
			);
			error = 2;
		}
		if((i.OPERATION != JMFAR and i.OPERATION != JLFAR) and 
		   (i.IMMEDIATE != -1024) and (i.IMMEDIATE >= 512 or i.IMMEDIATE <= -512))
		{
			std::printf("SANITY: Immediate too large in line @ 0x%06x: %s%s %c, #%d\n",
				i.POSITION,
				unmappings[i.OPERATION].c_str(),
				(i.PREDICATED)?".P":"",
				(i.REG_A != -127)? i.REG_A + 'A':'\0',
				i.IMMEDIATE
			);
			error = 3;
		}
		if(
			(i.LABEL_POS >= 0)  and
		   !(i.OPERATION >= MVLA and i.OPERATION <= MVHH) and 
		   !(i.OPERATION == JMA or i.OPERATION == JLA or
		     i.OPERATION == JMFAR or i.OPERATION == JLFAR) and
			(i.LABEL_OFF != -(1 << 24)) and (i.LABEL_OFF >= 512 or i.LABEL_OFF <= -512)
		) {
			std::printf("SANITY: Label @%s too far away in line @ 0x%06x: %s%s %c, #%d\n",
				i.LABELNAME.c_str(),
				i.POSITION,
				unmappings[i.OPERATION].c_str(),
				(i.PREDICATED)?".P":"",
				(i.REG_A != -127)? i.REG_A + 'A':'\0',
				i.IMMEDIATE
			);
			error = 4;
		}
		if(
			(i.LABEL_POS >= 0)  and
		   !((i.OPERATION >= MVLA and i.OPERATION <= MVHH) or
		     i.OPERATION != JMA or i.OPERATION != JLA or
		     i.OPERATION != JMFAR or i.OPERATION != JLFAR) and 
			(i.LABEL_OFF >= 32 or i.LABEL_OFF <= -32)
		) {
			std::printf("SANITY: Label @%s potentially too far away in line @ 0x%06x: %s%s %c, #%d\n",
				i.LABELNAME.c_str(),
				i.POSITION,
				unmappings[i.OPERATION].c_str(),
				(i.PREDICATED)?".P":"",
				(i.REG_A != -127)? i.REG_A + 'A':'\0',
				i.IMMEDIATE
			);
		}
	}
	if(error) throw std::invalid_argument(" SANITY CHECK FAILED. LAST ERROR = " + std::to_string(error));
}
