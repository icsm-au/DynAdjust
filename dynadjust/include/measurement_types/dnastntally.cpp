//============================================================================
// Name         : dnastntally.cpp
// Author       : Roger Fraser
// Contributors :
// Version      : 1.00
// Copyright    : Copyright 2017 Geoscience Australia
//
//                Licensed under the Apache License, Version 2.0 (the "License");
//                you may not use this file except in compliance with the License.
//                You may obtain a copy of the License at
//               
//                http ://www.apache.org/licenses/LICENSE-2.0
//               
//                Unless required by applicable law or agreed to in writing, software
//                distributed under the License is distributed on an "AS IS" BASIS,
//                WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//                See the License for the specific language governing permissions and
//                limitations under the License.
//
// Description  : DynAdjust station tally implementation file
//============================================================================

#include <include/measurement_types/dnastntally.hpp>

namespace dynadjust {
namespace measurements {

StnTally::StnTally() {
	initialise();
}

void StnTally::initialise() {
	CCC = FFF = CCF = CFF = FFC = FCC = CFC = FCF = 0;
}

StnTally& StnTally::operator+=(const StnTally& rhs) {
	CCC += rhs.CCC;
	FFF += rhs.FFF;
	CCF += rhs.CCF;
	CFF += rhs.CFF;
	FFC += rhs.FFC;
	FCC += rhs.FCC;
	CFC += rhs.CFC;
	FCF += rhs.FCF;
	return *this;
}

StnTally& StnTally::operator-=(const StnTally& rhs) {
	CCC -= rhs.CCC;
	FFF -= rhs.FFF;
	CCF -= rhs.CCF;
	CFF -= rhs.CFF;
	FFC -= rhs.FFC;
	FCC -= rhs.FCC;
	CFC -= rhs.CFC;
	FCF -= rhs.FCF;
	return *this;
}

const StnTally StnTally::operator+(const StnTally& rhs) const {
	StnTally t = *this;
	t += rhs;
	return t;
}

const StnTally StnTally::operator-(const StnTally& rhs) const {
	StnTally t = *this;
	t -= rhs;
	return t;
}

UINT32 StnTally::TotalCount() {
	return CCC + FFF + CCF + CFF + FFC + FCC + CFC + FCF;
}

void StnTally::addstation(const std::string& constraint) {
		
	if (boost::iequals(constraint, "CCC"))
		CCC += 1;
	else if (boost::iequals(constraint, "FFF"))
		FFF += 1;
	else if (boost::iequals(constraint, "CCF"))
		CCF += 1;
	else if (boost::iequals(constraint, "CFF"))
		CFF += 1;
	else if (boost::iequals(constraint, "FFC"))
		FFC += 1;
	else if (boost::iequals(constraint, "FCC"))
		FCC += 1;
	else if (boost::iequals(constraint, "CFC"))
		CFC += 1;
	else if (boost::iequals(constraint, "FCF"))
		FCF += 1;
}
	

void StnTally::removestation(const std::string& constraint) {

	if (boost::iequals(constraint, "CCC"))
	{
		if (CCC > 0)
			CCC -= 1;
	}
	else if (boost::iequals(constraint, "FFF"))
	{
		if (FFF > 0)
			FFF -= 1;
	}
	else if (boost::iequals(constraint, "CCF"))
	{
		if (CCF > 0)
			CCF -= 1;
	}
	else if (boost::iequals(constraint, "CFF"))
	{
		if (CFF > 0)
			CFF -= 1;
	}
	else if (boost::iequals(constraint, "FFC"))
	{
		if (FFC > 0)
			FFC -= 1;
	}
	else if (boost::iequals(constraint, "FCC"))
	{
		if (FCC > 0)
			FCC -= 1;
	}
	else if (boost::iequals(constraint, "CFC"))
	{
		if (CFC > 0)
			CFC -= 1;
	}
	else if (boost::iequals(constraint, "FCF"))
	{
		if (FCF > 0)
			FCF -= 1;
	}
}
	

void StnTally::coutSummary(std::ostream &os, const std::string& title) 
{
	// Print title
	os << title << " " << TotalCount() << " stations:" << std::endl;
	UINT32 i, w(PRINT_VAR_PAD+NUMERIC_WIDTH);
	// Print line
	os << " ";
	for (i=0; i<w; ++i)
		os << "-";
	os << std::endl;
	// Print stations
	if (CCC)
		os << std::left << std::setw(PRINT_VAR_PAD) << "  (CCC) 3D constrained:" << std::right << std::setw(NUMERIC_WIDTH) << CCC << std::endl;
	if (FFF)
		os << std::left << std::setw(PRINT_VAR_PAD) << "  (FFF) 3D free:" << std::right << std::setw(NUMERIC_WIDTH) << FFF << std::endl;
	if (CCF)
		os << std::left << std::setw(PRINT_VAR_PAD) << "  (CCF) 2D constrained, 1D free:" << std::right << std::setw(NUMERIC_WIDTH) << CCF << std::endl;
	if (FFC)
		os << std::left << std::setw(PRINT_VAR_PAD) << "  (FFC) 2D free, 1D constrained:" << std::right << std::setw(NUMERIC_WIDTH) << FFC << std::endl;
	if (FCF)
		os << std::left << std::setw(PRINT_VAR_PAD) << "  (FCF) custom 2D constraints:" << std::right << std::setw(NUMERIC_WIDTH) << FCF << std::endl;
	if (CFF)
		os << std::left << std::setw(PRINT_VAR_PAD) << "  (CFF) custom 2D constraints:" << std::right << std::setw(NUMERIC_WIDTH) << CFF << std::endl;
	if (CFC)
		os << std::left << std::setw(PRINT_VAR_PAD) << "  (CFC) custom 3D constraints:" << std::right << std::setw(NUMERIC_WIDTH) << CFC << std::endl;
	os << " ";
	for (i=0; i<w; ++i)
		os << "-";
	os << std::endl;
	os << std::left << std::setw(PRINT_VAR_PAD) << "  Total" << std::right << std::setw(NUMERIC_WIDTH) << TotalCount() << std::endl;
}
	
void StnTally::CreateTally(const vdnaStnPtr& vStations)	{
	initialise();		
	for (_it_vdnastnptr_const _it_stn=vStations.begin(); _it_stn!=vStations.end(); ++_it_stn)
		addstation(_it_stn->get()->GetConstraints());
}	


}	// namespace measurements
}	// namespace dynadjust


