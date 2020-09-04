// metadata.cpp : Defines the entry point for the console application.
//
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>
#include <time.h>

#include <boost/timer/timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/iostreams/detail/absolute_path.hpp>

using namespace std;
using namespace boost;
using namespace boost::filesystem;
using namespace boost::posix_time;
using namespace boost::program_options;

boost::mutex cout_mutex;
boost::mutex import_file_mutex;

#include <include/exception/dnaexception.hpp>

#include <include/config/dnaversion.hpp>
#include <include/config/dnaconsts.hpp>
#include <include/config/dnaconsts-interface.hpp>
#include <include/config/dnaoptions.hpp>
#include <include/config/dnaoptions-interface.hpp>

#include <include/functions/dnastringfuncs.hpp>
#include <include/functions/dnaiostreamfuncs.hpp>
#include <include/functions/dnafilepathfuncs.hpp>
#include <include/functions/dnastrmanipfuncs.hpp>

#include <include/parameters/dnadatum.hpp>

using namespace dynadjust;
using namespace dynadjust::exception;
using namespace dynadjust::datum_parameters;

void processGNSSMeasurement(string& dnaMeasurementGNSS, string& referenceFrame, string& epoch, size_t& lineNo, size_t& measurementUpdateCount)
{
	string tmp(dnaMeasurementGNSS);

	// Validate reference frame and epoch.  Throws on error
	CDnaDatum datum;
	datum.SetDatumFromName(referenceFrame, epoch);

	string FirstTag = "<First>";
	string referenceframeTagOpen = "<ReferenceFrame>";
	string referenceframeTagClose = "</ReferenceFrame>";
	string epochTagOpen = "<Epoch>";
	string epochTagClose = "</Epoch>";
	string closeTag = "</";

	string referenceFrameElement, epochElement;
	referenceFrameElement = referenceframeTagOpen + referenceFrame + referenceframeTagClose + "\n        ";
	epochElement = epochTagOpen + epoch + epochTagClose + "\n        ";

	size_t pos;

	bool updated(false);

	///////////////////////////////////////////////////////////
	// Look for the existence of <ReferenceFrame> in data
	// If none, add new <ReferenceFrame>
	if ((pos = tmp.find(referenceframeTagOpen)) == string::npos)
	{
		// Okay, no reference frame tag.  Add it before <First>
		if ((pos = tmp.find(FirstTag)) == string::npos)
		{
			stringstream ss;
			ss << "processGNSSMeasurement(): Could not find <First> element in:" << endl << 
				dnaMeasurementGNSS << endl;
			throw XMLInteropException(ss.str(), (int)lineNo);
		}

		tmp.insert(pos, referenceFrameElement);
		updated = true;
	}

	///////////////////////////////////////////////////////////
	// Look for the existence of <Epoch> in data
	// If none, add new <ReferenceFrame>
	if ((pos = tmp.find(epochTagOpen)) == string::npos)
	{
		// Okay, no reference frame tag.  Add it before <First>
		if ((pos = tmp.find(FirstTag)) == string::npos)
		{
			stringstream ss;
			ss << "processGNSSMeasurement(): Could not find <First> element in:" << endl << 
				dnaMeasurementGNSS << endl;
			throw XMLInteropException(ss.str(), (int)lineNo);
		}

		tmp.insert(pos, epochElement);
		updated = true;
	}

	dnaMeasurementGNSS = tmp;
	if (updated)
		measurementUpdateCount++;
}

void GetDefaultReferenceFrameEpoch(string& sBuf, string& referenceFrame, string& epoch)
{
	size_t pos;
	string referenceFrameAttribute = "referenceframe";
	string epochAttribute = "epoch";

	string tmp;

	if ((pos = sBuf.find(referenceFrameAttribute)) != string::npos)
	{
		// Okay, get the reference frame
		// Advance to =
		while (sBuf.at(pos) != '\"')
			pos++;
		pos++;

		tmp = sBuf.substr(pos);
		if ((pos = tmp.find("\"")) != string::npos)
			tmp = tmp.substr(0, pos);

		referenceFrame = tmp;
	}

	if ((pos = sBuf.find(epochAttribute)) != string::npos)
	{
		// Okay, get the epoch
		// Advance to =
		while (sBuf.at(pos) != '\"')
			pos++;
		pos++;

		tmp = sBuf.substr(pos);
		if ((pos = tmp.find("\"")) != string::npos)
			tmp = tmp.substr(0, pos);

		epoch = tmp;
	}
}
	

void processFile(std::ifstream* ifsFile, std::ofstream* ofsFile, string& defaultFrame, string& defaultEpoch, 
	size_t& measurementUpdateCount, size_t& unsureFrameCount, size_t& unsureEpochCount)
{
	string sBuf, tmp;

	string referenceFrame(GDA94_s);
	string epoch("01.01.1994");
	
	string openingDnaMeasurementTag = "<DnaMeasurement>";
	string closingDnaMeasurementTag = "</DnaMeasurement>";

	string dnaxmlFormatTag = "<DnaXmlFormat";

	string gType = "<Type>G</Type>";
	string xType = "<Type>X</Type>";
	string yType = "<Type>Y</Type>";

	const string xmlComment = "<!--";
	// <!--[BASELINE REFFRAME] ITRF2005-->
	const string metadataReferenceFrame = "[BASELINE REFFRAME]";
	// <!--[BASELINE EPOCH] 19.08.2010-->
	const string metadataEpoch = "[BASELINE EPOCH]";

	stringstream dnameasurementStream;
	bool newMeasurement(false);

	dnameasurementStream.str("");

	size_t lineNo = 0;
	size_t pos;
	
	CDnaDatum datum;

	while (ifsFile)
	{
		if (ifsFile->eof())
			break;

		lineNo++;

		try {
			getline((*ifsFile), sBuf);			
		}
		catch (...) {
			if (ifsFile->eof())
				return;
			
			stringstream ss;
			ss << "processFile(): Could not read from the station file." << endl;
			throw XMLInteropException(ss.str(), (int)lineNo);
		}

		// Reached a DnaMeasurement opening tag?  
		
		if (sBuf.find(dnaxmlFormatTag) != string::npos)
		{
			// Get the default reference frame and epoch
			GetDefaultReferenceFrameEpoch(sBuf, referenceFrame, epoch);

			// Validate reference frame and epoch.  Throws on error
			datum.SetDatumFromName(referenceFrame, epoch);

			// Set defaults
			defaultFrame = referenceFrame;
			defaultEpoch = epoch;
		}

		// Reached a DnaMeasurement opening tag?
		if (sBuf.find(openingDnaMeasurementTag) != string::npos)
		{	
			// yes, found an opening tag!
			newMeasurement = true;

			// Reset the stream
			dnameasurementStream.str("");

			// Add this string to the stringstream (for subsequent processing)
			dnameasurementStream << sBuf << endl;
		
		}
		// Reached a DnaMeasurement closing tag?
		else if (sBuf.find(closingDnaMeasurementTag) != string::npos)
		{	
			// yes, found a closing tag!
			dnameasurementStream << sBuf << endl;

			// capture the measurement
			string dnaMeasurementGNSS = dnameasurementStream.str();

			// Is this a GNSS measurement?  if not, dump and continue
			if (dnaMeasurementGNSS.find(gType) != string::npos)
			{
				// Process GNSS measurement
				processGNSSMeasurement(dnaMeasurementGNSS, referenceFrame, epoch, lineNo, measurementUpdateCount);
			}
			else if (dnaMeasurementGNSS.find(xType) != string::npos)
			{
				// Process GNSS measurement
				processGNSSMeasurement(dnaMeasurementGNSS, referenceFrame, epoch, lineNo, measurementUpdateCount);
			}
			else if (dnaMeasurementGNSS.find(yType) != string::npos)
			{
				// Process GNSS measurement
				processGNSSMeasurement(dnaMeasurementGNSS, referenceFrame, epoch, lineNo, measurementUpdateCount);
			}
			
			// Write dnaMeasurementGNSS to output file
			(*ofsFile) << dnaMeasurementGNSS;

			newMeasurement = false;
		}
		else
		{
			// At this point, the line is one of the following:
			// - A line before the first measurement (in which case, newMeasurement is false)
			// - An XML element within DnaMeasurement (in which case, newMeasurement is true)
			// - A comment within DnaMeasurement (in which case, newMeasurement is true)

			// Is this line within a DnaMeasurement
			if (newMeasurement)
			{
				// Yes, add this string to the stringstream (for subsequent processing)
				dnameasurementStream << sBuf << endl;
			}
			else
			{
				// A new measurement hasn't been reached yet, so this line is either:
				// - Before the first DnaMeasurement
				// - A comment preceding a DnaMeasurement

				// Dump the line to output file
				(*ofsFile) << sBuf << endl;
			}			

			// Now check for Reference frame and epoch comments
			tmp = trimstr(sBuf);

			// Is this line a comment?
			if (tmp.find(xmlComment) == string::npos)
			{
				// No, this is a measurement element.
				
				// Check for an existing ReferenceFrame tag
			}
			else
			{
				// At this point, tmp is a comment.

				// Is this line a reference frame metadata tag in the comment?
				if (tmp.find(metadataReferenceFrame) != string::npos)
				{
					if ((pos = tmp.find("]", 0)) != string::npos)
					{
						referenceFrame = tmp.substr(pos+1);
						if ((pos = referenceFrame.find("-->", 0)) != string::npos)
							referenceFrame = trimstr(referenceFrame.substr(0, pos));

						if (iequals(referenceFrame, "unsure"))
						{
							referenceFrame = defaultFrame;
							unsureFrameCount++;
						}

						if (iequals(referenceFrame, "WGS84"))
						{
							referenceFrame = "ITRF1989";
						}
					}
				}

				// Is this line a epoch metadata tag in the comment?
				else if (tmp.find(metadataEpoch) != string::npos)
				{
					if ((pos = tmp.find("]", 0)) != string::npos)
					{
						epoch = tmp.substr(pos+1);
						if ((pos = epoch.find("-->", 0)) != string::npos)
							epoch = trimstr(epoch.substr(0, pos));

						if (iequals(epoch, "unsure"))
						{
							epoch = "01.01.2005";
							unsureEpochCount++;
						}
					}
				}
			}
		}		
	}

}

int main(int argc, char* argv[])
{
	stringstream ss_err;

	string cmd_line_banner;
	fileproc_help_header(&cmd_line_banner);

	project_settings p;
	variables_map vm;
	positional_options_description positional_options;

	options_description standard_options("+ " + string(ALL_MODULE_STDOPT), PROGRAM_OPTIONS_LINE_LENGTH);
	options_description generic_options("+ " + string(ALL_MODULE_GENERIC), PROGRAM_OPTIONS_LINE_LENGTH);

	string cmd_line_usage("+ ");
	cmd_line_usage.append(__BINARY_NAME__).append(" usage:  ").append(__BINARY_NAME__).append(" ").append(" [options]");
	options_description allowable_options(cmd_line_usage, PROGRAM_OPTIONS_LINE_LENGTH);

	try {
		standard_options.add_options()
			(IMPORT_FILE_F, value< vstring >(&p.i.input_files), 
				"Station and measurement input file(s). Switch is not required.")
			;

		generic_options.add_options()
			(VERBOSE, value<UINT16>(&p.g.verbose),
				string("Give detailed information about what ").append(__BINARY_NAME__).append(" is doing.\n  0: No information (default)\n  1: Helpful information\n  2: Extended information\n  3: Debug level information").c_str())
			(QUIET,
				string("Suppresses all explanation of what ").append(__BINARY_NAME__).append(" is doing unless an error occurs.").c_str())
			(VERSION_V, "Display the current program version.")
			(HELP_H, "Show this help message.")
			(HELP_MODULE, value<string>(),
				"Provide help for a specific help category.")
			;

		allowable_options.add(standard_options).add(generic_options);

		// add "positional options" to handle command line tokens which have no option name
		positional_options.add(IMPORT_FILE, -1);

		command_line_parser parser(argc, argv);
		store(parser.options(allowable_options).positional(positional_options).run(), vm);
		notify(vm);
	} 
	catch (const std::exception& e) {
		cout_mutex.lock();
		cout << "- Error: " << e.what() << endl;
		cout << cmd_line_banner << allowable_options << endl;
		cout_mutex.unlock();
		return EXIT_FAILURE;
	}

	if (argc < 2)
	{
		cout << endl << "- Nothing to do - no files or options provided. " << endl << endl;  
		cout << cmd_line_banner << allowable_options << endl;
		return EXIT_FAILURE;
	}

	if (vm.count(VERSION))
	{
		cout << cmd_line_banner << endl;
		return EXIT_SUCCESS;
	}

	if (vm.count(HELP))
	{
		cout << cmd_line_banner << allowable_options << endl;
		return EXIT_SUCCESS;
	}

	if (vm.count(HELP_MODULE))
	{
		cout << cmd_line_banner;
		string original_text = vm[HELP_MODULE].as<string>();
		string help_text = str_upper<string>(original_text);

		if (str_upper<string, char>(ALL_MODULE_STDOPT).find(help_text) != string::npos) {
			cout << standard_options << endl;
		}
		else if (str_upper<string, char>(ALL_MODULE_GENERIC).find(help_text) != string::npos) {
			cout << generic_options << endl;
		}
		else {
			cout << endl << "- Error: Help module '" <<
				original_text << "' is not in the list of options." << endl;
			return EXIT_FAILURE;
		}

		return EXIT_SUCCESS;
	}

	// Normalise files using input folder
	for_each(p.i.input_files.begin(), p.i.input_files.end(),
		[&p] (string& file) { 
			formPath<string>(p.g.input_folder, file);
	}
	);

	UINT32 stnCount(0), msrCount(0), clusterID(0);

	size_t i, nfiles(p.i.input_files.size());		// for each file...
	string input_file, output_file, ss;
	string defaultFrame, defaultEpoch;
	
	ostringstream ss_time, ss_msg;
	milliseconds elapsed_time(milliseconds(0));
	ptime pt;

	if (!p.g.quiet)
	{
		cout << endl << cmd_line_banner;
		cout << "+ Parsing: " << endl;
	}
	
	cpu_timer time;	// constructor of boost::timer::cpu_timer calls start()

	std::ifstream*	ifsDynaML_;
	size_t		sifsFileSize_, measurementUpdateCount(0), unsureFrameCount(0), unsureEpochCount(0);

	std::ofstream*	ofsDynaML_;

	ifsDynaML_ = 0;
	ofsDynaML_ = 0;

	size_t strlen_arg = 0;
	for_each(p.i.input_files.begin(), p.i.input_files.end(),
		[&strlen_arg](string& file) {
			if (leafStr<string>(file).length() > strlen_arg)
				strlen_arg = leafStr<string>(file).length();
	});
	strlen_arg += (6 + PROGRESS_PERCENT_04);

	for (i=0; i<nfiles; i++)
	{
		input_file = p.i.input_files.at(i);

		time.start();

		if (!exists(input_file))
		{
			input_file = formPath<string>(p.g.input_folder, input_file);
			if (!exists(input_file))
			{	
				cout << "- Error:  " << input_file << " does not exist" << endl;
				return EXIT_FAILURE;
			}
		}

		// Form output file path
		stringstream ss_outputfile("");
		ss_outputfile << path(input_file).stem().generic_string();
		ss_outputfile << ".edit.xml";
		output_file = ss_outputfile.str();

		ss = leafStr<string>(p.i.input_files.at(i)) + "... ";
		if (!p.g.quiet)
			cout << "  " << setw(strlen_arg) << left << ss;
		
		// Obtain exclusive use of the input file pointer
		import_file_mutex.lock();
		
		try 
		{
			if (ifsDynaML_)
			{
				ifsDynaML_->close();
				delete ifsDynaML_;
			}

			ifsDynaML_ = new std::ifstream;

			// Open and seek to end immediately after opening.
			file_opener(ifsDynaML_, input_file, ios::in | ios::ate, ascii, true);
		
			// get file size and return to start
			sifsFileSize_ = (size_t)ifsDynaML_->tellg();
			ifsDynaML_->seekg(0, ios::beg);
		}
		catch (const ios_base::failure& f) {	
			cout << "- Error: An error was encountered when opening " << input_file << "." << endl;
			cout << "  Check that the file exists and that the file is not already opened." << endl << f.what();
			return EXIT_FAILURE;
		}
		catch (...) {	
			cout << "- Error: An error was encountered when opening " << input_file << "." << endl;
			cout << "  Check that the file exists and that the file is not already opened.";
			return EXIT_FAILURE;
		}

		try 
		{
			if (ofsDynaML_)
			{
				ofsDynaML_->close();
				delete ofsDynaML_;
			}

			ofsDynaML_ = new std::ofstream;

			// Open output file
			file_opener(ofsDynaML_, output_file, ios::out, ascii);

		}
		catch (const ios_base::failure& f) {	
			cout << "- Error: An error was encountered when opening " << output_file << "." << endl << f.what();
			return EXIT_FAILURE;
		}
		catch (...) {	
			cout << "- Error: An error was encountered when opening " << output_file << "." << endl;
			return EXIT_FAILURE;
		}

		try {
			measurementUpdateCount = 0;
			unsureFrameCount = 0;
			unsureEpochCount = 0;

			// Process the file
			processFile(ifsDynaML_, ofsDynaML_, defaultFrame, defaultEpoch, 
				measurementUpdateCount, unsureFrameCount, unsureEpochCount);
		}
		catch (const runtime_error& e) {
			cout << endl << "- Error: " << e.what() << endl;
			return EXIT_FAILURE;
		}catch (const XMLInteropException& e) {
			cout << endl << "- Error: " << e.what() << endl;
			return EXIT_FAILURE;
		}
		catch (...) 
		{
			cout << "+ Exception of unknown type!\n";
			return EXIT_FAILURE;
		} 

		// Finish up
		time.stop();
		elapsed_time = milliseconds(time.elapsed().wall/MILLI_TO_NANO);

		if (!p.g.quiet)
		{
			cout << "done." << endl << endl;
			cout << "+ Output file:                          " << output_file << endl;
			cout << "+ Default reference frame:              " << defaultFrame << endl;
			cout << "+ Default epoch:                        " << defaultEpoch << endl;
			cout << "+ No. DnaMeasurement records updated:   " << measurementUpdateCount << endl;
			cout << "+ No. comments with \"unsure\" frame:     " << unsureFrameCount << endl;
			cout << "+ No. comments with \"unsure\" epoch:     " << unsureEpochCount << endl;
			        
		}

		// Close input file
		try {
			if (ifsDynaML_ != 0)
			{
				if (ifsDynaML_->good())
					ifsDynaML_->close();
				delete ifsDynaML_;
			}
		}
		catch (const std::ifstream::failure& e)
		{
			if (ifsDynaML_->rdstate() & std::ifstream::eofbit)
			{
				ifsDynaML_ = 0;
			}
			cout << "- Error: An error was encountered when opening " << input_file << "." << endl << endl << e.what() << endl << "  Check that the file exists and that the file is not already opened.";
			return EXIT_FAILURE;
		}

		ifsDynaML_ = 0;

		// Close output file
		try {
			if (ofsDynaML_ != 0)
			{
				if (ofsDynaML_->good())
					ofsDynaML_->close();
				delete ofsDynaML_;
			}
		}
		catch (const std::ifstream::failure& e)
		{
			if (ofsDynaML_->rdstate() & std::ifstream::eofbit)
			{
				ofsDynaML_ = 0;
			}
			cout << "- Error: An error was encountered when opening " << output_file << "." << endl << endl << e.what() << endl;
			return EXIT_FAILURE;
		}

		ofsDynaML_ = 0;

		if (!p.g.quiet)
		{
			string time_message = formatedElapsedTime<string>(&elapsed_time, "+ File processing time:                 ");
			cout << time_message << endl << endl;
		}

		// release file pointer mutex
		import_file_mutex.unlock();

	}

	return EXIT_SUCCESS;

}




	
