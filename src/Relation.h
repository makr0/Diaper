

/*
	
	This files defines the Relation abstraction and implemenation
	Released under GPL v 2.0 AND a later version, if it comes
*/
//	@TO-DO get_Left_Participation must be changed to is_Left_Partcipation

#ifndef __diaper_Relation_H
#define __diaper_Relation_H

#include <string>
#include <iostream>
#include <utility>
#include "Attribute.h"
#include "Entity.h"
#include <assert.h>

#include "Dia_Exception.h"

class Relation : public Entity { 

public: Relation(const std::string name):Entity(name) {		
			 left_Entity_ = NULL; right_Entity_=NULL; 

			 set_Weak_Flag(true);
			}	
public: void set_Left_Card(std::pair<int, int> n)		{ left_Card_ = n; }
public: std::pair<int, int> get_Left_Card()				{ return left_Card_;}

public: void set_Right_Card(std::pair<int,int> n)		{ right_Card_ = n; }
public: std::pair<int, int>& get_Right_Card()			{	return right_Card_;	}
public: std::pair<int, int>& right_Card()				{ return right_Card_; }


public: void set_Left_Entity(Entity* entity)		{ 
			// Make sure that we aren't setting this left entity again.
				if ( left_Entity_ != NULL )				{
					assert(0);
				}
				left_Entity_ = entity;
		}
public: Entity* get_Left_Entity() { return left_Entity_;	}

public: void set_Right_Entity(Entity* entity)	{
				
			if ( right_Entity_ != NULL )	{
				assert(0);
			}
			right_Entity_ = entity;
		}
public: Entity* get_Right_Entity() { return right_Entity_; }

public: void set_Left_Participation(bool flag=true)	{	left_Participation_  = flag; }
public: bool get_Left_Participation()				{ return left_Participation_; }


public: void set_Right_Participation(bool flag = true ) { right_Participation_ = flag; }
public: bool get_Right_Participation()					{ return right_Participation_; }




public: Entity*	get_Other_Entity(Entity* en)	
		{
			if ( en == left_Entity_	)	{ return right_Entity_;	}
			else return left_Entity_;
		}
public: void set_Identifying(bool flag = true) { is_Identifying_ = flag; }
public: bool is_Identifying()	{	return is_Identifying_;	}

public: bool is_One_N()	{

			if ( ( left_Card_.first == left_Card_.second ) && ( left_Card_.first != - 1)  ) {
				if (  ( right_Card_.second == -1 ) ||  ( ( right_Card_.second - right_Card_.first ) > 0  ) )
					return true;
			}
			return false;
		}
public: bool is_N_One()	{

			if ( ( right_Card_.first == right_Card_.second) && ( right_Card_.first != - 1) )	{
				if ( ( left_Card_.second == -1 ) || ( ( left_Card_.second - left_Card_.first ) > 0  ) )	{
					return true;
				}
			}		
			return false;
		}
public: bool is_M_N()	{
			if ( ( left_Card_.second == -1 ) && ( right_Card_.second == -1 ) )
				return true;
			if ( ( ( right_Card_.second - right_Card_.first ) > 0 ) && ( ( left_Card_.second - left_Card_.first ) > 0 ) ) 	{
				return true;
			}
			if ( right_Card_.second != -1 && left_Card_.second != - 1) {
				if ( ( right_Card_.first - right_Card_.second  > 0 ) || ( left_Card_.first - left_Card_.second > 0 ) )	{
					std::string msg = "The max is less than min for " + this->get_Name();
					throw Invalid_Relation_Exception(msg.c_str());
				}
			}
		return false;
}
public: bool is_One_One()	{

			if ( ( left_Card_.first == left_Card_.second ) && ( right_Card_.first == right_Card_.second ) && ( left_Card_.first != -1 ) && ( right_Card_.first != -1 ) ) {
				return true;
			}
			return false;
		}

/* Status Flags */
private: bool is_Identifying_;

/* Left and Right Cardinality */

private: std::pair<int, int> left_Card_;
private: std::pair<int, int> right_Card_;


/* The left and right entities participating in a relation */

private: Entity* left_Entity_;
private: Entity* right_Entity_;

private: bool left_Participation_;
private: bool right_Participation_;
};

#endif // __isa_Relation_H