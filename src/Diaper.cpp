

/*
	
	This files defines the parser implementation.
	Released under GPL v 2.0 AND on authors choice a later version, if it comes
*/


/*
	* TODO::	
	*	
	*	1)	Validation check for opening a file--Done
	*	2)	Validation check for every item that we use like making sure that if we expect
	*		an <dia:attribute name="name">
	*			we must 
 <dia:attribute
	*	3)	We need to deal with lines that just have one handle, and are in a way dangling around.	
	*	4)	Another thing is what if the user wants to specify a foreign key between attributes itself only,
	*		refer to figure 5.7 on page 165.
	*	5)	Shift the get_X_X_Mapping to relation class.

*/

/*
	Assumptions:
		The name of the attribute must not contain any form of #. It would break our implementation
		There is a simple way to deal with such issue actually

		As of now we don't deal with attributes, entitities or any "names" which have # in them even in quotes.
*/
#include "Diaper.h"
#include <iostream>
#include <fstream>		// For std::ifstream and std::ofstream.
#include <assert.h>
#include <string>
#include "Entity.h"
#include <vector>
#include <sstream>
#include <cctype> 
#include <algorithm>
#include "Dia_Exception.h"
//#include <Attribute.h>


std::string create_Table(const std::string& name, const std::list<Attribute*>& att);
std::string get_One_N_Mapping(Entity* left, Entity * right, Relation* rel );
std::string get_Foreign_Key(Entity* left, Entity* right, Relation* rel = NULL, bool append_Right=false);

void Diaper::resolve_Line_Pairs() throw ( Invalid_Relation_Exception)
{

	std::list<Line_Rep>::const_iterator it_p = to_Do_Pairs.begin();
	std::string msg;	
	while ( it_p != to_Do_Pairs.end() )		{
		int left  = cache[it_p->id_From];
		int right = cache[it_p->id_To];

		switch ( left )
		{
			case ENTITY_:
				if ( right == ATTRIBUTE_ ) { 
					entities_[it_p->id_From]->add_Attribute(attributes_[it_p->id_To]); 
					attributes_[it_p->id_To]->set_Entity(entities_[it_p->id_From]);
				}
				else if ( right == RELATION_ ) {
					relations_[it_p->id_To]->set_Left_Entity(entities_[it_p->id_From]);				
					entities_[it_p->id_From]->add_Relation(relations_[it_p->id_To]);
#ifdef _PARTICIPATION_
					if ( it_p->line == PARTICIPATION_ )	{
						relations_[it_p->id_To]->set_Right_Participation();
					}
#endif
				}

				else { 
					std::string msg = "Entity " + entities_[it_p->id_From]->get_Name();
					msg += " Can only be attached to some Attribute or Relation";
					throw Invalid_Diaper_Format_Exception(msg.c_str());
				}
			break;

			case RELATION_:
				if ( right == ENTITY_ )		{
					relations_[it_p->id_From]->set_Right_Entity(entities_[it_p->id_To]);
					entities_[it_p->id_To]->add_Relation(relations_[it_p->id_From]);
#ifdef _PARTICIPATION_
					if ( it_p->line == PARTICIPATION_ )	{
						relations_[it_p->id_From]->set_Left_Participation();
					}
#endif
				}
				else if ( right == ATTRIBUTE_ )				{
					relations_[it_p->id_From]->add_Attribute(attributes_[it_p->id_To]);	
					attributes_[it_p->id_To]->set_Entity(relations_[it_p->id_From]);
				}
				else	{
					std::string msg = "Relation " + relations_[it_p->id_From]->get_Name() + " attached to invalid object";
					throw Invalid_Diaper_Format_Exception(msg.c_str());
				}

			break;

			case ATTRIBUTE_:
				msg  = "Outgoing edge detected from " + attributes_[it_p->id_From]->get_Name();
				throw Invalid_Diaper_Format_Exception(msg.c_str());
				break;
			default:
				throw Invalid_Diaper_Format_Exception("Invalid Object connection found ");
		}

		++it_p;
	}

}

Diaper::Type Diaper::get_Object_Type(const std::string& str)
{

	// The following in with the assumption that we have a description of the form
	// ER - "A"ttribute
	// ER - "E"ntity
	// ER - "R"elationship	
	// Stand"a"rd - Line
	// "P"articipation.

	std::string msg;
	switch( str[5] )
	{
		case 'A':	return ATTRIBUTE_;	break;
		case 'R':	return RELATION_;	break;
		case 'E':	return ENTITY_	;	break; 
		case 'a':	return LINE_;		break;
		case 'P':	
			msg = "Diaper does not deal with participation, instead use line";
			throw Invalid_Diaper_Format_Exception(msg.c_str());
			break;
		default:	throw Invalid_Diaper_Format_Exception("Invalid Object found:" ) ;
	}
}

void Diaper::parse_File(const char* name) throw(std::ios::failure, Diaper_Exception)
{
	
	int line_Number = 0;

	input_File_.exceptions(std::ios::failbit);

	input_File_.open(name);
	
	input_File_.exceptions(std::ios::goodbit);

	std::string input_Line;
	const std::string DIA_OBJECT_TAG = "<dia:object type=\"";
	const std::string DIA_CLOSE_STRING = "\"";
	// Do a sanity check on file.

	while ( std::getline(input_File_, input_Line ) )
	{
		++line_Number_;		
		std::string::size_type index_Start = input_Line.find(DIA_OBJECT_TAG);
		if ( index_Start != std::string::npos )
		{
			/*
				*	Skip the object tag
			*/
			index_Start += DIA_OBJECT_TAG.length();
			std::string::size_type index_End   = input_Line.find("\"", index_Start);
			/*
				* The mistake that I was making here was that substr takes
				* ( start, number) rather than
				* ( start, end )
			*/
					
		 	std::string object	  = input_Line.substr(index_Start, index_End - index_Start );
            switch ( get_Object_Type(object) )
			{
				case ENTITY_:
					parse_Entity(input_Line);
					break;
				case ATTRIBUTE_:
					parse_Attribute(input_Line);
					break;
				case LINE_:
					parse_Line(input_Line);
					break;
				case RELATION_:
					parse_Relation(input_Line);
					break;
				case PARTICIPATION_:
					parse_Participation(input_Line);
				default:
					throw Invalid_Diaper_Format_Exception("Invalid Object Found");
			}
		}
	}

	// We have got a mapping of all the objects, now we deal with lines.
	resolve_Line_Pairs();

}

/* Just a utility function */

static int convert_String_To_Int(const std::string& str)
{
	std::istringstream i_string;
	i_string.exceptions(std::ios::failbit );
	i_string.str(str);
	assert(str[0] != 'O');
	int x;
	i_string >> x;

	return x;
}


void Diaper::skip_Lines(const int n)
{
	std::string str;

	for ( int i = 0; i != n; ++i)
	{		std::getline(input_File_, str);	}
	line_Number_ += n;
}

std::string Diaper::get_Object_Id(const std::string& obj_Line)
{
	const char* DIA_OBJ_ID = "id=\"";

	std::string::size_type index_Start = obj_Line.find(DIA_OBJ_ID);
	std::string::size_type index_End   = obj_Line.find('\"', index_Start + 4);
	
	std::string id = obj_Line.substr
							(
								index_Start + 5,	/* This 4 is the length of id="O */
								index_End - index_Start - 5
							);

#ifdef _DIAPER_DEBUG
	std::cout << "The ID of the object is " << id << '\n';
#endif
	return id;
}

/*
	* This function would just skip 2 lines and get the the element in # #
	* if it is non existant we just return (1,1)
	* if it is some alphabet we return -1
	* if it is some digit, we return it
	* What does string to int return in case the input is just a set of spaces.
*/

Diaper::
Int_Int_Pair_T Diaper::get_Relation_Cardinality(std::string& name)
{
	std::string msg = "Cardinality for relation " + name + " specified as negative " ;	
	std::string str;
	/* Skip 2 lines */
	std::getline(input_File_, str);
	std::getline(input_File_, str);
	line_Number_ += 2;

	/* Now get <dia:string>#(left)#(right)</dia:string>*/
	std::getline(input_File_, str);
	Int_Int_Pair_T card_Pair;
	std::string::size_type index_Start = str.find('#');
	std::string::size_type index_End   = str.find('#', index_Start + 1 );
	
	if ( index_End == index_Start + 1 )
	{	card_Pair.first = 1; card_Pair.second = 1;}
	else {	
		try	{
			card_Pair.first = convert_String_To_Int(str.substr(index_Start + 1, index_End - index_Start - 1 )); 		
			if ( card_Pair.first < 0 )	{
				throw Invalid_Relation_Exception(msg.c_str());
			}
		}
		catch (std::ifstream::failure e) {
				card_Pair.first = -1;
		}
		index_Start = index_End;
		index_End = str.find('#', index_Start + 1 );
		if ( ( index_End ==  std::string::npos ) || ( index_End == index_Start + 1) )		{
			card_Pair.second = card_Pair.first; 
		}
		else {
			try 		{
				card_Pair.second = convert_String_To_Int(
								str.substr(index_Start + 1, index_End - index_Start - 1 )
												    );
				if ( card_Pair.second < 0 )	{
					throw Invalid_Relation_Exception(msg.c_str());	
					}
			}
		
		catch(std::ifstream::failure e ) { card_Pair.second = -1; }
		}
	}
#ifdef _DIAPER_DEBUG
	std::cout << "The Cardinality is " << card_Pair.first << " " << card_Pair.second << '\n';
#endif
	return card_Pair;
		
}

void Diaper::parse_Relation(const std::string& obj_Line)
{
	std::string str;

	const std::string& id = get_Object_Id(obj_Line);
	skip_Lines(RELATION_SKIP_LINES + 1);


	std::string::size_type	index_Start;
	/*Get Name of the object */
	std::getline(input_File_, str); 
	index_Start = str.find("#");
	std::string name = str.substr(index_Start + 1, str.find("#", index_Start + 1 ) - index_Start - 1);
#ifdef _DIAPER_DEBUG
	std::cout << "Name of Relation " << name << '\n';
#endif

	Relation * rel = new Relation(name);

	// Now try to find out if we have total participation or not.

	if ( str.find("#LT", index_Start + 1 ) != std::string::npos ) {
		rel->set_Left_Participation(true);
	}
	else {
		rel->set_Right_Participation(false);
	}
	if ( str.find("#RT", index_Start + 1 ) != std::string::npos ) {
		rel->set_Right_Participation(true);
	}
	else	{
		rel->set_Right_Participation(false);
	}
	rel->set_Left_Card(get_Relation_Cardinality(name));
	rel->set_Right_Card(get_Relation_Cardinality(name));
	rel->set_Identifying(get_Next_Value());
	Relation_Int_Pair_T rel_t;
	rel_t.first =  convert_String_To_Int(id);
	rel_t.second=  rel;
	
	
	relations_.insert(rel_t);
	cache.insert(std::make_pair(rel_t.first, RELATION_));
}

bool Diaper::get_Next_Value()
{

	std::string str;
	// First skip two lines.
	std::getline(input_File_, str);
	std::getline(input_File_, str);
	
	std::getline(input_File_, str); // Get the boolean value for key.

	std::string::size_type	index_Start = str.find("val=\"");
	//
	//std::cout << "The index is " << str[index_Start+5] << '\n';

	return str[index_Start + 5 ] == 'f' ? false : true;

}


void Diaper::parse_Entity(const std::string& obj_Line)
{

	std::string id = get_Object_Id(obj_Line);
	skip_Lines(ENTITY_SKIP_LINES + 1);	

	std::string str;
	std::string::size_type	index_Start;
	/*Get Name of the object */
	std::getline(input_File_, str); 
	index_Start = str.find("#");


	std::string name = str.substr(index_Start + 1, str.find("#", index_Start + 1 ) - index_Start - 1);
	Entity* entity = new Entity(name);
#ifdef _DIAPER_DEBUG
	std::cout << "Name of entity is " << name << '\n';
#endif
	entity->set_Weak_Flag(get_Next_Value());
	entity->set_Associative_Flag(get_Next_Value());
	
	Entity_Int_Pair_T ent_i_pair;
	ent_i_pair.first = convert_String_To_Int(id);
	ent_i_pair.second = entity;

	/* Finally insert the entity in our data structures */
	entities_.insert(ent_i_pair);
	cache.insert(std::make_pair(ent_i_pair.first, ENTITY_));
	
}

void Diaper::parse_Attribute(const std::string& obj_Line)
{
	std::string str;

	const std::string & id = get_Object_Id(obj_Line);
	skip_Lines(ATTRIBUTE_SKIP_LINES + 1);

	std::string::size_type	index_Start;
	/*Get Name of the object */
	std::getline(input_File_, str); 
	index_Start = str.find("#") + 1;
	std::string::size_type index_End = str.find('#', index_Start + 1 );
	std::string name = str.substr(index_Start, index_End - index_Start);

	/* Get the type of the attribute */
	index_Start = index_End + 1;
	index_End =	 str.find('#', index_Start );
	
	std::string type;
	
	if ( index_End == std::string::npos )
	{
		std::cerr << "ERROR::Type of " << name << " not specified " << '\n';				
		exit(-1);					
	}
	else
	{
		type = str.substr(index_Start, index_End - index_Start );
	}
#ifdef _DIAPER_DEBUG
	std::cout << "Name of Attribute " << name << '\n';
#endif
	Attribute* attribute = new Attribute(name, type);

	attribute->set_Primary_Key(get_Next_Value());
	attribute->set_Weak_Key(get_Next_Value());
	attribute->set_Derived(get_Next_Value());
	attribute->set_Multivalued(get_Next_Value());

	Attribute_Int_Pair_T att_t;
	att_t.first = convert_String_To_Int(id);
	att_t.second = attribute; // Right now we are defaulting to varchar2



	/* Finally insert the attribute in our data structures */
	attributes_.insert(att_t);
	cache.insert(std::make_pair(att_t.first, ATTRIBUTE_));
	

}

inline int get_Connection_Handle(const std::string& str)
{
	
	std::string::size_type index_Start = str.find("to=\"");
	index_Start += 4;
	std::string::size_type index_End   = str.find("\"", index_Start  );

	
	return convert_String_To_Int(str.substr(index_Start + 1, index_End - index_Start - 1 ) );

	
	
}
Diaper::Int_Int_Pair_T Diaper::get_Line_Connection()
{

	const char* DIA_CONNECTION = "<dia:connection";
    const char* DIA_CONNECTION_GROUP_START = "<dia:connections>";
    const char* DIA_CONNECTION_GROUP_END = "</dia:connections>";
    const char* DIA_OBJECT_END = "</dia:object>";
	const char * error_Msg = 
					"ERROR::Unexpected Input File format\n"
					"Line with open connections found.\nMake sure all your Lines are connected.";

	std::string str;
	std::getline(input_File_, str);

	while ( str.find(DIA_CONNECTION_GROUP_START) == std::string::npos )
	{
	   line_Number_ += 1;
	   std::getline(input_File_, str);
	   if( str.find(DIA_OBJECT_END) != std::string::npos )
	   { 
	     std::cout << "Hit end of object while looking for connections\n"; 
	     throw Invalid_Diaper_Format_Exception(error_Msg); 
	   }
	}
					
    std::getline(input_File_, str);
    line_Number_ += 1;
	if ( str.find(DIA_CONNECTION) == std::string::npos  )
	{
        std::cout << "Last Line (" <<  line_Number_ << ") was: '" << str << "'\n";
	    throw Invalid_Diaper_Format_Exception(error_Msg);
	}

	Diaper::Int_Int_Pair_T rel;

	rel.first = get_Connection_Handle(str);

	/* Move on to the next line */
	std::getline(input_File_, str);
    line_Number_ += 1;

	if ( str.find(DIA_CONNECTION) == std::string::npos  )
    {
        std::cout << "Last Line (" <<  line_Number_ << ") was: '" << str << "'\n";
        throw Invalid_Diaper_Format_Exception(error_Msg);
    }

	rel.second = get_Connection_Handle(str);


#ifdef _DIAPER_DEBUG

	std::cout << "A connection has been established between " << rel.first 
				<< " and " << rel.second << '\n';
#endif
	return rel;
}

void Diaper::parse_Line(const std::string& obj_Line)
{
	std::string str;
	
	get_Object_Id(obj_Line);
	
//	skip_Lines(LINE_SKIP_LINES - 1);

	/* Now get the From and to and put them in the to_Do List */

    try {
	    Int_Int_Pair_T	p = get_Line_Connection();
        Line_Rep rep;
        rep.id_From = p.first;
        rep.id_To   = p.second;
        rep.line    = LINE_;
        to_Do_Pairs.push_back(rep);
	} catch ( Diaper_Exception diaper_Ex)    {
        std::cout << "Last line was:\n" << obj_Line << "\n";
	    throw diaper_Ex;
	}
	

}

std::string Diaper::create_Rec_Table( Entity* ent)
{
	
	std::list<Attribute*>	attr = ent->get_Attributes();

	if ( attr.size() == 0 )	{
		std::string msg = "Entity " + ent->get_Name() + " Has no attributes ";
		throw Invalid_Entity_Exception(msg.c_str());
	}
	std::list<Attribute*>::const_iterator it_A = attr.begin();
	std::list<Attribute*>				  pr_Key;
	
	std::string output			=	"CREATE TABLE ";
	output +=  ent->get_Name();
	output += " ( ";
	
	pr_Key = ent->get_Primary_Key(true);

	if ( pr_Key.size() == 0 )	{
		std::string msg = "Entity " + ent->get_Name() + " has no primary key";
		throw Invalid_Entity_Exception(msg.c_str());
	}

	if ( ent->is_Weak() == true)	{
		
		std::list<Attribute*>::const_iterator it_P = pr_Key.begin();

		while ( it_P != pr_Key.end())	{
			attr.push_back(*it_P);
			++it_P;
		}
		attr.sort();
		attr.unique();	
		it_A = attr.begin();
	}

	
	std::list<Entity*> multi_Val;

	while ( it_A != attr.end() )		{

		if ( (*it_A)->is_Multivalued() )	{

			if ( (*it_A)->is_Primary_Key() == true )	{
				std::string msg = "Multivalued Attribute " + (*it_A)->get_Name() + " can't be a primary key";
				throw Invalid_Attribute_Exception(msg.c_str());
			}
			else	{
				Entity *ent1 = new Entity(ent->get_Name() + "_" + (*it_A)->get_Name());
				ent1->set_Weak_Flag(true);
				Attribute* att = new Attribute((*it_A)->get_Name(), (*it_A)->get_Type());
				att->set_Entity(ent1);
				att->set_Primary_Key(true);

				ent1->add_Attribute(att);
				multi_Val.push_back(ent1);
			}		
		}
		else {
			if ( (*it_A)->get_Entity() != ent )	{
				output += (*it_A)->get_Entity()->get_Name() + "_";
			}
			output += (*it_A)->get_Name() + "  " + (*it_A)->get_Type();		
			if	( (*it_A)->is_Primary_Key() )			{
				output += " NOT NULL ";
			}
		}
		if  ( (*it_A)->is_Multivalued() == true )	{
			++it_A;
			continue;
		}
		it_A++;
		if ( ( it_A != attr.end() ) &&  ( (*it_A)->is_Multivalued() == false ) )		{
			output += " , ";
		}
	
		
	}
	
	// Now associate primary key constraint.	

	if ( pr_Key.size() > 0 )		{
		output += " , PRIMARY KEY  ( ";

		std::list<Attribute*>::const_iterator it_Pk = pr_Key.begin();

		while ( it_Pk != pr_Key.end() )			{

			if ( (*it_Pk)->get_Entity() != ent )	{
				output += (*it_Pk)->get_Entity()->get_Name() + "_";
			}
			output += (*it_Pk)->get_Name();
			it_Pk ++;
			// nur ein primary key pro tabelle
			// nämlich der erste der in dieser Liste steht. hoffentlich stimmt das immer
			it_Pk = pr_Key.end();
			
			if ( it_Pk != pr_Key.end() )		{
				output += " , ";
			}
		}
		output+= " ) ";
	}

	output += " )ENGINE=InnoDB DEFAULT CHARSET=utf8 ;\n";

	// For all multivalued attributes create a table
	std::list<Entity*>::const_iterator it_MV = multi_Val.begin();
	std::string att_Name;
	std::string tab_Name;

	while ( it_MV != multi_Val.end() )	{
		// The multivalued attribute should also be the primary key.
		
		// Push this attribute into the primary key list.
		// The MV entity would be having only one attribute
		pr_Key.push_back(*(*it_MV)->get_Attributes().begin());
		// Now create a table with this name_multi-val-attribute
		tab_Name =  (*it_MV)->get_Name();
		output += create_Table(tab_Name, pr_Key);
		pr_Key.pop_back();
		// Now Add Foreign key.
		output += get_Foreign_Key(ent, *it_MV, NULL);

		++it_MV;
	}

	return output;
}

/*
	@name --> The name of the table.
	@atts  --> The attribute list.
*/

std::string create_Table(const std::string& name, const std::list<Attribute*>& atts)
{

	std::string output =  "CREATE TABLE " + name + " ( ";

	std::list<Attribute*>::const_iterator it_A  = atts.begin();

	std::list<Attribute*> pr_Key;
	while ( it_A != atts.end() )	{
		// Append name
		output += (*it_A)->get_Entity()->get_Name();
		output += "_";
		output += (*it_A)->get_Name(); 
		// Append space
		output += " ";
		// Append type
		output += (*it_A)->get_Type();
		// If Primary key append
		if ( (*it_A)->is_Primary_Key() )		{
			pr_Key.push_back(*it_A);
				output += " NOT NULL ";
		}
		// Append ,
		it_A++;
		if ( it_A != atts.end() )	{
			output += " , ";
		}
	}

	if ( pr_Key.size() != 0 )	{
		output += " , PRIMARY KEY  ( ";

		std::list<Attribute*>::const_iterator it_Pk = pr_Key.begin();

		while ( it_Pk != pr_Key.end() )			{
			output += (*it_Pk)->get_Entity()->get_Name() + "_";
			output += (*it_Pk)->get_Name();
			it_Pk ++;
			if ( it_Pk != pr_Key.end() )		{
				output += " , ";
			}
		}
		output += " ) ";
	}
	
	
	output += " ) ENGINE=InnoDB DEFAULT CHARSET=utf8 ;\n";
	return output;
}

std::string get_M_N_Mapping(Entity* left, Entity* right, Relation* rel)	
{
	
	std::string output;
	std::string temp_Output;
	
	std::list<Attribute*> p_Key;

	p_Key.insert(p_Key.end(), left->get_Primary_Key().begin(), left->get_Primary_Key().end());

	// The case of a unary relationship with M:N mapping has to be handled specially.
	// WARNING: The following LEAKS memory.
	if ( left == right )	{

		std::list<Attribute*> mod_List;
		std::list<Attribute*> hold_List = right->get_Primary_Key();
		std::list<Attribute*>::const_iterator it_H = hold_List.begin();

		while ( it_H != hold_List.end())	{
			Attribute* att = new Attribute(rel->get_Name() + "_" + (*it_H)->get_Name(), (*it_H)->get_Type());
			att->set_Entity(right);
			att->set_Primary_Key(true);
			mod_List.push_back(att);
			++it_H;
		}
		p_Key.insert(p_Key.end(), mod_List.begin(), mod_List.end());
	}
	else {
		p_Key.insert(p_Key.end(), right->get_Primary_Key().begin(), right->get_Primary_Key().end());
	}
	p_Key.insert(p_Key.end(), rel->get_Attributes().begin(), rel->get_Attributes().end());
	

	output += create_Table(rel->get_Name(), p_Key);

	output += get_Foreign_Key(left, rel, NULL);

	if ( left == right )	{
		output += get_Foreign_Key(right, rel, NULL, true);
	}
	else {
		output += get_Foreign_Key(right, rel, NULL);
	}
	return output;
}

std::string get_Foreign_Key(Entity* left, Entity* right, Relation* rel, bool append_Right)
{
	std::list<Attribute*> f_Key = left->get_Primary_Key();

	std::list<Attribute*>::const_iterator it_FK = f_Key.begin();
	std::string ref_Name	=	left->get_Name();

	std::string foreign;
	std::string primary;

	std::string output = "ALTER TABLE " + right->get_Name();
	output += " ADD FOREIGN KEY ";

	std::string prefix ;

	while ( it_FK != f_Key.end() )	{

		if ( left == right )	{
			prefix += rel->get_Name() + "_";
		}
		foreign += prefix + (*it_FK)->get_Entity()->get_Name() + "_";
		foreign +=  (*it_FK)->get_Name();

		if ( append_Right == true )	{
			foreign += "_" + right->get_Name();
		}
		

		if ( (*it_FK)->get_Entity() != left )	{
			primary += (*it_FK)->get_Entity()->get_Name() + "_";
		}
		primary += (*it_FK)->get_Name();
		it_FK++;
		if ( it_FK != f_Key.end() )	{
				foreign += " , ";
				primary += " , ";
		}
	}

	std::string constraint;
	bool is_Left_Total_Part = false;
	bool is_Right_Total_Part = false;

	if ( rel != NULL )	{
		is_Left_Total_Part = rel->get_Left_Participation();
		is_Right_Total_Part = rel->get_Right_Participation();
	}
/*
	if ( ( is_Right_Total_Part == false ) && ( is_Left_Total_Part == false ) )	{
		constraint += " ON DELETE SET NULL ON UPDATE CASCADE ";
	}

	else if ( (right->is_Weak() == true) || ( is_Left_Total_Part == true ) ) {
				constraint += " ON DELETE CASCADE ON UPDATE CASCADE ";
	}
	else if ( ( right->is_Weak() == false ) && ( is_Right_Total_Part == true ) )	{
		constraint += " ON DELETE CASCADE ";
	}
	else if ( (right->is_Weak() == false) ) {
			constraint += " ON DELETE SET NULL ON UPDATE CASCADE ";
	}
*/
	output = output + " ( " + foreign + " ) REFERENCES " + ref_Name + " ( " + primary + " ) " + constraint + ";\n";
	return output;	
}

std::string get_One_N_Mapping(Entity* left, Entity * right, Relation* rel )	{

	// First add to the colums of the primary key name into it.
	
	std::list<Attribute*> p_Key = left->get_Primary_Key(true);
	if ( p_Key.size() == 0 )	{
		std::string msg;
		msg += "Entity " + left->get_Name() + " has no primary key";
		throw Invalid_Entity_Exception(msg.c_str());
	}
	
		
	std::list<Attribute*>::const_iterator it_P	= p_Key.begin();
	
	const std::string ALT_TABLE = "ALTER TABLE " + right->get_Name() + " ADD COLUMN (";
	std::string output;

	std::string prefix;

	if ( left == right )	{
		prefix = rel->get_Name() + "_";
	}

	// If the right entity is weak and the relation is identifying, we don't add any more columns, we just need to associate the foreign key constraint


	if ( ( right->is_Weak() == false ) || ( rel->is_Identifying() == false ) )	{

		output += ALT_TABLE;
		while ( it_P != p_Key.end() )		{

			output += prefix + (*it_P)->get_Entity()->get_Name() +  "_" + (*it_P)->get_Name() ;
			output +=	" " + (*it_P)->get_Type();
			++it_P;
			if ( it_P != p_Key.end() )	{
				output += " , ";
			}
		}
		output += " );\n";
	}
	// Now add the relational attributes to the output.
	std::list<Attribute*>& rel_Attr = rel->get_Attributes();
	if ( rel_Attr.size() > 0 )	{
		output += ALT_TABLE;  
	
		std::list<Attribute*>::const_iterator it_R = rel_Attr.begin();
		while ( it_R != rel_Attr.end() )	{
			
			output +=  rel->get_Name() + "_" + (*it_R)->get_Name();
			output += " ";
			output +=  (*it_R)->get_Type();
			++it_R;
			if ( it_R != rel_Attr.end() )	{
				output += " , ";
			}
		}

		output += " );\n";
	}

	//Now add the foreign key; the big one.

	output += get_Foreign_Key(left, right, rel);
	
	return output;
	

#ifdef _DIAPER_DEBUG
	std::cout << "Getting the output.......... " << output << '\n';
#endif
	return output;
}


std::string get_One_One_Mapping(Entity* left, Entity * right, Relation * rel)	
{

	// Always shift towards the right, except if the left is a weak key or totaly participats.
	if ( ( left->is_Weak() == true ) || (rel->get_Left_Participation() == true ) ) {
		Entity* temp;
		temp = left;
		left = right;
		right = temp;
	}
	// Check for total participation.
	return get_One_N_Mapping(left, right, rel);

}
void Diaper::write_Sql(const char* name) throw ( std::ios::failure, Diaper_Exception)
{
	std::ofstream output_File;
	
	output_File.exceptions(std::ios::failbit);
	try	{		
		output_File.open(name);	
	}
	catch(std::ios::failure fail)	{
		throw;
	}
	
	// Now go through all the entities and concatenate using the attributes

	std::map<int, Entity*>::const_iterator it_E = entities_.begin();
	std::string str;

	// First deal with entity types.
	while ( it_E != entities_.end() )	{
		str = create_Rec_Table(it_E->second);		
		output_File << str;
		it_E++;
	}	

	std::map<int, Relation*>::const_iterator it_R	=	relations_.begin();
	Entity* left ;
	Entity* right ;

	while ( it_R != relations_.end() )	{

			left = it_R->second->get_Left_Entity();
			right = it_R->second->get_Right_Entity();

			//Map 1:1 relationship.
			if ( it_R->second->is_One_One() == true )	{
				output_File << get_One_One_Mapping(left, right, it_R->second );
			}
			//Map 1:N or N:1 relationship.
			if ( it_R->second->is_One_N() == true )	{
					output_File << get_One_N_Mapping(left, right, it_R->second);			
				}
			if ( it_R->second->is_N_One() == true ) {
					left = it_R->second->get_Right_Entity();
					right  = it_R->second->get_Left_Entity();
					output_File << get_One_N_Mapping(left, right, it_R->second);			
				}
			// Now map M:N relation ship.
			if ( it_R->second->is_M_N() == true )	{
				output_File << get_M_N_Mapping(left, right , it_R->second);
			}
			it_R++;			
	}	
	// Now go through all the relations and output the tables corresponding it.
	output_File.close();
}
