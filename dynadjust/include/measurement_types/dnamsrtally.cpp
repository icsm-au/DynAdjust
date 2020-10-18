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
	msr_list.push_back('V'); // Zenith angle
	msr_list.push_back('X'); // GPS Baseline cluster
	msr_list.push_back('Y'); // GPS point cluster
	msr_list.push_back('Z'); // Vertical angle
}

string MsrTally::GetMsrName(const char& c)
{
	return measurement_name<char, string>(c);
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
	

MsrTally& MsrTally::operator-=(const MsrTally& rhs) {
	if (A >= rhs.A)
		A -= rhs.A;
	if (B >= rhs.B)
		B -= rhs.B;
	if (C >= rhs.C)
		C -= rhs.C;
	if (D >= rhs.D)
		D -= rhs.D;
	if (E >= rhs.E)
		E -= rhs.E;
	if (G >= rhs.G)
		G -= rhs.G;
	if (H >= rhs.H)
		H -= rhs.H;
	if (I >= rhs.I)
		I -= rhs.I;
	if (J >= rhs.J)
		J -= rhs.J;
	if (K >= rhs.K)
		K -= rhs.K;
	if (L >= rhs.L)
		L -= rhs.L;
	if (M >= rhs.M)
		M -= rhs.M;
	if (P >= rhs.P)
		P -= rhs.P;
	if (Q >= rhs.Q)
		Q -= rhs.Q;
	if (R >= rhs.R)
		R -= rhs.R;
	if (S >= rhs.S)
		S -= rhs.S;
	if (V >= rhs.V)
		V -= rhs.V;
	if (X >= rhs.X)
		X -= rhs.X;
	if (Y >= rhs.Y)
		Y -= rhs.Y;
	if (Z >= rhs.Z)
		Z -= rhs.Z;
	if (ignored >= rhs.ignored)
		ignored -= rhs.ignored;
	TotalCount();
	return *this;
}

MsrTally MsrTally::operator+(const MsrTally& rhs) const {
	MsrTally t = *this;
	t += rhs;
	return t;
}
	

MsrTally MsrTally::operator-(const MsrTally& rhs) const {
	MsrTally t = *this;
	t -= rhs;
	return t;
}

bool MsrTally::GPSOnly()
{
	if (G+X+Y == totalCount)
		return true;
	return false;
}

UINT32 MsrTally::TotalCount() {
	containsNonGPS = (A || B || C || D || E || H || I || J || K || L || M || P || Q || R || S || V || Z);
	return totalCount=A+B+C+D+E+G+H+I+J+K+L+M+P+Q+R+S+V+X+Y+Z;
}

void MsrTally::coutSummary(ostream &os, const string& title) 
{
	// Print title
	os << title << " " << TotalCount() << " measurements:" << endl;
	UINT32 i, w(PRINT_VAR_PAD+NUMERIC_WIDTH);
	// Print line
	os << " ";
	for (i=0; i<w; ++i)
		os << "-";
	os << endl;
	// Print measurements
	string msr = "  " + GetMsrName('A') + ":";
	os << left << setw(PRINT_VAR_PAD) << msr << right << setw(NUMERIC_WIDTH) << A << endl;
	msr = "  " + GetMsrName('B') + ":";
	os << left << setw(PRINT_VAR_PAD) << msr << right << setw(NUMERIC_WIDTH) << B << endl;
	msr = "  " + GetMsrName('C') + ":";
	os << left << setw(PRINT_VAR_PAD) << msr << right << setw(NUMERIC_WIDTH) << C << endl;
	msr = "  " + GetMsrName('D') + ":";
	os << left << setw(PRINT_VAR_PAD) << msr << right << setw(NUMERIC_WIDTH) << D << endl;
	msr = "  " + GetMsrName('E') + ":";
	os << left << setw(PRINT_VAR_PAD) << msr << right << setw(NUMERIC_WIDTH) << E << endl;
	msr = "  " + GetMsrName('G') + ":";
	os << left << setw(PRINT_VAR_PAD) << msr << right << setw(NUMERIC_WIDTH) << G << endl;
	msr = "  " + GetMsrName('H') + ":";
	os << left << setw(PRINT_VAR_PAD) << msr << right << setw(NUMERIC_WIDTH) << H << endl;
	msr = "  " + GetMsrName('I') + ":";
	os << left << setw(PRINT_VAR_PAD) << msr << right << setw(NUMERIC_WIDTH) << I << endl;
	msr = "  " + GetMsrName('J') + ":";
	os << left << setw(PRINT_VAR_PAD) << msr << right << setw(NUMERIC_WIDTH) << J << endl;
	msr = "  " + GetMsrName('K') + ":";
	os << left << setw(PRINT_VAR_PAD) << msr << right << setw(NUMERIC_WIDTH) << K << endl;
	msr = "  " + GetMsrName('L') + ":";
	os << left << setw(PRINT_VAR_PAD) << msr << right << setw(NUMERIC_WIDTH) << L << endl;
	msr = "  " + GetMsrName('M') + ":";
	os << left << setw(PRINT_VAR_PAD) << msr << right << setw(NUMERIC_WIDTH) << M << endl;
	msr = "  " + GetMsrName('P') + ":";
	os << left << setw(PRINT_VAR_PAD) << msr << right << setw(NUMERIC_WIDTH) << P << endl;
	msr = "  " + GetMsrName('Q') + ":";
	os << left << setw(PRINT_VAR_PAD) << msr << right << setw(NUMERIC_WIDTH) << Q << endl;
	msr = "  " + GetMsrName('R') + ":";
	os << left << setw(PRINT_VAR_PAD) << msr << right << setw(NUMERIC_WIDTH) << R << endl;
	msr = "  " + GetMsrName('S') + ":";
	os << left << setw(PRINT_VAR_PAD) << msr << right << setw(NUMERIC_WIDTH) << S << endl;
	msr = "  " + GetMsrName('V') + ":";
	os << left << setw(PRINT_VAR_PAD) << msr << right << setw(NUMERIC_WIDTH) << V << endl;
	msr = "  " + GetMsrName('X') + ":";
	os << left << setw(PRINT_VAR_PAD) << msr << right << setw(NUMERIC_WIDTH) << X << endl;
	msr = "  " + GetMsrName('Y') + ":";
	os << left << setw(PRINT_VAR_PAD) << msr << right << setw(NUMERIC_WIDTH) << Y << endl;
	msr = "  " + GetMsrName('Z') + ":";
	os << left << setw(PRINT_VAR_PAD) << msr << right << setw(NUMERIC_WIDTH) << Z << endl;
	
	os << " ";
	for (i=0; i<w; ++i)
		os << "-";
	os << endl;
	os << left << setw(PRINT_VAR_PAD) << "  Total" << right << setw(NUMERIC_WIDTH) << TotalCount();

	if (ignored)
		os << "*" << endl << left << "  * Includes " << ignored << " ignored measurements";

	os << endl;
}
	

void MsrTally::coutSummaryMsrToStn(ostream &os, const string& station)
{
	// Print station
	os << setw(STATION) << left << station;

	// Print measurement count (alphabetical)
	if (A)
		os << setw(NUMERIC_WIDTH) << right << A;
	else
		os << setw(NUMERIC_WIDTH) << right << " ";
	if (B)
		os << setw(NUMERIC_WIDTH) << right << B;
	else
		os << setw(NUMERIC_WIDTH) << right << " ";
	if (C)
		os << setw(NUMERIC_WIDTH) << right << C;
	else
		os << setw(NUMERIC_WIDTH) << right << " ";
	if (D)
		os << setw(NUMERIC_WIDTH) << right << D;
	else
		os << setw(NUMERIC_WIDTH) << right << " ";
	if (E)
		os << setw(NUMERIC_WIDTH) << right << E;
	else
		os << setw(NUMERIC_WIDTH) << right << " ";
	if (G)
		os << setw(NUMERIC_WIDTH) << right << G;
	else 
		os << setw(NUMERIC_WIDTH) << right << " ";
	if (H)
		os << setw(NUMERIC_WIDTH) << right << H;
	else
		os << setw(NUMERIC_WIDTH) << right << " ";
	if (I)
		os << setw(NUMERIC_WIDTH) << right << I;
	else
		os << setw(NUMERIC_WIDTH) << right << " ";
	if (J)
		os << setw(NUMERIC_WIDTH) << right << J;
	else
		os << setw(NUMERIC_WIDTH) << right << " ";
	if (K)
		os << setw(NUMERIC_WIDTH) << right << K;
	else
		os << setw(NUMERIC_WIDTH) << right << " ";
	if (L)
		os << setw(NUMERIC_WIDTH) << right << L;
	else
		os << setw(NUMERIC_WIDTH) << right << " ";
	if (M)
		os << setw(NUMERIC_WIDTH) << right << M;
	else
		os << setw(NUMERIC_WIDTH) << right << " ";
	if (P)
		os << setw(NUMERIC_WIDTH) << right << P;
	else
		os << setw(NUMERIC_WIDTH) << right << " ";
	if (Q)
		os << setw(NUMERIC_WIDTH) << right << Q;
	else
		os << setw(NUMERIC_WIDTH) << right << " ";
	if (R)
		os << setw(NUMERIC_WIDTH) << right << R;
	else
		os << setw(NUMERIC_WIDTH) << right << " ";
	if (S)
		os << setw(NUMERIC_WIDTH) << right << S;
	else
		os << setw(NUMERIC_WIDTH) << right << " ";	
	if (V)
		os << setw(NUMERIC_WIDTH) << right << V;
	else
		os << setw(NUMERIC_WIDTH) << right << " ";
	if (X)
		os << setw(NUMERIC_WIDTH) << right << X;
	else 
		os << setw(NUMERIC_WIDTH) << right << " ";
	if (Y)
		os << setw(NUMERIC_WIDTH) << right << Y;
	else 
		os << setw(NUMERIC_WIDTH) << right << " ";
	if (Z)
		os << setw(NUMERIC_WIDTH) << right << Z;
	else
		os << setw(NUMERIC_WIDTH) << right << " ";
	
	// Total
	os << setw(STAT) << totalCount << endl;

}

void MsrTally::coutSummaryMsrToStn_Compressed(ostream &os, const string& station)
{
	// Print station
	os << setw(STATION) << left << station;

	stringstream msrs;

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
	
	os << left << setw(30) << msrs.str();

	// Total
	os << left << totalCount << endl;

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
	case 'V': // Zenith angle
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

// vector< boost::shared_ptr<CDnaMeasurement> >
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
		case 'V': // Zenith angle
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
	case 'V': // Zenith angle
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
	

// vector<measurement_t>
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
				D += vMeasurements.at(*_it_msr).vectorCount1 - 1;
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
		case 'V': // Zenith angle
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
	case 'V':	// Zenith angle
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


