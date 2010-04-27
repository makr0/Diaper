

/*
	
	This files defines the parser abstraction.
	Released under GPL v 2.0 AND on authors choice a later version, if it comes
*/
/*
	* This file is the one that is to be used by programs that want to use the parser.
	* This could be as a seprate dll itself.


	IMPORTANT ASSUMPTION:
		THE FILE IS IN THE RIGHT FORMAT, AND ALL THE FIELDS THAT WE REQUIRE ARE PRESENT IN ORDER
		Importantly it has been tested only with dia 0.94 and the parser is HIGHLY optimized for 0.94
*/

#ifndef __diaper_DIAPER_H
#define __diaper_DIAPER_H

#include <map>			// For std::map
#include <string>		// For std::string
#include <list>			// For std::list
#include <fstream>		// For std::ios::exception
#include <fstream>		// For std::ifstream and std::ofstream.
#include <utility>		// For std::pair
#include "Attribute.h"  // For Attribute
#include "Entity.h"		// For Entity
#include "Relation.h"   // For Relation
#include "Dia_Exception.h"	// For exceptions related to Diaper

class Diaper
{

/*
	* The following functions parse the file for the object mentioned 
*/
private:
	enum Type 
	{
		ENTITY_,
		ATTRIBUTE_,
		LINE_,
		RELATION_,
		PARTICIPATION_
	};
/*
	* The following functions take as argument the string of the format
	* <dia:object.......
	* to get the id of the entity.
*/

private: void parse_Entity(const std::string&);
private: void parse_Attribute(const std::string&);
private: void parse_Relation(const std::string&);
private: void parse_Line(const std::string&);
private: void parse_Participation(const std::string& )	{ std::cerr << "We don't deal with Participation as of now\n" ; exit(-1); }

 /*	
	* This function would return the object Id for a particular
	* <dia:object....>
*/
private: std::string get_Object_Id(const std::string& str);
 /*
	
 */

/* 
	*	Used by above routines to skip n lines in the text
*/
private: void skip_Lines(const int n);

/*
	* Get value from 
	* val="
*/
bool get_Next_Value();
 /*
	* Used by the parse_* functions to determine the type of object
*/
private: Type get_Object_Type(const std::string & str);
private: static const int ENTITY_SKIP_LINES	  =	24;
private: static const int ATTRIBUTE_SKIP_LINES= 24;
private: static const int RELATION_SKIP_LINES = 24;
private: static const int LINE_SKIP_LINES	  = 24;



public: Diaper()	{ line_Number_ = 0;}

	   /*
		   * The following function would parse an input dia file and fill the data structures up
	   */
public: void parse_File(const char * name) throw (std::ios::failure, Diaper_Exception);
	   /*
			* The following function would 
			* a)	Map the ER model to a Relational model
			* b)	Normalize it.
		*/
public:void write_Sql(const char* name) throw(std::ios::failure, Diaper_Exception);


private:void resolve_Line_Pairs() throw ( Invalid_Relation_Exception);

private:	typedef std::pair<int, Entity*> Entity_Int_Pair_T;
private:	typedef std::pair<int, Attribute*> Attribute_Int_Pair_T;
private:	typedef std::pair<int, Relation*> Relation_Int_Pair_T;
private:	typedef std::pair<int, int>		Int_Int_Pair_T;

// These interfaces must be coherent, i.e. they must take arguments of the same type
//private:	std::string	create_Table(const std::string&, const std::list<Attribute*>& ent);
private:	std::string create_Rec_Table(Entity* ent);
private:	std::string		create_Relation_Table(const Relation* rel);

private: Int_Int_Pair_T get_Relation_Cardinality(std::string& name);
private: Int_Int_Pair_T get_Line_Connection();

  /*
	 * The map would exist for a specific purpose, wherein each pair
	 * [id, X] denotes the id that has been assigned by dia and X a pointer to the actual Object
 */

private: std::map<int, Entity*>		 entities_;
private: std::map<int, Attribute*>	 attributes_;
private: std::map<int, Relation*>	 relations_;
private: std::map<int, Type>		 cache;

 /*
	* It is possible to draw line before doing anything else,
	* The following will just include the information about all those to-do pairs
*/
private: struct Line_Rep	{
			int id_From;
			int id_To;
			Type line;
	};

private: std::list<Line_Rep> to_Do_Pairs;
private: std::ifstream input_File_;
private: int line_Number_;



};

#endif /* End isa_XML_Parser */