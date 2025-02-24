//============================================================================
// Name         : dnaiotpb.hpp
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
// Description  : PB2002 Tectonic Plate Boundary file io operations
//============================================================================

#ifndef DNAIOTPB_H_
#define DNAIOTPB_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <include/io/dnaiobase.hpp>
#include <include/config/dnatypes.hpp>

namespace dynadjust {
namespace iostreams {

/////////////////////////////////////////////////////////////
// Custom type to manage euler plate motion model parameters
template <class T1 = std::string, class T2 = double>
struct plate_motion_euler_t
{
	T1 plate_name;
	T1 pole_param_author;
	T2 pole_latitude;
	T2 pole_longitude;
	T2 pole_rotation_rate;

	plate_motion_euler_t() 
		: plate_name(""), pole_param_author("")
		, pole_latitude(T2()), pole_longitude(T2()), pole_rotation_rate(T2()) {}
	plate_motion_euler_t(const T1& name, const T1& author, const T2& lat, const T2& lon, const T2& rot) 
		: plate_name(name), pole_param_author(author)
		, pole_latitude(lat), pole_longitude(lon), pole_rotation_rate(rot) {}
	plate_motion_euler_t(const plate_motion_euler_t <T1, T2> &p) 
		: plate_name(p.plate_name), pole_param_author(p.pole_param_author)
		, pole_latitude(p.pole_latitude), pole_longitude(p.pole_longitude)
		, pole_rotation_rate(p.pole_rotation_rate) { }

	bool operator== (const plate_motion_euler_t<T1, T2>& rhs) {
		return equals(plate_name, rhs.plate_name);
	}

	bool operator< (const plate_motion_euler_t<T1, T2>& rhs) const
	{
		if (plate_name == rhs.plate_name) {
			if (pole_latitude == rhs.pole_latitude) 	// don't think this is needed
				return pole_longitude < rhs.pole_longitude;
			else
				return pole_latitude < rhs.pole_latitude;
		}
		else
			return plate_name < rhs.plate_name;
	}

};

typedef plate_motion_euler_t<std::string, double> plate_motion_euler;
typedef std::vector<plate_motion_euler> v_plate_motion_eulers;
typedef v_plate_motion_eulers::iterator it_plate_motion_euler;
/////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////
// Custom type to manage cartesian plate motion model parameters
template <class T1 = std::string, class T2 = double>
struct plate_motion_cartesian_t
{
	T1 plate_name;
	T1 pole_param_author;
	T2 x_rotation;
	T2 y_rotation;
	T2 z_rotation;

	plate_motion_cartesian_t()
		: plate_name(""), pole_param_author("")
		, x_rotation(T2()), y_rotation(T2()), z_rotation(T2()) {}
	plate_motion_cartesian_t(const T1& name, const T1& author, const T2& x, const T2& y, const T2& z)
		: plate_name(name), pole_param_author(author)
		, x_rotation(x), y_rotation(y), z_rotation(z) {}
	plate_motion_cartesian_t(const plate_motion_cartesian_t <T1, T2> &p)
		: plate_name(p.plate_name), pole_param_author(p.pole_param_author)
		, x_rotation(p.x_rotation), y_rotation(p.y_rotation), z_rotation(p.z_rotation) { }

	bool operator== (const plate_motion_cartesian_t<T1, T2>& rhs) {
		return equals(plate_name, rhs.plate_name);
	}

	bool operator< (const plate_motion_cartesian_t<T1, T2>& rhs) const
	{
		if (plate_name == rhs.plate_name) {
			if (x_rotation == rhs.x_rotation) {			// don't think comparing x, y and z is needed
				if (y_rotation == rhs.y_rotation)
					return z_rotation < rhs.z_rotation;
				else
					return y_rotation < rhs.y_rotation;
			}
			else
				return x_rotation < rhs.x_rotation;
		}
		else
			return plate_name < rhs.plate_name;
	}

};

typedef plate_motion_cartesian_t<std::string, double> plate_motion_cartesian;
typedef std::vector<plate_motion_cartesian> v_plate_motion_cartesians;
typedef v_plate_motion_cartesians::iterator it_plate_motion_cartesian;
/////////////////////////////////////////////////////////////


class dna_io_tpb : public dna_io_base
{
public:
	dna_io_tpb(void) {};
	dna_io_tpb(const dna_io_tpb& tpb) : dna_io_base(tpb) {};
	virtual ~dna_io_tpb(void) {};

	dna_io_tpb& operator=(const dna_io_tpb& rhs);

	void load_tpb_file(const std::string& tpb_filename, v_string_v_doubledouble_pair& global_plates);
	void load_tpp_file(const std::string& tpp_filename, v_plate_motion_eulers& plate_pole_parameters);

	bool validate_plate_files(v_string_v_doubledouble_pair& global_plates, 
		v_plate_motion_eulers& plate_pole_parameters, std::string& message);

protected:
	
};

}	// namespace measurements
}	// namespace dynadjust

#endif
