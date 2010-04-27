


/*
	
	This files defines the Attribute abstraction and implementation..
	Released under GPL v 2.0 AND on authors choice a later version, if it comes
*/

#ifndef __isa_ATTRIBUTE_H
#define __isa_ATTRIBUTE_H

#include <string>
#include <assert.h>

class Entity;

class Attribute 
{

public:	Attribute(const std::string name, const std::string type) { name_ = name; type_ = type; this->entity_ = NULL;}

public: void set_Primary_Key(bool flag=true) { is_Primary_Key_ = flag; }
public: bool is_Primary_Key()				 { return is_Primary_Key_; }

public: void set_Weak_Key(bool flag = true ) { is_Weak_Key_		=	flag; }
public: bool is_Weak_Key()					 { return is_Weak_Key_; }

public:	void set_Derived(bool flag=true)	{ is_Derived_	= flag; }
public: bool is_Derived()					{ return is_Derived_;	}

public:	void set_Multivalued(bool flag=true)	{ is_Multivalued_  = flag; }
public: bool is_Multivalued()					{	return is_Multivalued_; }

public: std::string get_Name()					{ return name_; }
public: std::string get_Type()					{ return type_; }

public: void set_Entity(Entity* ent)			{ assert( entity_ == NULL );	entity_ = ent; }
public: Entity* get_Entity()					{ return entity_;	}

/* The name of the attribute */
private: std::string name_;
/* The type of the attribute */
private: std::string type_;
/* The entity attached to this attribute */

private: Entity* entity_;
/* The status flags for the attribute*/
private: bool is_Primary_Key_;
private: bool is_Weak_Key_;
private: bool is_Derived_;
private: bool is_Multivalued_;
};

#endif /* __isa_ATTRIBUTE_H */
