

/*
	
	This files defines the Entity implemenation.
	Released under GPL v 2.0 AND on authors choice a later version, if it comes
*/

#include "Entity.h"
#include <iostream>
#include "Relation.h"
#include "Dia_Exception.h"

Entity::Entity(const std::string name)
{
	this->name_ = name;
	this->is_Weak_ =  false;
	this->is_Associative_ = false;	
}

void Entity::add_Attribute(Attribute* attr)
{

	if ( attr->is_Primary_Key())	{
		this->primary_Key_.push_back(attr);
	}
	if ( is_Weak_ == true)	{
		if ( attr->is_Weak_Key() )	{
			this->primary_Key_.push_back(attr);
		}
	}
	
#if _DIAPER_DEBUG
	std::cout << "Inserting attribute " << attr->get_Name(); 
#endif
	attributes_.push_back(attr);
}



std::list<Attribute*>& Entity::get_Primary_Key(bool rec)	{

	if ( rec == true )	{
		return get_Primary_Key();
	}
	else {
		return primary_Key_;
	}
}


std::list<Attribute*>& Entity::get_Primary_Key()
{ 

// This should be called in the following scenario only.
//	1) The entity is weak.
//		1.1)	The primary key of it is not available. That is this function is called
// 			the first time.

	

	if ( ( is_Weak_Computed_ == false) )	{
	// We find the key ONLY once, for other times, we just return the value directly.
		is_Weak_Computed_ = true;
		if ( identifying_Relations_.size() == 0  )	{
			std::string msg = "Weak Entity " + this->get_Name() + " is not participating in any Identifying Relationship";
			throw Invalid_Relation_Exception(msg.c_str());
		}
		std::list<Relation*>::const_iterator it_IR = identifying_Relations_.begin();
		std::list<Attribute*> pk;

		while ( it_IR != identifying_Relations_.end() )	{
			
			pk = (*it_IR)->get_Other_Entity(this)->get_Primary_Key();
			primary_Key_.insert(primary_Key_.end(), pk.begin(), pk.end());
			it_IR++;
		}		
		return primary_Key_;
	}
	else {
		return primary_Key_;
	}
}

void Entity::add_Relation(Relation* rel)
{	

	bool is_Present	=	false;

	std::list<Relation*>::const_iterator it_R = relations_.begin();

	while ( it_R != relations_.end() )	{
		if ( (*it_R) == rel ) {
			is_Present = true;
			break;
		}
		++it_R;
	}
	if ( is_Present == false ) {

		relations_.push_back(rel); 

		// If the entity is weak and the relationship is identifying then we can only have 1:N
		if ( this->is_Weak() == true )	{
				
			// If the relationship is identifying
			if ( ( rel->is_Identifying() == true ) && ( rel->get_Right_Entity() == this ) )	{
				if ( rel->is_One_N() == false )	{
					std::string msg = "Relation " + rel->get_Name() + " is not 1:N with entity " + this->get_Name();
					throw Invalid_Relation_Exception(msg.c_str());		
				}
				identifying_Relations_.push_back(rel);			
			}
		}
	}
}
	
