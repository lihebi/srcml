#!/bin/bash

# test framework
source $(dirname "$0")/framework_test.sh

# test language_attribute_cpp
define output <<- 'OUTPUT'
	<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
	<unit xmlns="http://www.srcML.org/srcML/src" xmlns:cpp="http://www.srcML.org/srcML/cpp" revision="REVISION" language="C++"/>
	OUTPUT

echo -n "" | src2srcml -l "C++"

check 3<<< "$output"

echo -n "" | src2srcml --language "C++"

check 3<<< "$output"

echo -n "" | src2srcml --language="C++"

check 3<<< "$output"

