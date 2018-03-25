// $Id: libfns.cpp,v 1.4 2015-07-03 14:46:41-07 - - $
/*
* Name: Michael Ettinger
* ID:   mcetting : 1559249
* Email:mcetting@ucsc.edu
* Date: 1/22/2018
*/
#include "libfns.h"

//
// This algorithm would be more efficient with operators
// *=, /=2, and is_odd.  But we leave it here.
//

// modified pow function because it was broken
bigint pow (const bigint &base_arg, const bigint &exponent_arg) {
    // bigint* ptr = &base_arg;
    bigint result(base_arg);

    for(bigint i("0"); i < exponent_arg - bigint("1"); i = i + bigint("1")){
        result = result * base_arg;
    }

    return result;
}

