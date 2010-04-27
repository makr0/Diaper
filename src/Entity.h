



/*
	
	This files defines the Entity abstraction
	Released under GPL v 2.0 AND on authors choice a later version, if it comes
*/

#ifndef __diaper_ENTITY_H
#define __diaper_ENTITY_H

#include <string>			// For std::string
#include <vector>			// For std::vector
#include "Attribute.h"		// For Attribute
//#include "Relation.h"
#include <list>
#include <iostream>
#include <exception>
#include "Dia_Exception.h"


class Relation;

	class Entity
	{

	public:	Entity(const std::string name);
	public: void add_Attribute(Attribute* attribute);

	public: void set_Weak_Flag(bool flag=true) {  is_Weak_ = flag ; is_Weak_Computed_ = !flag;}
	public: bool is_Weak()		   { return is_Weak_;  }

	public: void set_Associative_Flag(bool flag=true) { is_Associative_ = flag; }
	public: bool is_Associative()				  { return is_Associative_;  }

	public:	 std::string get_Name()					  { return name_; }
	public:  std::string set_Name(std::string name)	  { name_ = name; }
	public:  std::list<Attribute*>& get_Attributes()	{ return attributes_; }

	public:	 void add_Relation(Relation* rel);//	throw(Invalid_Relation_Exception);
	public:	 std::list<Relation*>& get_Relations()	{	return relations_;		   }

	/* The following functions return the primary key.
		* The first one returns the key by recursively traversing the hierarchy.
		* The second one checks the rec flag to determine if it has to traverse
	*/
	public:  std::list<Attribute*>& get_Primary_Key();
	public:  std::list<Attribute*>& get_Primary_Key(bool rec);
				 
	 
	/*	The name of the entity */
	protected: std::string name_;
	/* For fast retrival, when required */
	private:std::list<Attribute*>				primary_Key_;
	/* The set of attributes */
	private: std::list<Attribute*>	attributes_;
	/* The relations in which this entity is involved */
	private: std::list<Relation*>	relations_;
	/* Flag to determine if the entity is weak */
	private: bool is_Weak_;
	/* Flag to determine if the entity is associative */
	private: bool is_Associative_;
	/*The set of identifying relationships*/
	private: std::list<Relation*>identifying_Relations_;
	/* The following flag will be used when we have to consider weak entities, and will be set if their primary key has already been omputed */
	private: bool is_Weak_Computed_;
	};
	
#endif 