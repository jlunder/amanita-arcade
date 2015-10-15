/*
 * core_util.h
 *
 *  Created on: Oct 13, 2015
 *      Author: jlunder
 */

#ifndef CORE_UTIL_H_
#define CORE_UTIL_H_


#define cu_ctassert(pred) do { switch(0) { case 0: case (pred):; } } while(0)

#define cu_lengthof(x) (sizeof (x) / sizeof (*x))


void cu_abort(void);
void cu_log(char const * format, ...);

#define cu_error(...) do { \
	cu_log("%s:%d: in %s: ", __FILE__, __LINE__, __FUNCTION__); \
	cu_log(__VA_ARGS__); \
	cu_abort(); \
} while(0)

#define cu_verify(assertion) do { \
	if(!(assertion)) { \
		cu_error("Assertion failed: %s\n", #assertion); \
	} \
} while(0)


#endif /* CORE_UTIL_H_ */
