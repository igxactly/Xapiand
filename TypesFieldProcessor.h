/*
 * Copyright deipi.com LLC and contributors. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef __Xapiand__TypesFieldProcessor__
#define __Xapiand__TypesFieldProcessor__

#include <string.h>
#include <sstream>
#include <xapian.h>
#include "utils.h"


class NumericFieldProcessor : public Xapian::FieldProcessor {
	std::string prefix;

	public:
		NumericFieldProcessor(const std::string &prefix);
		Xapian::Query operator()(const std::string &str);
};


class LatLongFieldProcessor : public Xapian::FieldProcessor {
	std::string prefix;
	
	public:
		LatLongFieldProcessor(const std::string &prefix);
		Xapian::Query operator()(const std::string &str);
};


class BooleanFieldProcessor : public Xapian::FieldProcessor {
	std::string prefix;
	
	public:
		BooleanFieldProcessor(const std::string &prefix);
		Xapian::Query operator()(const std::string &str);
};


class DateFieldProcessor : public Xapian::FieldProcessor {
	std::string prefix;
	
	public:
		DateFieldProcessor(const std::string &prefix);
		Xapian::Query operator()(const std::string &str);
};

class LatLongDistanceFieldProcessor: public Xapian::FieldProcessor {
	std::string prefix;
	std::string field;
	double coords_[3];
	
public:
	Xapian::Query operator()(const std::string &str);
	LatLongDistanceFieldProcessor(std::string prefix, std::string field);
};


class DateTimeValueRangeProcessor: public Xapian::ValueRangeProcessor {
	std::string prefix;
	Xapian::valueno valno;

	public:
		DateTimeValueRangeProcessor(Xapian::valueno slot_, const std::string prefix);
		Xapian::valueno operator()(std::string &begin, std::string &end);
};


#endif /* defined(__Xapiand__TypesFieldProcessor__) */