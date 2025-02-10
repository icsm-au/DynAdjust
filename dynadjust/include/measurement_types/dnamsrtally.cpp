//============================================================================
// Name         : dnamsrtally.cpp
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
// Description  : DynAdjust measurement tally implementation file
//============================================================================

#include <include/exception/dnaexception.hpp>
#include <include/measurement_types/dnameasurement.hpp>
#include <include/measurement_types/dnadirection.hpp>
#include <include/measurement_types/dnagpsbaseline.hpp>
#include <include/measurement_types/dnagpspoint.hpp>
#include <include/config/dnaconsts.hpp>

using namespace dynadjust::exception;

namespace dynadjust {
namespace measurements {

MsrTally::MsrTally()
	: SUPPORTED_MSR_COUNT(20) {
	initialise();
}

void MsrTally::FillMsrList(vchar& msr_list)
{
	msr_list.push_back('A'); // Horizontal angle
	msr_list.push_back('B'); // Geodetic azimuth
	msr_list.push_back('C'); // Chord dist
	msr_list.push_back('D'); // Direction set
	msr_list.push_back('E'); // Ellipsoid arc
	msr_list.push_back('G'); // GPS Baseline (treat as single-baseline cluster)
	msr_list.push_back('H'); // Orthometric height
	msr_list.push_back('I'); // Astronomic latitude
	msr_list.push_back('J'); // Astronomic longitude
	msr_list.push_back('K'); // Astronomic azimuth
	msr_list.push_back('L'); // Level difference
	msr_list.push_back('M'); // MSL arc
	msr_list.push_back('P'); // Geodetic latitude
	msr_list.push_back('Q'); // Geodetic longitude
	msr_list.push_back('R'); // Ellipsoidal height
	msr_list.push_back('S'); // Slope distance
	msr_list.push_back('V'); // Zenith distance
	msr_list.push_back('X'); // GPS Baseline cluster
	msr_list.push_back('Y'); // GPS point cluster
	msr_list.push_back('Z'); // Vertical angle
}

std::string MsrTally::GetMsrName(const char& c)
{
	return measurement_name<char, std::string>(c);
}
	

void MsrTally::initialise() {
	totalCount = A = B = C = D = E = G = H = I = J = K = L = M = P = Q = R = S = V = X = Y = Z = 0;
	ignored = 0;
	containsNonGPS = false;
}
	

MsrTally& MsrTally::operator+=(const MsrTally& rhs) {
	A += rhs.A;
	B += rhs.B;
	C += rhs.C;
	D += rhs.D;
	E += rhs.E;
	G += rhs.G;
	H += rhs.H;
	I += rhs.I;
	J += rhs.J;
	K += rhs.K;
	L += rhs.L;
	M += rhs.M;
	P += rhs.P;
	Q += rhs.Q;
	R += rhs.R;
	S += rhs.S;
	V += rhs.V;
	X += rhs.X;
	Y += rhs.Y;
	Z += rhs.Z;
	ignored += rhs.ignored;
	TotalCount();
	return *this;
}
	

//MsrTally& MsrTally::operator-=(const MsrTally& rhs) {
//	if (A >= rhs.A)
//		A -= rhs.A;
//	if (B >= rhs.B)
//		B -= rhs.B;
//	if (C >= rhs.C)
//		C -= rhs.C;
//	if (D >= rhs.D)
//		D -= rhs.D;
//	if (E >= rhs.E)
//		E -= rhs.E;
//	if (G >= rhs.G)
//		G -= rhs.G;
//	if (H >= rhs.H)
//		H -= rhs.H;
//	if (I >= rhs.I)
//		I -= rhs.I;
//	if (J >= rhs.J)
//		J -= rhs.J;
//	if (K >= rhs.K)
//		K -= rhs.K;
//	if (L >= rhs.L)
//		L -= rhs.L;
//	if (M >= rhs.M)
//		M -= rhs.M;
//	if (P >= rhs.P)
//		P -= rhs.P;
//	if (Q >= rhs.Q)
//		Q -= rhs.Q;
//	if (R >= rhs.R)
//		R -= rhs.R;
//	if (S >= rhs.S)
//		S -= rhs.S;
//	if (V >= rhs.V)
//		V -= rhs.V;
//	if (X >= rhs.X)
//		X -= rhs.X;
//	if (Y >= rhs.Y)
//		Y -= rhs.Y;
//	if (Z >= rhs.Z)
//		Z -= rhs.Z;
//	if (ignored >= rhs.ignored)
//		ignored -= rhs.ignored;
//	TotalCount();
//	return *this;
//}

MsrTally MsrTally::operator+(const MsrTally& rhs) const {
	MsrTally t = *this;
	t += rhs;
	return t;
}
	

//MsrTally MsrTally::operator-(const MsrTally& rhs) const {
//	MsrTally t = *this;
//	t -= rhs;
//	return t;
//}

//bool MsrTally::GPSOnly()
//{
//	if (G+X+Y == totalCount)
//		return true;
//	return false;
//}

UINT32 MsrTally::TotalCount() {
	containsNonGPS = (A || B || C || D || E || H || I || J || K || L || M || P || Q || R || S || V || Z);
	return totalCount=A+B+C+D+E+G+H+I+J+K+L+M+P+Q+R+S+V+X+Y+Z;
}

void MsrTally::coutSummary(std::ostream &os, const std::string& title) 
{
	// Print title
	os << title << " " << TotalCount() << " measurements:" << std::endl;
	UINT32 i, w(PRINT_VAR_PAD+NUMERIC_WIDTH);
	// Print line
	os << " ";
	for (i=0; i<w; ++i)
		os << "-";
	os << std::endl;
	// Print measurements
	std::string msr = "  " + GetMsrName('A') + ":";
	os << std::left << std::setw(PRINT_VAR_PAD) << msr << std::right << std::setw(NUMERIC_WIDTH) << A << std::endl;
	msr = "  " + GetMsrName('B') + ":";
	os << std::left << std::setw(PRINT_VAR_PAD) << msr << std::right << std::setw(NUMERIC_WIDTH) << B << std::endl;
	msr = "  " + GetMsrName('C') + ":";
	os << std::left << std::setw(PRINT_VAR_PAD) << msr << std::right << std::setw(NUMERIC_WIDTH) << C << std::endl;
	msr = "  " + GetMsrName('D') + ":";
	os << std::left << std::setw(PRINT_VAR_PAD) << msr << std::right << std::setw(NUMERIC_WIDTH) << D << std::endl;
	msr = "  " + GetMsrName('E') + ":";
	os << std::left << std::setw(PRINT_VAR_PAD) << msr << std::right << std::setw(NUMERIC_WIDTH) << E << std::endl;
	msr = "  " + GetMsrName('G') + ":";
	os << std::left << std::setw(PRINT_VAR_PAD) << msr << std::right << std::setw(NUMERIC_WIDTH) << G << std::endl;
	msr = "  " + GetMsrName('H') + ":";
	os << std::left << std::setw(PRINT_VAR_PAD) << msr << std::right << std::setw(NUMERIC_WIDTH) << H << std::endl;
	msr = "  " + GetMsrName('I') + ":";
	os << std::left << std::setw(PRINT_VAR_PAD) << msr << std::right << std::setw(NUMERIC_WIDTH) << I << std::endl;
	msr = "  " + GetMsrName('J') + ":";
	os << std::left << std::setw(PRINT_VAR_PAD) << msr << std::right << std::setw(NUMERIC_WIDTH) << J << std::endl;
	msr = "  " + GetMsrName('K') + ":";
	os << std::left << std::setw(PRINT_VAR_PAD) << msr << std::right << std::setw(NUMERIC_WIDTH) << K << std::endl;
	msr = "  " + GetMsrName('L') + ":";
	os << std::left << std::setw(PRINT_VAR_PAD) << msr << std::right << std::setw(NUMERIC_WIDTH) << L << std::endl;
	msr = "  " + GetMsrName('M') + ":";
	os << std::left << std::setw(PRINT_VAR_PAD) << msr << std::right << std::setw(NUMERIC_WIDTH) << M << std::endl;
	msr = "  " + GetMsrName('P') + ":";
	os << std::left << std::setw(PRINT_VAR_PAD) << msr << std::right << std::setw(NUMERIC_WIDTH) << P << std::endl;
	msr = "  " + GetMsrName('Q') + ":";
	os << std::left << std::setw(PRINT_VAR_PAD) << msr << std::right << std::setw(NUMERIC_WIDTH) << Q << std::endl;
	msr = "  " + GetMsrName('R') + ":";
	os << std::left << std::setw(PRINT_VAR_PAD) << msr << std::right << std::setw(NUMERIC_WIDTH) << R << std::endl;
	msr = "  " + GetMsrName('S') + ":";
	os << std::left << std::setw(PRINT_VAR_PAD) << msr << std::right << std::setw(NUMERIC_WIDTH) << S << std::endl;
	msr = "  " + GetMsrName('V') + ":";
	os << std::left << std::setw(PRINT_VAR_PAD) << msr << std::right << std::setw(NUMERIC_WIDTH) << V << std::endl;
	msr = "  " + GetMsrName('X') + ":";
	os << std::left << std::setw(PRINT_VAR_PAD) << msr << std::right << std::setw(NUMERIC_WIDTH) << X << std::endl;
	msr = "  " + GetMsrName('Y') + ":";
	os << std::left << std::setw(PRINT_VAR_PAD) << msr << std::right << std::setw(NUMERIC_WIDTH) << Y << std::endl;
	msr = "  " + GetMsrName('Z') + ":";
	os << std::left << std::setw(PRINT_VAR_PAD) << msr << std::right << std::setw(NUMERIC_WIDTH) << Z << std::endl;
	
	os << " ";
	for (i=0; i<w; ++i)
		os << "-";
	os << std::endl;
	os << std::left << std::setw(PRINT_VAR_PAD) << "  Total" << std::right << std::setw(NUMERIC_WIDTH) << TotalCount();

	if (ignored)
		os << "*" << std::endl << std::left << "  * Includes " << ignored << " ignored measurements";

	os << std::endl;
}
	

void MsrTally::coutSummaryMsrToStn(std::ostream &os, const std::string& station)
{
	// Print station
	os << std::setw(STATION) << std::left << station;

	// Print measurement count (alphabetical)
	if (A)
		os << std::setw(NUMERIC_WIDTH) << std::right << A;
	else
		os << std::setw(NUMERIC_WIDTH) << std::right << " ";
	if (B)
		os << std::setw(NUMERIC_WIDTH) << std::right << B;
	else
		os << std::setw(NUMERIC_WIDTH) << std::right << " ";
	if (C)
		os << std::setw(NUMERIC_WIDTH) << std::right << C;
	else
		os << std::setw(NUMERIC_WIDTH) << std::right << " ";
	if (D)
		os << std::setw(NUMERIC_WIDTH) << std::right << D;
	else
		os << std::setw(NUMERIC_WIDTH) << std::right << " ";
	if (E)
		os << std::setw(NUMERIC_WIDTH) << std::right << E;
	else
		os << std::setw(NUMERIC_WIDTH) << std::right << " ";
	if (G)
		os << std::setw(NUMERIC_WIDTH) << std::right << G;
	else 
		os << std::setw(NUMERIC_WIDTH) << std::right << " ";
	if (H)
		os << std::setw(NUMERIC_WIDTH) << std::right << H;
	else
		os << std::setw(NUMERIC_WIDTH) << std::right << " ";
	if (I)
		os << std::setw(NUMERIC_WIDTH) << std::right << I;
	else
		os << std::setw(NUMERIC_WIDTH) << std::right << " ";
	if (J)
		os << std::setw(NUMERIC_WIDTH) << std::right << J;
	else
		os << std::setw(NUMERIC_WIDTH) << std::right << " ";
	if (K)
		os << std::setw(NUMERIC_WIDTH) << std::right << K;
	else
		os << std::setw(NUMERIC_WIDTH) << std::right << " ";
	if (L)
		os << std::setw(NUMERIC_WIDTH) << std::right << L;
	else
		os << std::setw(NUMERIC_WIDTH) << std::right << " ";
	if (M)
		os << std::setw(NUMERIC_WIDTH) << std::right << M;
	else
		os << std::setw(NUMERIC_WIDTH) << std::right << " ";
	if (P)
		os << std::setw(NUMERIC_WIDTH) << std::right << P;
	else
		os << std::setw(NUMERIC_WIDTH) << std::right << " ";
	if (Q)
		os << std::setw(NUMERIC_WIDTH) << std::right << Q;
	else
		os << std::setw(NUMERIC_WIDTH) << std::right << " ";
	if (R)
		os << std::setw(NUMERIC_WIDTH) << std::right << R;
	else
		os << std::setw(NUMERIC_WIDTH) << std::right << " ";
	if (S)
		os << std::setw(NUMERIC_WIDTH) << std::right << S;
	else
		os << std::setw(NUMERIC_WIDTH) << std::right << " ";	
	if (V)
		os << std::setw(NUMERIC_WIDTH) << std::right << V;
	else
		os << std::setw(NUMERIC_WIDTH) << std::right << " ";
	if (X)
		os << std::setw(NUMERIC_WIDTH) << std::right << X;
	else 
		os << std::setw(NUMERIC_WIDTH) << std::right << " ";
	if (Y)
		os << std::setw(NUMERIC_WIDTH) << std::right << Y;
	else 
		os << std::setw(NUMERIC_WIDTH) << std::right << " ";
	if (Z)
		os << std::setw(NUMERIC_WIDTH) << std::right << Z;
	else
		os << std::setw(NUMERIC_WIDTH) << std::right << " ";
	
	// Total
	os << std::setw(STAT) << totalCount << std::endl;

}

void MsrTally::coutSummaryMsrToStn_Compressed(std::ostream &os, const std::string& station)
{
	// Print station
	os << std::setw(STATION) << std::left << station;

	std::stringstream msrs;

	// Print measurement count (alphabetical)
	if (A)
		msrs << "A";
	if (B)
		msrs << "B";
	if (C)
		msrs << "C";
	if (D)
		msrs << "D";
	if (E)
		msrs << "E";
	if (G)
		msrs << "G";
	if (H)
		msrs << "H";
	if (I)
		msrs << "I";
	if (J)
		msrs << "J";
	if (K)
		msrs << "K";
	if (L)
		msrs << "L";
	if (M)
		msrs << "M";
	if (P)
		msrs << "P";
	if (Q)
		msrs << "Q";
	if (R)
		msrs << "R";
	if (S)
		msrs << "S";
	if (V)
		msrs << "V";
	if (X)
		msrs << "X";
	if (Y)
		msrs << "Y";
	if (Z)
		msrs << "Z";
	
	os << std::left << std::setw(30) << msrs.str();

	// Total
	os << std::left << totalCount << std::endl;

}


UINT32 MsrTally::MeasurementCount(const char& msrType)
{
	switch (msrType)
	{
	case 'A': // Horizontal angle
		return A;
	case 'B': // Geodetic azimuth
		return B;
	case 'C': // Chord dist
		return C;
	case 'D': // Direction set
		return D;
	case 'E': // Ellipsoid arc
		return E;
	case 'G': // GPS Baseline
		return G;
	case 'H': // Orthometric height
		return H;
	case 'I': // Astronomic latitude
		return I;
	case 'J': // Astronomic longitude
		return J;
	case 'K': // Astronomic azimuth
		return K;
	case 'L': // Level difference
		return L;
	case 'M': // MSL arc
		return M;
	case 'P': // Geodetic latitude
		return P;
	case 'Q': // Geodetic longitude
		return Q;
	case 'R': // Ellipsoidal height
		return R;
	case 'S': // Slope distance
		return S;
	case 'V': // Zenith distance
		return V;
	case 'X': // GPS Baseline cluster
		return X;
	case 'Y': // GPS point cluster
		return Y;
	case 'Z': // Vertical angle
		return Z;
	}
	return 0;
}

// std::vector< boost::shared_ptr<CDnaMeasurement> >
void MsrTally::CreateTally(const vdnaMsrPtr& vMeasurements)
{
	initialise();

	for (_it_vdnamsrptr_const _it_msr=vMeasurements.begin(); _it_msr!=vMeasurements.end(); ++_it_msr)
	{
		// Increment single station measurement counters...
		switch (_it_msr->get()->GetTypeC())
		{
		case 'A': // Horizontal angle
			A++;
			break;
		case 'B': // Geodetic azimuth
			B++;
			break;
		case 'C': // Chord dist
			C++;
			break;
		case 'D': // Direction set
			D += static_cast<UINT32>(_it_msr->get()->GetDirections_ptr()->size());
			break;
		case 'E': // Ellipsoid arc
			E++;
			break;
		case 'G': // GPS Baseline (treat as single-baseline cluster)
			G += static_cast<UINT32>(_it_msr->get()->GetBaselines_ptr()->size()) * 3;
			break;
		case 'X': // GPS Baseline cluster
			X += static_cast<UINT32>(_it_msr->get()->GetBaselines_ptr()->size()) * 3;
			break;
		case 'H': // Orthometric height
			H++;
			break;
		case 'I': // Astronomic latitude
			I++;
			break;
		case 'J': // Astronomic longitude
			J++;
			break;
		case 'K': // Astronomic azimuth
			K++;
			break;
		case 'L': // Level difference
			L++;
			break;
		case 'M': // MSL arc
			M++;
			break;
		case 'P': // Geodetic latitude
			P++;
			break;
		case 'Q': // Geodetic longitude
			Q++;
			break;
		case 'R': // Ellipsoidal height
			R++;
			break;
		case 'S': // Slope distance
			S++;
			break;
		case 'V': // Zenith distance
			V++;
			break;
		case 'Y': // GPS point cluster
			Y += static_cast<UINT32>(_it_msr->get()->GetPoints_ptr()->size()) * 3;
			break;
		case 'Z': // Vertical angle
			Z++;
			break;
		}
	}
	TotalCount();
}

void MsrTally::IncrementMsrType(const char& msrType, const UINT32& count)
{
	switch (msrType)
	{
	case 'A': // Horizontal angle
		A += count;
		break;
	case 'B': // Geodetic azimuth
		B += count;
		break;
	case 'C': // Chord dist
		C += count;
		break;
	case 'D': // Direction set
		D += count;
		break;
	case 'E': // Ellipsoid arc
		E += count;
		break;
	case 'G': // GPS Baseline (treat as single-baseline cluster)
		G += count;
		break;
	case 'X': // GPS Baseline cluster
		X += count;
		break;
	case 'H': // Orthometric height
		H += count;
		break;
	case 'I': // Astronomic latitude
		I += count;
		break;
	case 'J': // Astronomic longitude
		J += count;
		break;
	case 'K': // Astronomic azimuth
		K += count;
		break;
	case 'L': // Level difference
		L += count;
		break;
	case 'M': // MSL arc
		M += count;
		break;
	case 'P': // Geodetic latitude
		P += count;
		break;
	case 'Q': // Geodetic longitude
		Q += count;
		break;
	case 'R': // Ellipsoidal height
		R += count;
		break;
	case 'S': // Slope distance
		S += count;
		break;
	case 'V': // Zenith distance
		V += count;
		break;
	case 'Y': // GPS point cluster
		Y += count;
		break;
	case 'Z': // Vertical angle
		Z += count;
		break;
	}

}
	

// std::vector<measurement_t>
void MsrTally::CreateTally(const vmsr_t& vMeasurements, const vUINT32& CML)
{
	initialise();
	for (it_vUINT32_const _it_msr=CML.begin(); _it_msr<CML.end(); _it_msr++)
	{
		// Increment single station measurement counters...
		switch (vMeasurements.at(*_it_msr).measType)
		{
		case 'A': // Horizontal angle
			A++;
			break;
		case 'B': // Geodetic azimuth
			B++; 
			break;
		case 'C': // Chord dist
			C++;
			break;
		case 'D': // Direction set
			if (vMeasurements.at(*_it_msr).measStart == xMeas)
				D += vMeasurements.at(*_it_msr).vectorCount2 - 1;
			break;
		case 'E': // Ellipsoid arc
			E++;
			break;
		case 'G': // GPS Baseline (treat as single-baseline cluster)
			G ++;
			break;
		case 'X': // GPS Baseline cluster
			if (vMeasurements.at(*_it_msr).measStart == xMeas)
				X += vMeasurements.at(*_it_msr).vectorCount1;
			break;
		case 'H': // Orthometric height
			H++;
			break;
		case 'I': // Astronomic latitude
			I++;
			break;
		case 'J': // Astronomic longitude
			J++;
			break;
		case 'K': // Astronomic azimuth
			K++;
			break;
		case 'L': // Level difference
			L++;
			break;
		case 'M': // MSL arc
			M++;
			break;
		case 'P': // Geodetic latitude
			P++;
			break;
		case 'Q': // Geodetic longitude
			Q++;
			break;
		case 'R': // Ellipsoidal height
			R++;
			break;
		case 'S': // Slope distance
			S++;
			break;
		case 'V': // Zenith distance
			V++;
			break;
		case 'Y': // GPS point cluster
			if (vMeasurements.at(*_it_msr).measStart == xMeas)
				Y += vMeasurements.at(*_it_msr).vectorCount1;
			break;
		case 'Z': // Vertical angle
			Z++;
			break;
		}
	}
	TotalCount();
}

// std::vector<measurement_t>
UINT32 MsrTally::CreateTally(const vmsr_t& vMeasurements, bool countValidOnly)
{
	initialise();
	it_vmsr_t_const _it_msr;
	for (_it_msr=vMeasurements.begin(); 
		_it_msr!=vMeasurements.end();
		++_it_msr)
	{
		// Don't include ignored measurements in the measurement count.
		if (countValidOnly)
			if (_it_msr->ignore)
				continue;

		// Don't include X and Y elements or covariance terms in the measurement count.
		if (_it_msr->measStart > xMeas)
			continue;

		// Increment single station measurement counters...
		switch (_it_msr->measType)
		{
		case 'A': // Horizontal angle
			A++;
			break;
		case 'B': // Geodetic azimuth
			B++; 
			break;
		case 'C': // Chord dist
			C++;
			break;
		case 'D': // Direction set
			if (_it_msr->measStart == xMeas)
				D += _it_msr->vectorCount2 - 1;
			break;
		case 'E': // Ellipsoid arc
			E++;
			break;
		case 'G': // GPS Baseline (treat as single-baseline cluster)
			G ++;
			break;
		case 'X': // GPS Baseline cluster
			if (_it_msr->measStart == xMeas)
				X += _it_msr->vectorCount1;
			break;
		case 'H': // Orthometric height
			H++;
			break;
		case 'I': // Astronomic latitude
			I++;
			break;
		case 'J': // Astronomic longitude
			J++;
			break;
		case 'K': // Astronomic azimuth
			K++;
			break;
		case 'L': // Level difference
			L++;
			break;
		case 'M': // MSL arc
			M++;
			break;
		case 'P': // Geodetic latitude
			P++;
			break;
		case 'Q': // Geodetic longitude
			Q++;
			break;
		case 'R': // Ellipsoidal height
			R++;
			break;
		case 'S': // Slope distance
			S++;
			break;
		case 'V': // Zenith distance
			V++;
			break;
		case 'Y': // GPS point cluster
			if (_it_msr->measStart == xMeas)
				Y += _it_msr->vectorCount1;
			break;
		case 'Z': // Vertical angle
			Z++;
			break;
		}
	}
	return TotalCount();
}

_MEASUREMENT_STATIONS_ MsrTally::Stations(const char& measType)
{
	switch(measType)
	{
		// single station measurements
	case 'H':	// Orthometric height
	case 'R':	// Ellipsoidal height
	case 'I':	// Astronomic latitude
	case 'J':	// Astronomic longitude
	case 'P':	// Geodetic latitude
	case 'Q':	// Geodetic longitude
	case 'Y':	// GPS point cluster
		return ONE_STATION;
		// dual station measurements
	case 'D':	// Direction set
	case 'B':	// Geodetic azimuth
	case 'G':	// GPS Baseline (treat as single-baseline cluster)
	case 'X':	// GPS Baseline cluster
	case 'K':	// Astronomic azimuth
	case 'C':	// Chord dist
	case 'E':	// Ellipsoid arc
	case 'M':	// MSL arc
	case 'S':	// Slope distance
	case 'L':	// Level difference
	case 'V':	// Zenith distance
	case 'Z':	// Vertical angle
		return TWO_STATION;
		// triple station measurements
	case 'A':	// Horizontal angles
		return THREE_STATION;
	default:
		return UNKNOWN_TYPE;
	}
	return UNKNOWN_TYPE;
}
	


}	// namespace measurements
}	// namespace dynadjust


