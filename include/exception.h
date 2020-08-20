#ifndef EXCEPTION_H

#define EXCEPTION_H

#include <iostream>
#include <exception>

struct ExceptIllegalFunctionCall : public std::exception
{
	const char * what () const throw () {
    	return "Ex: Illegal Function Call. Argument not allowed";
    }
};


#endif
