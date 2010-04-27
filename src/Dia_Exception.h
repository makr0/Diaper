



/*
	
	This files defines the exception classes, that can be thrown by parser.
	Released under GPL v 2.0 AND on authors choice a later version, if it comes
*/
#ifndef _DIAPER_EXCEPTION_H

#define _DIAPER_EXCEPTION_H

#include <stdexcept>

class Diaper_Exception	: public std::runtime_error {

public : 
	Diaper_Exception(const char* str) : std::runtime_error(str) { }
	
};


class Invalid_Relation_Exception: public Diaper_Exception {
public: 
	Invalid_Relation_Exception(const char* str):Diaper_Exception(str) {}
};

class Invalid_Attribute_Exception: public Diaper_Exception {
public:
	Invalid_Attribute_Exception(const char* str):Diaper_Exception(str){}
};

class Invalid_Entity_Exception:	public Diaper_Exception {

public:
	Invalid_Entity_Exception(const char* str):Diaper_Exception(str) {}
} ;

class Invalid_Diaper_Format_Exception : public Diaper_Exception { 
public:
	Invalid_Diaper_Format_Exception( const char * str) : Diaper_Exception(str) {}
};
#endif