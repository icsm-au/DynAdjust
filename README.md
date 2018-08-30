# DynAdjust
DynAdjust is a rigorous, high performance least squares adjustment application. It has been designed
to estimate 3D station coordinates and uncertainties for both small and extremely large geodetic networks,
and can be used for the adjustment of a large array of Global Navigation Satellite System
(GNSS) and conventional terrestrial survey measurement types. On account of the phased adjustment
approach used by DynAdjust, the maximum network size which can be adjusted is effectively
unlimited, other than by the limitations imposed by a computer’s processor, physical memory and
operating system memory model. Example projects where DynAdjust can and has been used include
the adjustment of small survey control networks, engineering surveys, deformation monitoring
surveys, national and state geodetic networks and digital cadastral database upgrade initiatives.
DynAdjust provides the following capabilities:
• Import of data in geographic, cartesian and/or projection (UTM) coordinates contained in
DNA, DynaML and SINEX data formats;
• Input of a diverse range of measurement types;
• Transformation of station coordinates and measurements between several static and dynamic
reference frames;
• Rigorous application of geoid–ellipsoid separations and deflections of the vertical;
• Simultaneous (traditional) and phased adjustment modes;
• Automatic segmentation and adjustment of extremely large networks in an efficient manner;
• Rigorous estimation of positional uncertainty for all points in a network;
• Detailed statistical analysis of adjusted measurements and station corrections;
• Production of high quality network plots;
• Automated processing and analysis with minimal user interaction.


## License Details
DynAdjust has an Apache 2.0 Licence - http ://www.apache.org/licenses/LICENSE-2.0.   

NOTE: DynAdjust makes use of Boost C++, Apache's Xerces-C++ XML Parser (Apache 2.0 Licence) and CodeSynthesis XSD code. Hence, the following licence agreements will also need to be taken into account with the Apache 2.0 Licence. 

* https://www.boost.org/users/license.html 
* https://www.codesynthesis.com/products/xsd/license.xhtml

The free licence of CodeSynthesis XSD is GPL2, which requires any software that uses it to also be open source.  However, the CodeSynthesis site above states that you can use XSD generated code in proprietary applications provided that the lines of code do not exceed 10,000 lines.  The files generated for DynaML from XSD (dnaparser_pimpl.hxx/cxx and dnaparser_pskel.hxx/cxx), all of which have been heavily modified after they were originally generated, contain less than 10,000 lines. 
