#!/bin/sh
#
#  Copyright 2014 by Janos Kasza
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#

OUTFILE="./test_results/perf_1.log"

rm -i $OUTFILE
for i in `seq 10 30`; do 
	make clean
	ADDFLAGS=-DSEGMENT_BITS=$i make
	echo "SEGMENT_BITS=$i -- " `./test_perf -s up_to_10_billion 2>&1 | grep tests | head -1` >> $OUTFILE
done
