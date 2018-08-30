# DynAdjust
Least squares adjustment software

## License Details
As far as licence agreements are concerned, DynAdjust will be released with the Apache 2.0 Licence - http ://www.apache.org/licenses/LICENSE-2.0.   

Keep in mind however, that DynAdjust makes use of Boost C++, Apache's Xerces-C++ XML Parser (Apache 2.0 Licence) and CodeSynthesis XSD code.  Hence, the following licence agreements will also need to be taken into account with the Apache 2.0 Licence. 
•	https://www.boost.org/users/license.html 
•	https://www.codesynthesis.com/products/xsd/license.xhtml

You'll note that the free licence of CodeSynthesis XSD is GPL2, which requires any software that uses it to also be open source.  However, the CodeSynthesis site above states that you can use XSD generated code in proprietary applications provided that the lines of code do not exceed 10,000 lines.  The files generated for DynaML from XSD (dnaparser_pimpl.hxx/cxx and dnaparser_pskel.hxx/cxx), all of which have been heavily modified after they were originally generated, contain less than 10,000 lines. 
