//============================================================================
// Name         : dnadirection.cpp
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
// Description  : CDnaDirection implementation file
//============================================================================

#include <include/exception/dnaexception.hpp>
#include <include/measurement_types/dnadirection.hpp>

using namespace dynadjust::exception;

namespace dynadjust {
namespace measurements {

CDnaDirection::CDnaDirection(void)
	: m_strTarget("")
	, m_drValue(0.)
	, m_dStdDev(0.)
	, m_fInstHeight(0.)
	, m_fTargHeight(0.)
	, m_lRecordedTotal(0)
	, m_lsetID(0)
{
	m_strType = "D";
	m_MSmeasurementStations = TWO_STATION;
}

CDnaDirection::~CDnaDirection(void)
{
}

// copy constructors
CDnaDirection::CDnaDirection(const CDnaDirection& newDirection)
{
	m_strFirst = newDirection.m_strFirst;
	m_bIgnore = newDirection.m_bIgnore;
	m_strTarget = newDirection.m_strTarget;
	m_drValue = newDirection.m_drValue;
	m_dStdDev = newDirection.m_dStdDev;
	m_fInstHeight = newDirection.m_fInstHeight;
	m_fTargHeight = newDirection.m_fTargHeight;
	m_lRecordedTotal = newDirection.m_lRecordedTotal;
	m_lsetID = newDirection.m_lsetID;
	m_MSmeasurementStations = newDirection.m_MSmeasurementStations;

	m_strType = "D";

	m_databaseIdSet = newDirection.m_databaseIdSet;
	m_msr_db_map = newDirection.m_msr_db_map;
}


CDnaDirection::CDnaDirection(const bool bIgnore, const string& strFirst, const string& strTarget, const double& drValue, const double& dStdDev)
{
	m_strType = "D";
	m_strFirst = strFirst;
	m_bIgnore = bIgnore;
	m_strTarget = strTarget;
	m_drValue = drValue;
	m_dStdDev = dStdDev;
	m_lRecordedTotal = 0;
	m_lsetID = 0;
}

CDnaDirection& CDnaDirection::operator= (const CDnaDirection& rhs)
{
	// check for assignment to self!
	if (this == &rhs)
		return *this;

	CDnaMeasurement::operator=(rhs);
	m_strTarget = rhs.m_strTarget;
	m_drValue = rhs.m_drValue;
	m_dStdDev = rhs.m_dStdDev;
	m_fInstHeight = rhs.m_fInstHeight;
	m_fTargHeight = rhs.m_fTargHeight;
	m_lRecordedTotal = rhs.m_lRecordedTotal;
	m_lsetID = rhs.m_lsetID;
	m_MSmeasurementStations = rhs.m_MSmeasurementStations;

	m_databaseIdSet = rhs.m_databaseIdSet;
	m_msr_db_map = rhs.m_msr_db_map;

	return *this;
}


bool CDnaDirection::operator== (const CDnaDirection& rhs) const
{
	return (
		m_strFirst == rhs.m_strFirst &&
		m_bIgnore == rhs.m_bIgnore &&
		m_strTarget == rhs.m_strTarget &&
		m_drValue == rhs.m_drValue &&
		m_dStdDev == rhs.m_dStdDev &&
		m_strType == rhs.m_strType &&
		m_fInstHeight == rhs.m_fInstHeight &&
		m_fTargHeight == rhs.m_fTargHeight
		);
}


bool CDnaDirection::operator< (const CDnaDirection& rhs) const
{
	if (m_strFirst == rhs.m_strFirst) {
		if (m_strType == rhs.m_strType) {	// could be B, D, V, Z
			if (m_bIgnore == rhs.m_bIgnore) {
				if (m_strTarget == rhs.m_strTarget) {
					if (m_drValue == rhs.m_drValue) {
						if (m_dStdDev == rhs.m_dStdDev) {
							if (m_fInstHeight == rhs.m_fInstHeight)
								return m_fTargHeight < rhs.m_fTargHeight;
							else
								return m_fInstHeight < rhs.m_fInstHeight; }
						else
							return m_dStdDev < rhs.m_dStdDev; }
					else
						return m_drValue < rhs.m_drValue; }
				else
					return m_strTarget < rhs.m_strTarget; }
			else
				return m_bIgnore < rhs.m_bIgnore; }
		else
			return m_strType < rhs.m_strType; }
	else
		return m_strFirst < rhs.m_strFirst;
}

void CDnaDirection::coutMeasurementData(ostream &os, const UINT16& uType) const
{
	coutMeasurement(os);
	os << setw(INST_WIDTH) << m_strFirst;
	os << setw(TARG_WIDTH) << m_strTarget;
	os << setw(3) << (m_bIgnore ? "*" : " ") << setw(MEAS_WIDTH) << setprecision(9) << fixed << m_drValue;
	os << setw(VAR_WIDTH) << setprecision(3) << fixed << m_dStdDev;
	switch (GetTypeC())
	{
	case 'Z':
	case 'V':
		os << setw(7) << setprecision(3) << fixed << m_fInstHeight << 
			setw(7) << setprecision(3) << fixed << m_fTargHeight;
		break;
	default:
		os << setw(7) << " " << setw(7) << " ";
		break;
	}
	
	os << endl;
}
	

void CDnaDirection::WriteDynaMLMsr(std::ofstream* dynaml_stream, bool bSubMeasurement /*= false*/) const
{
	if (bSubMeasurement)
	{
		// Direction set
		*dynaml_stream << "    <Directions>" << endl;
		if (m_bIgnore)
			*dynaml_stream << "      <Ignore>*</Ignore>" << endl;
		else
			*dynaml_stream << "      <Ignore/>" << endl;
		*dynaml_stream << "      <Target>" << m_strTarget << "</Target>" << endl;
		*dynaml_stream << "      <Value>" << setprecision(8) << fixed 
			<< RadtoDms(m_drValue) << "</Value>" << endl;
		*dynaml_stream << "      <StdDev>" << scientific << setprecision(6) << Seconds(m_dStdDev) << "</StdDev>" << endl;	
		if (m_databaseIdSet)
			*dynaml_stream << "      <MeasurementID>" << m_msr_db_map.msr_id << "</MeasurementID>" << endl;
		*dynaml_stream << "    </Directions>" << endl;
	}
	else
	{
		// Zenith angle, vertical angle
		*dynaml_stream << "  <!--Type " << measurement_name<char, string>(GetTypeC()) << "-->" << endl;
		*dynaml_stream << "  <DnaMeasurement>" << endl;
		*dynaml_stream << "    <Type>" << m_strType << "</Type>" << endl;
		// Source file from which the measurement came
		*dynaml_stream << "    <Source>" << m_sourceFile << "</Source>" << endl;
		if (m_bIgnore)
			*dynaml_stream << "    <Ignore>*</Ignore>" << endl;
		else
			*dynaml_stream << "    <Ignore/>" << endl;
		*dynaml_stream << "    <First>" << m_strFirst << "</First>" << endl;
		*dynaml_stream << "    <Second>" << m_strTarget << "</Second>" << endl;
		*dynaml_stream << "    <Value>" << setprecision(8) << fixed 
			<< RadtoDms(m_drValue) << "</Value>" << endl;
		*dynaml_stream << "    <StdDev>" << scientific << setprecision(6) << Seconds(m_dStdDev) << "</StdDev>" << endl;	

		// Zenith angle, vertical angle
		switch (GetTypeC())
		{
		case 'Z':
		case 'V':
			*dynaml_stream << "    <InstHeight>" << fixed << setprecision(3) << m_fInstHeight << "</InstHeight>" << endl;
			*dynaml_stream << "    <TargHeight>" << fixed << setprecision(3) << m_fTargHeight << "</TargHeight>" << endl;
			break;
		}
		
		if (m_databaseIdSet)
			*dynaml_stream << "    <MeasurementID>" << m_msr_db_map.msr_id << "</MeasurementID>" << endl;
		*dynaml_stream << "  </DnaMeasurement>" << endl;
	}
}
	
void CDnaDirection::WriteDNAMsr(std::ofstream* dynaml_stream, const dna_msr_fields& dmw, const dna_msr_fields& dml, bool bSubMeasurement /*= false*/) const
{
	*dynaml_stream << setw(dmw.msr_type) << m_strType;
	if (m_bIgnore)
		*dynaml_stream << setw(dmw.msr_ignore) << "*";
	else
		*dynaml_stream << setw(dmw.msr_ignore) << " ";

	// database id width
	UINT32 width(dml.msr_id_msr - dml.msr_inst_ht);

	if (bSubMeasurement)
	{
		// Direction set
		*dynaml_stream << left << setw(dmw.msr_inst) << " ";
		*dynaml_stream << setw(dmw.msr_targ1) << " ";
		*dynaml_stream << left << setw(dmw.msr_targ2) << m_strTarget;
		*dynaml_stream << setw(dmw.msr_linear) << " ";	// linear measurement value
		*dynaml_stream << setw(dmw.msr_ang_d + dmw.msr_ang_m + dmw.msr_ang_s) << 
			right << FormatDnaDmsString(RadtoDms(m_drValue), 8);
		*dynaml_stream << setw(dmw.msr_stddev) << fixed << setprecision(3) << Seconds(m_dStdDev);
	}
	else
	{
		// Azimuth, zenith angle, vertical angle
		*dynaml_stream << left << setw(dmw.msr_inst) << m_strFirst;
		*dynaml_stream << left << setw(dmw.msr_targ1) << m_strTarget;
		*dynaml_stream << setw(dmw.msr_targ2) << " ";
		*dynaml_stream << setw(dmw.msr_linear) << " ";	// linear measurement value
		*dynaml_stream << setw(dmw.msr_ang_d + dmw.msr_ang_m + dmw.msr_ang_s) << 
			right << FormatDnaDmsString(RadtoDms(m_drValue), 8);
		*dynaml_stream << setw(dmw.msr_stddev) << fixed << setprecision(3) << Seconds(m_dStdDev);

		// Zenith angle, vertical angle
		switch (GetTypeC())
		{
		case 'Z':
		case 'V':
			*dynaml_stream << setw(dmw.msr_inst_ht) << fixed << setprecision(3) << m_fInstHeight;
			*dynaml_stream << setw(dmw.msr_targ_ht) << fixed << setprecision(3) << m_fTargHeight;
			width = dml.msr_id_msr - dml.msr_targ_ht - dmw.msr_targ_ht;
			break;
		}
	}

	if (m_databaseIdSet)
	{
		*dynaml_stream << setw(width) << " ";
		*dynaml_stream << setw(dmw.msr_id_msr) << m_msr_db_map.msr_id;

		if (bSubMeasurement)
			*dynaml_stream << setw(dmw.msr_id_cluster) << m_msr_db_map.cluster_id;
	}

	*dynaml_stream << endl;
}
	
void CDnaDirection::SimulateMsr(vdnaStnPtr* vStations, const CDnaEllipsoid* ellipsoid)
{
	_it_vdnastnptr stn1_it(vStations->begin() + m_lstn1Index);
	_it_vdnastnptr stn2_it(vStations->begin() + m_lstn2Index);
	
	double zenithDistance, direction;

	// compute direction for geodetic azimuths, directions, astronomic azimuths, or if 
	// deflections have been supplied
	if (GetTypeC() == 'B' ||
		GetTypeC() == 'D' ||
		GetTypeC() == 'K' ||
		fabs(stn1_it->get()->GetverticalDef()) > E4_SEC_DEFLECTION || 
		fabs(stn1_it->get()->GetmeridianDef()) > E4_SEC_DEFLECTION)
	{
		direction = Direction(
			stn1_it->get()->GetXAxis(), 
			stn1_it->get()->GetYAxis(),
			stn1_it->get()->GetZAxis(),
			stn2_it->get()->GetXAxis(), 
			stn2_it->get()->GetYAxis(),
			stn2_it->get()->GetZAxis(),
			stn1_it->get()->GetcurrentLatitude(),
			stn1_it->get()->GetcurrentLongitude());
	}

	switch (GetTypeC())
	{
	case 'D':	
		// assign direction
		m_drValue = direction;

		// Deflections available?
		if (fabs(stn1_it->get()->GetverticalDef()) > E4_SEC_DEFLECTION || fabs(stn1_it->get()->GetmeridianDef()) > E4_SEC_DEFLECTION)
		{
			// 1. compute zenith angle 
			zenithDistance = ZenithDistance<double>(
				stn1_it->get()->GetXAxis(), 
				stn1_it->get()->GetYAxis(),
				stn1_it->get()->GetZAxis(),
				stn2_it->get()->GetXAxis(), 
				stn2_it->get()->GetYAxis(),
				stn2_it->get()->GetZAxis(),
				stn1_it->get()->GetcurrentLatitude(),
				stn1_it->get()->GetcurrentLongitude(),
				stn2_it->get()->GetcurrentLatitude(),
				stn2_it->get()->GetcurrentLongitude(),
				0.0,				
				0.0);	

			// 2. Compute deflection
			m_preAdjCorr = DirectionDeflectionCorrection<double>(
				m_drValue,
				zenithDistance,
				stn1_it->get()->GetverticalDef(),			// deflection in prime vertical
				stn1_it->get()->GetmeridianDef());			// deflection in prime meridian
			
			// 3. apply deflection correction
			m_drValue += m_preAdjCorr;
		}
		break;
	case 'B':	
		// assign direction
		m_drValue = direction;
		break;
	case 'K':	
		// assign direction
		m_drValue = direction;

		// Deflections available?
		if (fabs(stn1_it->get()->GetverticalDef()) > E4_SEC_DEFLECTION || fabs(stn1_it->get()->GetmeridianDef()) > E4_SEC_DEFLECTION)
		{
			// 1. compute zenith angle 
			zenithDistance = ZenithDistance<double>(
				stn1_it->get()->GetXAxis(), 
				stn1_it->get()->GetYAxis(),
				stn1_it->get()->GetZAxis(),
				stn2_it->get()->GetXAxis(), 
				stn2_it->get()->GetYAxis(),
				stn2_it->get()->GetZAxis(),
				stn1_it->get()->GetcurrentLatitude(),
				stn1_it->get()->GetcurrentLongitude(),
				stn2_it->get()->GetcurrentLatitude(),
				stn2_it->get()->GetcurrentLongitude(),
				0.0,				
				0.0);	

			// 2. Compute correction
			m_preAdjCorr = LaplaceCorrection<double>(		// Laplace correction
				m_drValue,									// geodetic azimuth
				zenithDistance,								// zenith distance
				stn1_it->get()->GetverticalDef(),			// deflection in prime vertical
				stn1_it->get()->GetmeridianDef(),			// deflection in prime meridian
				stn1_it->get()->GetcurrentLatitude());

			// 3. apply deflection correction
			m_drValue += m_preAdjCorr;
		}

		break;
	
	case 'Z':
		m_fInstHeight = float(1.650);
		m_fTargHeight = float(1.651);

		// compute vertical angle 
		m_drValue = VerticalAngle<double>(
			stn1_it->get()->GetXAxis(), 
			stn1_it->get()->GetYAxis(),
			stn1_it->get()->GetZAxis(),
			stn2_it->get()->GetXAxis(), 
			stn2_it->get()->GetYAxis(),
			stn2_it->get()->GetZAxis(),
			stn1_it->get()->GetcurrentLatitude(),
			stn1_it->get()->GetcurrentLongitude(),
			stn2_it->get()->GetcurrentLatitude(),
			stn2_it->get()->GetcurrentLongitude(),
			m_fInstHeight,				
			m_fTargHeight);

		// Deflections available?
		if (fabs(stn1_it->get()->GetverticalDef()) > E4_SEC_DEFLECTION || fabs(stn1_it->get()->GetmeridianDef()) > E4_SEC_DEFLECTION)
		{
			// 1. compute bearing from estimated coordinates
			// see above
			
			// 2. Compute correction
			m_preAdjCorr = ZenithDeflectionCorrection<double>(	// Correction to vertical angle for deflection of vertical
				direction,										// geodetic azimuth
				stn1_it->get()->GetverticalDef(),				// deflection in prime vertical
				stn1_it->get()->GetmeridianDef());				// deflection in prime meridian

			// 3. apply deflection correction
			m_drValue += m_preAdjCorr;
		}

		break;
	case 'V':
		m_fInstHeight = float(1.650);
		m_fTargHeight = float(1.651);

		// compute zenith angle 
		m_drValue = ZenithDistance<double>(
			stn1_it->get()->GetXAxis(), 
			stn1_it->get()->GetYAxis(),
			stn1_it->get()->GetZAxis(),
			stn2_it->get()->GetXAxis(), 
			stn2_it->get()->GetYAxis(),
			stn2_it->get()->GetZAxis(),
			stn1_it->get()->GetcurrentLatitude(),
			stn1_it->get()->GetcurrentLongitude(),
			stn2_it->get()->GetcurrentLatitude(),
			stn2_it->get()->GetcurrentLongitude(),
			m_fInstHeight,
			m_fTargHeight);

		// Deflections available?
		if (fabs(stn1_it->get()->GetverticalDef()) > E4_SEC_DEFLECTION || fabs(stn1_it->get()->GetmeridianDef()) > E4_SEC_DEFLECTION)
		{
			// 1. compute bearing from estimated coordinates
			// see above

			// 2. Compute correction
			m_preAdjCorr = ZenithDeflectionCorrection<double>(	// Correction to vertical angle for deflection of vertical
				direction,										// geodetic azimuth
				stn1_it->get()->GetverticalDef(),				// deflection in prime vertical
				stn1_it->get()->GetmeridianDef());				// deflection in prime meridian

			// 3. apply deflection correction
			m_drValue += m_preAdjCorr;
		}
	}

	m_dStdDev = SecondstoRadians(0.010);
}
	

UINT32 CDnaDirection::SetMeasurementRec(std::ifstream* ifs_stns, std::ifstream* ifs_msrs, measurement_t* measRecord)
{
	char stationName[STN_NAME_WIDTH];

	if (GetTypeC() == 'D')		// part of a DnaDirectionSet
	{
		// get data relating to each direction
		if (ifs_msrs->eof() || !ifs_msrs->good())
			throw XMLInteropException("SetMeasurementRec(): Errors were encountered when reading from the binary measurement file.", 0);
		ifs_msrs->read(reinterpret_cast<char *>(measRecord), sizeof(measurement_t));
	}

	m_bIgnore = measRecord->ignore;
	m_lsetID = measRecord->clusterID;
	m_MSmeasurementStations = (MEASUREMENT_STATIONS)measRecord->measurementStations;
	// measRecord holds the full number of measurement blocks, which is 
	// the numberofdirections in the vector plus one for the RO
	m_lRecordedTotal = measRecord->vectorCount1;

	if (GetTypeC() != 'D')		// not part of a DnaDirectionSet (e.g. vertical angle, zenith angle)
	{
		m_strType = measRecord->measType;
		
		// first station
		m_lstn1Index = measRecord->station1;
		ifs_stns->seekg(sizeof(UINT32) + measRecord->station1 * sizeof(station_t), ios::beg);
		ifs_stns->read(reinterpret_cast<char *>(&stationName), sizeof(stationName));
		m_strFirst = stationName;

		m_fInstHeight = static_cast<float> (measRecord->term3);
		m_fTargHeight = static_cast<float> (measRecord->term4);
	}

	// target station
	m_lstn2Index = measRecord->station2;
	ifs_stns->seekg(sizeof(UINT32) + measRecord->station2 * sizeof(station_t), ios::beg);
	ifs_stns->read(reinterpret_cast<char *>(&stationName), sizeof(stationName));
	m_strTarget = stationName;
	
	m_measAdj = measRecord->measAdj;
	m_measCorr = measRecord->measCorr;
	m_measAdjPrec = measRecord->measAdjPrec;
	m_residualPrec = measRecord->residualPrec;
	m_preAdjCorr = measRecord->preAdjCorr;
	m_drValue = measRecord->term1;
	m_dStdDev = sqrt(measRecord->term2);
	
	return 1;
}
	

UINT32 CDnaDirection::SetMeasurementRec(const vstn_t& binaryStn, it_vmsr_t& it_msr)
{
	m_bIgnore = it_msr->ignore;
	m_lsetID = it_msr->clusterID;
	m_MSmeasurementStations = (MEASUREMENT_STATIONS)it_msr->measurementStations;
	// it_msr holds the full number of measurement blocks, which is 
	// the numberofdirections in the vector plus one for the RO
	m_lRecordedTotal = it_msr->vectorCount1;

	if (it_msr->measType != 'D')		// not part of a DnaDirectionSet (e.g. vertical angle, zenith angle)
	{
		m_strType = it_msr->measType;
		
		// first station
		m_lstn1Index = it_msr->station1;
		m_strFirst = binaryStn.at(it_msr->station1).stationName;
		// inst and targ heights
		m_fInstHeight = static_cast<float> (it_msr->term3);
		m_fTargHeight = static_cast<float> (it_msr->term4);
	}

	// target station
	m_lstn2Index = it_msr->station2;
	m_strTarget = binaryStn.at(it_msr->station2).stationName;
	
	m_measAdj = it_msr->measAdj;
	m_measCorr = it_msr->measCorr;
	m_measAdjPrec = it_msr->measAdjPrec;
	m_residualPrec = it_msr->residualPrec;
	m_preAdjCorr = it_msr->preAdjCorr;
	m_drValue = it_msr->term1;
	m_dStdDev = sqrt(it_msr->term2);

	return 0;
}
	

void CDnaDirection::WriteBinaryMsr(std::ofstream* binary_stream, PUINT32 msrIndex) const
{
	measurement_t measRecord;
	measRecord.measType = GetTypeC();
	if (m_strType == "D")
		measRecord.measStart = yMeas;
	else
		measRecord.measStart = xMeas;
	measRecord.ignore = m_bIgnore;
	measRecord.station1 = m_lstn1Index;
	measRecord.station2 = m_lstn2Index;
	measRecord.measAdj = m_measAdj;
	measRecord.measCorr = m_measCorr;
	measRecord.measAdjPrec = m_measAdjPrec;
	measRecord.residualPrec = m_residualPrec;
	measRecord.preAdjCorr = m_preAdjCorr;
	measRecord.term1 = m_drValue;
	measRecord.term2 = m_dStdDev * m_dStdDev;	// convert to variance
	measRecord.term3 = m_fInstHeight;
	measRecord.term4 = m_fTargHeight;

	measRecord.clusterID = m_lsetID;
	// number of Directions in the parent cluster including the first
	// measRecord.vectorCount1 = m_lRecordedTotal + 1;
	measRecord.vectorCount1 = m_lRecordedTotal;
	measRecord.measurementStations = m_MSmeasurementStations;
	measRecord.fileOrder = ((*msrIndex)++);

	binary_stream->write(reinterpret_cast<char *>(&measRecord), sizeof(measurement_t));
}


void CDnaDirection::coutDirectionData(ostream &os) const
{
	os << setw(16) << " " << setw(MSR2_WIDTH) << m_strTarget;
	os << setw(3) << (m_bIgnore ? "*" : " ") << setw(MEAS_WIDTH) << m_drValue;
	os << setw(VAR_WIDTH) << m_dStdDev;
	os << setw(7) << " ";	// m_fInstHeight
	switch (GetTypeC())
	{
	case 'Z':
	case 'V':
		os << setw(7) << setprecision(3) << fixed << m_fTargHeight;
		break;
	default:
		os << setw(7) << " ";
		break;
	}
	os << endl;
}

void CDnaDirection::SetValue(const string& str)
{
	RadFromDmsString(&m_drValue, trimstr(str));
}

void CDnaDirection::SetStdDev(const string& str)
{
	// convert seconds to radians
	RadFromSecondsString(&m_dStdDev, trimstr(str));
}

// Instrument and target heights only make sense for 
// slope distances, vertical angles and zenith distances.
// Note: vertical angles and zenith distances are derived
// from CDnaDirection, so these methods are provided here,
// but don't need instrument height for 'D' measurements.
void CDnaDirection::SetInstrumentHeight(const string& str)
{
	FloatFromString<float>(m_fInstHeight, trimstr(str));
}

void CDnaDirection::SetTargetHeight(const string& str)
{
	FloatFromString<float>(m_fTargHeight, trimstr(str));
}












CDnaAzimuth::CDnaAzimuth(void)
{
	m_strType = "";		// could be B or K
	m_strTarget = "";
	m_drValue = 0.;
	m_dStdDev = 0.;
	m_fInstHeight = 0.;
	m_fTargHeight = 0.;
	m_lRecordedTotal = 0;
	m_lsetID = 0;
	m_MSmeasurementStations = TWO_STATION;

}

CDnaAzimuth::~CDnaAzimuth(void)
{
}

// copy constructors
CDnaAzimuth::CDnaAzimuth(const CDnaAzimuth& newAzimuth)
{
	m_strType = newAzimuth.m_strType;
	m_strFirst = newAzimuth.m_strFirst;
	m_bIgnore = newAzimuth.m_bIgnore;
	m_strTarget = newAzimuth.m_strTarget;
	m_drValue = newAzimuth.m_drValue;
	m_dStdDev = newAzimuth.m_dStdDev;
	m_MSmeasurementStations = newAzimuth.m_MSmeasurementStations;

}


CDnaAzimuth::CDnaAzimuth(const bool bIgnore, const string& strFirst, const string& strTarget, const double& drValue, const double& dStdDev, bool bConvertAstroToGeodetic, bool bComputeDeflectionsFromAstro)
{
	m_bIgnore = bIgnore;
	m_strTarget = strTarget;
	m_drValue = drValue;
	m_dStdDev = dStdDev;
}


CDnaAzimuth& CDnaAzimuth::operator= (const CDnaAzimuth& rhs)
{
	// check for assignment to self!
	if (this == &rhs)
		return *this;

	CDnaMeasurement::operator=(rhs);
	m_strTarget = rhs.m_strTarget;
	m_drValue = rhs.m_drValue;
	m_dStdDev = rhs.m_dStdDev;
	m_lRecordedTotal = rhs.m_lRecordedTotal;
	m_lsetID = rhs.m_lsetID;
	m_MSmeasurementStations = rhs.m_MSmeasurementStations;

	return *this;
}


bool CDnaAzimuth::operator== (const CDnaAzimuth& rhs) const
{
	return (
		m_strFirst == rhs.m_strFirst &&
		m_bIgnore == rhs.m_bIgnore &&
		m_strTarget == rhs.m_strTarget &&
		m_drValue == rhs.m_drValue &&
		m_dStdDev == rhs.m_dStdDev &&
		m_strType == rhs.m_strType
		);
}

}	// namespace measurements
}	// namespace dynadjust
