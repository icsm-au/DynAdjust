//============================================================================
// Name         : dnafile_mapping.cpp
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
// Description  : DynAdjust Memory Mapped File Library
//============================================================================

#include <include/memory/dnafile_mapping.hpp>

namespace dynadjust { 
namespace memory {

block_map_t::block_map_t()
		: data_size_(0), region_offset_(0) 
{

}
	

block_map_t::block_map_t(const size_t& size)
		: data_size_(size), region_offset_(0) 
{

}
	

block_map_t::block_map_t(const block_map_t &p) 
		: data_size_(p.data_size_)
		, region_offset_(p.region_offset_)
		, region_ptr_(p.region_ptr_) 
{

}
	

block_map_t& block_map_t::operator=(const block_map_t& rhs) 
{
	if (this == &rhs)
		return *this;
	data_size_ = rhs.data_size_;
	region_offset_ = rhs.region_offset_;
	region_ptr_ = rhs.region_ptr_;
	return *this;
}
	

bool block_map_t::operator==(const block_map_t& rhs) const 
{
	return (
		data_size_ == rhs.data_size_ &&
		region_offset_ == rhs.region_offset_ &&
		region_ptr_ == rhs.region_ptr_
		);
}
	

void block_map_t::MapRegion(FileMapPtr file_map_ptr) {
	region_ptr_.reset(
		new mapped_region(
			*file_map_ptr, 
			read_write, 
			region_offset_, 
			data_size_
			)
		);
}


// class to hold addresses and sizes for all matrices 
// in a vector of segmented blocks
vmat_file_map::vmat_file_map()
{

}
	

vmat_file_map::vmat_file_map(const string& filePath, bool remove_mapped_file) 
	: filePath_(filePath), remove_mapped_file_(remove_mapped_file) 
{ 
		file_map_ptr_.reset(new file_mapping(filePath_.c_str(), read_write));
}
	

vmat_file_map::~vmat_file_map() 
{
	if (remove_mapped_file_)
		if (exists(filePath_))
			file_mapping::remove(filePath_.c_str());
}
	

void vmat_file_map::reserveblockMapRegions(const UINT32& size)
{
	vblockMapRegions_.reserve(size); 
}
	

void vmat_file_map::addblockMapRegion(const block_map_t& map) 
{
	vblockMapRegions_.push_back(map); 
}


void vmat_file_map::setnewFilePath(const string& filePath, bool remove_mapped_file) 
{
	filePath_ = filePath; 
	remove_mapped_file_ = remove_mapped_file;
}
	

void vmat_file_map::CreateFileMapping() 
{
	file_map_ptr_.reset(new file_mapping(filePath_.c_str(), read_write));
}
	

void vmat_file_map::MapRegion(const UINT32 block) 
{
	vblockMapRegions_.at(block).MapRegion(file_map_ptr_);
}
	

}	// namespace memory 
}	// namespace dynadjust 


