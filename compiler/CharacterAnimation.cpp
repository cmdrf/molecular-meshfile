/*	CharacterAnimation.cpp

MIT License

Copyright (c) 2018 Fabian Herb

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "CharacterAnimation.h"

namespace CharacterAnimation
{

const Hash kBoneNameToIndex[] =
{
	"root"_H,
	"pelvis"_H,
	"spine01"_H,
	"spine02"_H,
	"spine03"_H,
	"spine04"_H,
	"neck"_H,
	"head"_H,
	"lookIK"_H,
	"lookIK_start"_H,
	"L_clavicle"_H,
	"L_upperarm"_H,
	"L_forearm"_H,
	"L_hand"_H,
	"L_thumb_01"_H,
	"L_thumb_02"_H,
	"L_thumb_03"_H,
	"L_thumb_end"_H,
	"L_index_01"_H,
	"L_index_02"_H,
	"L_index_03"_H,
	"L_index_end"_H,
	"L_middle_01"_H,
	"L_middle_02"_H,
	"L_middle_03"_H,
	"L_middle_end"_H,
	"L_ring_01"_H,
	"L_ring_02"_H,
	"L_ring_03"_H,
	"L_ring_end"_H,
	"L_pinky_01"_H,
	"L_pinky_02"_H,
	"L_pinky_03"_H,
	"L_pinky_end"_H,
	"RT_weapon_L_default_target"_H,
	"L_forearm_twist"_H,
	"L_upperarm_twist"_H,
	"R_clavicle"_H,
	"R_upperarm"_H,
	"R_forearm"_H,
	"R_hand"_H,
	"R_thumb_01"_H,
	"R_thumb_02"_H,
	"R_thumb_03"_H,
	"R_thumb_04"_H,
	"R_index_01"_H,
	"R_index_02"_H,
	"R_index_03"_H,
	"R_index_04"_H,
	"R_middle_01"_H,
	"R_middle_02"_H,
	"R_middle_03"_H,
	"R_middle_04"_H,
	"R_ring_01"_H,
	"R_ring_02"_H,
	"R_ring_03"_H,
	"R_ring_04"_H,
	"R_pinky_01"_H,
	"R_pinky_02"_H,
	"R_pinky_03"_H,
	"R_pinky_04"_H,
	"RT_weapon_R_default_target"_H,
	"R_forearm_twist"_H,
	"R_upperarm_twist"_H,
	"L_thigh"_H,
	"L_calf"_H,
	"L_foot"_H,
	"L_toe"_H,
	"L_toe_end"_H,
	"planeTargetLeft"_H,
	"L_Heel"_H,
	"L_thigh_twist"_H,
	"R_thigh"_H,
	"R_calf"_H,
	"R_foot"_H,
	"R_toe"_H,
	"R_toe_end"_H,
	"planeTargetRight"_H,
	"R_Heel"_H,
	"R_thigh_twist"_H
};

int GetBoneIndex(Hash hash)
{
	for(int i = 0; i < sizeof(kBoneNameToIndex); i++)
	{
		if(kBoneNameToIndex[i] == hash)
			return i;
	}
	return -1;
}

}
